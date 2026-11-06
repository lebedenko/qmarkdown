"""Rejection tests for fixed schedule, terminal recovery and exact comparisons."""
import copy
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import run as runner

class HarnessTests(unittest.TestCase):
    def test_schedule(self):
        s=runner.schedule()
        self.assertEqual(len(s),72);self.assertEqual(len({e['id'] for e in s}),72)
        self.assertEqual([e['mode'] for e in s[:3]],['oracle','shadow','prototype'])

    def prior(self,root):
        p=root/'prior';p.mkdir()
        runner.recovery.durable_json(p/'identities.json',{})
        runner.recovery.durable_json(p/'schedule.json',runner.schedule())
        (p/'index.jsonl').touch();return p

    def test_terminal_unacknowledged_and_recorded_failures(self):
        with tempfile.TemporaryDirectory() as d,patch.object(runner,'pinned',return_value={}):
            root=Path(d);p=self.prior(root);e=runner.schedule()[0]
            raw=p/(e['id']+'-trace.jsonl');raw.write_text('{"phase":')
            with self.assertRaises(AssertionError):runner.resume(p,root/'next')
            fixture=p/(e['case']+'.json');fixture.write_text('{}')
            r=dict(**e,status='failed',raw=raw.name,rawSha256=runner.digest(raw),fixtureSha256=runner.digest(fixture))
            (p/'index.jsonl').write_text(json.dumps(r)+'\n')
            with self.assertRaises(AssertionError):runner.resume(p,root/'next')

    def test_changed_identity_checkpoint_and_existing_destination(self):
        with tempfile.TemporaryDirectory() as d,patch.object(runner,'pinned',return_value={}):
            root=Path(d);p=self.prior(root)
            with self.assertRaises(AssertionError):runner.resume(p,p)
            runner.recovery.checkpoint(p,'build',[p/'schedule.json'])
            (p/'schedule.json').write_text('[]')
            with self.assertRaises(ValueError):runner.resume(p,root/'next')
            runner.recovery.durable_json(p/'identities.json',{str(p/'index.jsonl'):'bad'})
            with self.assertRaises(AssertionError):runner.resume(p,root/'next')

    def rows(self):
        scan=dict(evaluations=2,skipped=0,scratchSize=2,scratchCapacity=3,
                  preprocessing=2,trimming=0,shadowChecks=0,fallback=[0]*7)
        bucket=dict(calls=2,ownerComparisons=0)
        work=dict(scan=scan,candidates=2,fitting=bucket,formatting=dict(ownerComparisons=0),ownerBuilds=0,ownerExaminations=0)
        return [dict(phase=p,geometry=[{'runs':[]}],shapeDeltas=[0],counters=[copy.deepcopy(work)]) for p in runner.PHASES]

    def test_differential_rejects_geometry_shaping_work_and_fallback(self):
        with tempfile.TemporaryDirectory() as d,patch.object(runner,'ROOT',Path(d)):
            root=Path(d);out=root/'out';out.mkdir();baseline=root/'build-ci/073-runtime-evidence';baseline.mkdir(parents=True)
            e=dict(case='case',variant='padded',mode='prototype')
            original=self.rows()
            path=baseline/'correctness-00-case-padded-baseline-sample.jsonl'
            path.write_text(''.join(json.dumps(r)+'\n' for r in original))
            (out/'case-padded-oracle-trace.jsonl').write_bytes(path.read_bytes())
            raw=out/'case-padded-prototype-trace.jsonl'
            for mutate in (lambda r:r[0].update(geometry=[]),lambda r:r[0].update(shapeDeltas=[1]),
                           lambda r:r[0]['counters'][0]['scan'].update(evaluations=1),
                           lambda r:r[0]['counters'][0]['scan'].update(fallback=[0,1,0,0,0,0,0])):
                rows=copy.deepcopy(original);mutate(rows);raw.write_text(''.join(json.dumps(r)+'\n' for r in rows))
                with self.assertRaises(AssertionError):runner.validate(raw,e,out)
            raw.write_bytes(path.read_bytes());runner.validate(raw,e,out)
            raw.write_bytes(raw.read_bytes().rstrip(b'\n'))
            with self.assertRaises(AssertionError):runner.validate(raw,e,out)

if __name__=='__main__':unittest.main()
