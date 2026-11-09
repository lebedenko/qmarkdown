"""Recovery tests plus executable evidence checks (optional evidence argument)."""
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest

HERE=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('phase_run',HERE/'run.py')
runner=importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)
EVIDENCE=Path(sys.argv.pop()) if len(sys.argv)>1 else None


class ResumeTests(unittest.TestCase):
    def setup_prior(self, root):
        prior=root/'prior';prior.mkdir()
        (prior/'identities.json').write_text('{}')
        (prior/'index.jsonl').write_text('')
        (prior/'dense-480-1024.json').write_text('{}')
        raw=prior/'dense-480-1024-padded-instrumented-trace.jsonl'
        raw.write_text('{"phase":"initial"}\n{"phase":')
        return prior,raw

    def test_interrupted_phase_is_terminal_and_preserved(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);prior,raw=self.setup_prior(root)
            rows=runner.resume(prior,root/'next')
            self.assertEqual(rows[0]['status'],'interrupted')
            self.assertEqual((root/'next'/raw.name).read_bytes(),raw.read_bytes())
            again=runner.resume(root/'next',root/'again')
            self.assertEqual(again,rows)

    def test_changed_checkpoint_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);prior,raw=self.setup_prior(root)
            runner.recovery.checkpoint(prior,'phase',[raw.with_name('dense-480-1024.json')])
            (prior/'dense-480-1024.json').write_text('{"changed":true}')
            with self.assertRaises(ValueError):runner.resume(prior,root/'next')

    def test_changed_identity_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);prior,raw=self.setup_prior(root)
            (prior/'identities.json').write_text(json.dumps({str(raw):'incorrect'}))
            with self.assertRaises(AssertionError):runner.resume(prior,root/'next')

    def test_existing_destination_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);prior,_=self.setup_prior(root)
            with self.assertRaises(AssertionError):runner.resume(prior,prior)


@unittest.skipUnless(EVIDENCE,'pass evidence directory to check runtime attribution')
class AttributionTests(unittest.TestCase):
    def test_phase_attribution_and_query_exclusion(self):
        records=runner.recovery.rows(EVIDENCE/'index.jsonl')
        self.assertEqual(len(records),48)
        self.assertTrue(all(r['status']=='completed' for r in records))
        for record in records:
            if record['label']!='instrumented':continue
            rows=runner.recovery.rows(EVIDENCE/record['raw'])
            self.assertEqual([r['phase'] for r in rows],['initial','reflow','repeat','metadata'])
            for row in rows:
                for work in row['counters']:
                    self.assertEqual(work['fitting']['calls'],work['candidates'])
                    if row['phase']=='repeat':
                        # Queries in the preceding phase must not leak into this snapshot.
                        self.assertEqual(sum(work['formatting'].values()),0)
                        self.assertEqual(work['candidates'],0)
                    if record['variant']=='zero':self.assertEqual(work['candidates'],0)
                if row['phase'] in ('repeat','reflow'):self.assertFalse(any(row['shapeDeltas']))
            peer=EVIDENCE/record['raw'].replace('-instrumented-','-headline-')
            self.assertEqual([(r['geometry'],r['shapeDeltas']) for r in rows],
                             [(r['geometry'],r['shapeDeltas']) for r in runner.recovery.rows(peer)])

if __name__=='__main__':unittest.main()
