import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from build import digest
import recovery
from run import append


class Recovery(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.prior = self.root/'prior'
        self.prior.mkdir()
        self.exe = self.root/'exe'
        self.exe.write_text('identity')
        (self.prior/'resume-identities.json').write_text(json.dumps({str(self.exe):digest(self.exe)}))
        (self.prior/'index.jsonl').touch()

    def test_interrupted_output_and_index(self):
        raw = self.prior/'case-zero-instrumented.jsonl'
        raw.write_text('{"truncated":')
        (self.prior/'index.jsonl').write_text('{"unacknowledged":')
        output = self.root/'new'
        recovery.import_prior(self.prior, output, {})
        self.assertEqual((output/'interrupted'/raw.name).read_bytes(), raw.read_bytes())
        self.assertEqual((output/'index.jsonl').read_text(), '')
        self.assertTrue((output/'inherited-index.jsonl').read_text())

    def test_repeated_interruption_keeps_every_attempt(self):
        raw = self.prior/'case-zero-instrumented.jsonl'
        raw.write_text('first interrupted bytes')
        output = self.root/'new'
        recovery.import_prior(self.prior, output, {})
        (output/raw.name).write_text('second interrupted bytes')
        again = self.root/'again'
        recovery.import_prior(output, again, {})
        self.assertEqual((again/'interrupted'/raw.name).read_text(), 'first interrupted bytes')
        self.assertEqual((again/'interrupted'/(raw.name+'.1')).read_text(), 'second interrupted bytes')

    def test_hash_mismatch(self):
        self.exe.write_text('changed')
        with self.assertRaisesRegex(ValueError, 'hash mismatch'):
            recovery.import_prior(self.prior, self.root/'new', {})
        self.assertFalse((self.root/'new').exists())

    def test_timeout_and_skip_preserved(self):
        decisions = [dict(case='small', variant='padded', status='timeout'),
                     dict(case='large', variant='padded', status='skipped-larger-in-failed-series')]
        for row in decisions:
            append(self.prior/'index.jsonl', row)
        recovery.import_prior(self.prior, self.root/'new', {})
        self.assertEqual(recovery.rows(self.root/'new/index.jsonl'), decisions)

    def test_later_phase_restart_validates_checkpoint(self):
        raw = self.prior/'upstream.json'
        raw.write_text('{}')
        recovery.checkpoint(self.prior, 'control-build', [raw])
        recovery.import_prior(self.prior, self.root/'new', {})
        self.assertTrue(recovery.phase_complete(self.root/'new', 'control-build'))
        raw.write_text('{"changed":true}')
        with self.assertRaisesRegex(ValueError, 'checkpoint hash mismatch'):
            recovery.import_prior(self.prior, self.root/'other', {})

    def test_live_child_keeps_lock_after_supervisor_closes(self):
        lock = self.root/'execution.lock'
        with recovery.execution_lock(lock):
            child = subprocess.Popen([sys.executable, '-c', 'import sys; sys.stdin.read()'],
                                     stdin=subprocess.PIPE, pass_fds=recovery.LOCK_FDS)
            with self.assertRaises(BlockingIOError):
                with recovery.execution_lock(lock):
                    pass
        try:
            with self.assertRaises(BlockingIOError):
                with recovery.execution_lock(lock):
                    pass
        finally:
            child.communicate(timeout=5)
        with recovery.execution_lock(lock):
            pass

    def test_later_upstream_operation_and_checkpoint_survive_import(self):
        raw = self.prior/'case-upstream-instrumented.jsonl'
        raw.write_text(json.dumps(dict(geometry=[{'sourceToBytes':[0]}]))+'\n')
        append(self.prior/'index.jsonl', dict(case='case', variant='upstream',
            label='instrumented', raw=raw.name, rawSha256=digest(raw), status='completed'))
        binary = self.prior/'control-executable'
        binary.write_text('compiled upstream counter')
        recovery.checkpoint(self.prior, 'control-build', [binary])
        pending = self.prior/'case-upstream-headline.jsonl'
        pending.write_text('{"unfinished":')
        output = self.root/'new'
        recovery.import_prior(self.prior, output, {})
        self.assertTrue(recovery.phase_complete(output, 'control-build'))
        self.assertEqual(recovery.rows(output/'index.jsonl')[0]['label'], 'instrumented')
        self.assertEqual((output/raw.name).read_bytes(), raw.read_bytes())
        self.assertTrue((output/'interrupted'/pending.name).exists())

    def test_completed_raw_hash_mismatch_stops_import(self):
        raw = self.prior/'case-zero-instrumented.jsonl'
        raw.write_text('{"geometry":[]}\n')
        append(self.prior/'index.jsonl', dict(case='case', variant='zero', label='instrumented',
            raw=raw.name, rawSha256=digest(raw), status='completed'))
        raw.write_text('{"geometry":[1]}\n')
        with self.assertRaisesRegex(ValueError, 'raw hash mismatch'):
            recovery.import_prior(self.prior, self.root/'new', {})

    def test_claimed_completed_truncated_output_is_rejected(self):
        raw = self.prior/'case-zero-instrumented.jsonl'
        raw.write_text('{')
        append(self.prior/'index.jsonl', dict(case='case', variant='zero', label='instrumented', raw=raw.name, status='completed'))
        with self.assertRaises(json.JSONDecodeError):
            recovery.import_prior(self.prior, self.root/'new', {})


if __name__ == '__main__':
    unittest.main()
