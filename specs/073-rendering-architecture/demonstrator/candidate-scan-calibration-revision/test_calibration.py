#!/usr/bin/env python3
"""Synthetic recovery and analysis tests; never execute a measurement binary."""
import copy
import json
from pathlib import Path
import random
import shutil
import tempfile
import unittest
from unittest.mock import patch
import run
import report


def synthetic(root):
    root.mkdir()
    for case in run.CASES:
        shutil.copyfile(run.HISTORICAL / (case + '.json'), root / (case + '.json'))
        name = f'correctness-00-{case}-zero-baseline-sample.jsonl'
        target = root / 'inputs/continuation' / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(run.HISTORICAL / name, target)
    env = run.environment()
    sample = run.continuation.old.strict_rows(target)
    env['affinity'] = sample[0]['cpuAffinity']
    for name, value in {'identities.json': {}, 'environment.json': env,
                        'commands.json': [run.command(e) for e in run.schedule()],
                        'schedule.json': run.schedule()}.items():
        run.write(root / name, value)
    for name in ('index.jsonl', 'attempts.jsonl'):
        (root / name).touch()
    run.recovery.checkpoint(root, 'prepared', [root / name for name in
        ('identities.json', 'environment.json', 'commands.json', 'schedule.json', *(c+'.json' for c in run.CASES))])


def fake_supervise(cmd, raw, err, **kwargs):
    case, mode = Path(cmd[1]).stem, cmd[3]
    source = run.HISTORICAL / f'{mode}-00-{case}-zero-baseline-sample.jsonl'
    shutil.copyfile(source, raw)
    err.write_text('')
    return dict(status='completed', returncode=0, peakObservedRssBytes=1000, wallSeconds=.1, command=cmd)


def sample(ns, count=2, resolution=1):
    return dict(elapsedNs=ns, applicableParagraphs=count, clockPeriodNs=1, minimumPositiveDeltaNs=resolution)


class ScheduleTests(unittest.TestCase):
    def test_schedule(self):
        entries = run.schedule()
        self.assertEqual(entries, run.schedule())
        self.assertEqual(len(entries), 322)
        self.assertEqual(len({e['id'] for e in entries}), 322)
        self.assertEqual([e['case'] for e in entries[:2]], list(run.CASES))
        rng = random.Random(73078)
        orders = {}
        for case in run.CASES:
            orders[case] = []
            for _ in range(10):
                block = ['ABBA', 'ABBA', 'BAAB', 'BAAB']
                rng.shuffle(block)
                orders[case] += block
        actual = iter(entries[2:])
        for i in range(40):
            cases = list(run.CASES)
            rng.shuffle(cases)
            for case in cases:
                group = [next(actual) for _ in range(4)]
                self.assertEqual(''.join(e['label'] for e in group), orders[case][i])
                self.assertEqual([e['order'] for e in group], list(range(4)))
                for e in group:
                    self.assertEqual((e['sample'], e['case'], e['pair']), (i, case, group[0]['pair']))
                    self.assertEqual(run.command(e), [str(run.BINARY), str(run.HISTORICAL/(case+'.json')), 'zero', 'timing'])
        for e in entries[:2]:
            self.assertEqual(run.command(e), [str(run.BINARY), str(run.HISTORICAL/(e['case']+'.json')), 'zero', 'correctness'])


class StatisticsTests(unittest.TestCase):
    def stats(self, a=10000, b=10020, count=2, resolution=1):
        orders = ['ABBA', 'BAAB']*20
        groups = [[sample(a if arm == 'A' else b, count, resolution) for arm in order] for order in orders]
        return report.summarize(groups, [list(range(40))]*100, orders)

    def test_normalization_and_boundary(self):
        c = self.stats()
        self.assertEqual(c['overheadPerParagraphNs'], 10)
        self.assertEqual(c['descriptive']['mad'], 0)
        self.assertTrue(report.envelope(c, 10))
        self.assertFalse(report.envelope(c, 9.999))
        self.assertTrue(report.envelope(self.stats(b=9980), 10))
        self.assertEqual(c['intervals95']['overheadPerParagraphNs'], [10, 10])
        self.assertEqual(c['medianImprovement'], -0.002)

    def test_linear_position_drift(self):
        for order in ('ABBA', 'BAAB'):
            for drift in (-100, 0, 100):
                rows = [sample(10000 + drift*i + (20 if arm == 'B' else 0)) for i, arm in enumerate(order)]
                self.assertEqual(report.group_value(rows, order)['overheadPerParagraphNs'], 10)

    def test_timer_and_nulls(self):
        self.assertTrue(self.stats(a=99, b=100)['timerLimited'])
        self.assertIsNone(self.stats(a=99, b=100)['medianImprovement'])
        self.assertFalse(report.envelope(self.stats(a=99, b=100), 1000))
        self.assertFalse(self.stats(a=100, b=100)['timerLimited'])
        self.assertTrue(self.stats(resolution=101)['timerLimited'])
        self.assertFalse(self.stats(a=None, b=None, count=0)['applicable'])
        with self.assertRaises(ValueError): self.stats(a=None, b=10, count=0)
        with self.assertRaises(ValueError): self.stats(a=None, b=None, count=2)
        with self.assertRaises(ValueError): report.group_value([sample(100, n) for n in (2, 2, 3, 2)], 'ABBA')
        rows = [sample(1000), sample(1000), sample(1000), sample(0)]
        self.assertFalse(report.summarize([rows]*40, [list(range(40))], ['ABBA','BAAB']*20)['timerLimited'])

    def test_extremes_and_percentiles(self):
        orders = ['ABBA']*20 + ['BAAB']*20
        groups = [[sample(1000 + (i if arm == 'B' else 0)) for arm in order] for i, order in enumerate(orders)]
        groups[-1] = [sample(10**18), sample(1000), sample(1000), sample(10**18)]
        indices = [[i]*40 for i in range(40)]
        c = report.summarize(groups, indices, orders)
        self.assertEqual(c['intervals95']['overheadPerParagraphNs'], [0,19])
        self.assertEqual(c['descriptive']['maximum'], (10**18-1000)/2)
        with self.assertRaises(ValueError): report.summarize(groups[:-1], indices, orders)


class RecoveryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.base = Path(cls.temp.name) / 'base'
        synthetic(cls.base)
        with patch.object(run.supervisor, 'supervise', side_effect=fake_supervise), patch('builtins.print'):
            run.execute(cls.base, [])

    @classmethod
    def tearDownClass(cls): cls.temp.cleanup()

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name) / 'evidence'
        shutil.copytree(self.base, self.root)

    def tearDown(self): self.temp.cleanup()

    def journal_mutation(self, change):
        rows = run.journal(self.root / 'index.jsonl'); change(rows)
        (self.root / 'index.jsonl').write_text(''.join(json.dumps(r)+'\n' for r in rows))
        # Remove only the synthetic finished checkpoint to test record-level checks.
        (self.root / 'finished-checkpoint.json').unlink()

    def test_completed_resume_no_launch(self):
        with patch.object(run.supervisor, 'supervise', side_effect=AssertionError('launch')), \
             patch.object(run.supervisor.subprocess, 'Popen', side_effect=AssertionError('Popen')):
            destination = self.root.parent / 'resume'
            rows = run.resume(self.root, destination)
            run.execute(destination, rows)
        self.assertEqual(len(rows),322)

    def test_prefix_recovery(self):
        records = run.journal(self.root / 'index.jsonl')
        (self.root / 'finished-checkpoint.json').unlink()
        for record in records[3:]:
            (self.root / record['raw']).unlink(); (self.root / record['stderr']).unlink()
        for name in ('index.jsonl','attempts.jsonl'):
            lines = (self.root/name).read_text().splitlines()
            (self.root/name).write_text('\n'.join(lines[:3])+'\n')
        destination = self.root.parent/'prefix'
        with patch.object(run.supervisor, 'supervise', side_effect=fake_supervise) as launch, patch('builtins.print'):
            rows = run.resume(self.root,destination)
            run.execute(destination,rows)
        self.assertEqual(launch.call_count,319)
        self.assertEqual(launch.call_args_list[0].args[0],run.command(run.schedule()[3]))

    def test_raw_and_stderr_hashes(self):
        for key in ('raw','stderr'):
            with self.subTest(key=key):
                row=run.journal(self.root/'index.jsonl')[0]; path=self.root/row[key]
                before=path.read_bytes(); path.write_bytes(before+b' ')
                with self.assertRaises(ValueError): run.validate(self.root)
                path.write_bytes(before)

    def test_altered_identity(self):
        run.write(self.root/'identities.json',{str(run.BINARY):'wrong'})
        with self.assertRaises(ValueError): run.validate(self.root)
        with self.assertRaises(ValueError): run.diagnosis.verify_identities({str(run.BINARY):'wrong'})

    def test_failed_missing_duplicate(self):
        for mutate in (lambda rows: rows[3].update(status='failed'), lambda rows: rows.pop(),
                       lambda rows: rows.__setitem__(4,copy.deepcopy(rows[3])),
                       lambda rows: rows.__setitem__(slice(3,5), rows[3:5][::-1])):
            with self.subTest(mutation=mutate):
                original=(self.root/'index.jsonl').read_bytes()
                if not (self.root/'finished-checkpoint.json').exists():
                    shutil.copyfile(self.base/'finished-checkpoint.json',self.root/'finished-checkpoint.json')
                self.journal_mutation(mutate)
                with self.assertRaises(ValueError): run.validate(self.root, complete=True)
                (self.root/'index.jsonl').write_bytes(original)

    def test_ambiguous_launch_and_partial_journal(self):
        with (self.root/'attempts.jsonl').open('ab') as f:f.write(b'{')
        with self.assertRaises(ValueError): run.resume(self.root,self.root.parent/'bad')
        shutil.copyfile(self.base/'attempts.jsonl',self.root/'attempts.jsonl')
        (self.root/'finished-checkpoint.json').unlink()
        run.supervisor.append(self.root/'attempts.jsonl', {'id':'attempted-before-output'})
        with self.assertRaises(ValueError): run.resume(self.root,self.root.parent/'bad')

    def test_fixture_schedule_command_corruption(self):
        for name in (run.CASES[0]+'.json', 'schedule.json', 'commands.json'):
            path=self.root/name; before=path.read_bytes(); path.write_bytes(before+b' ')
            with self.assertRaises(ValueError): run.validate(self.root)
            path.write_bytes(before)

    def test_launch_exception_is_terminal(self):
        fresh=self.root.parent/'fresh'; synthetic(fresh)
        with patch.object(run.supervisor, 'supervise', side_effect=RuntimeError('interrupted Popen')):
            with self.assertRaises(RuntimeError): run.execute(fresh, [])
        self.assertEqual(len(run.journal(fresh/'attempts.jsonl')),1)
        with self.assertRaises(ValueError): run.resume(fresh,self.root.parent/'retry')

    def test_existing_destination_and_incomplete_report(self):
        with self.assertRaises(ValueError): run.resume(self.root,self.root)
        self.journal_mutation(lambda rows:rows.pop())
        with self.assertRaises(ValueError): report.report(self.root)

    def test_single_reference_lifecycle(self):
        for e in run.schedule()[2:4]:
            run.validate_output(self.root/(e['id']+'-sample.jsonl'),e,self.root)
        e=run.schedule()[2]; path=self.root/(e['id']+'-sample.jsonl')
        rows=run.continuation.old.strict_rows(path)
        rows[1]['shapeDeltas'][0]=99
        path.write_text(''.join(json.dumps(r)+'\n' for r in rows))
        with self.assertRaises(AssertionError): run.validate_output(path,e,self.root)


if __name__ == '__main__': unittest.main()
