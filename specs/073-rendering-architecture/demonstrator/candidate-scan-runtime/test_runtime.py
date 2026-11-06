#!/usr/bin/env python3
import copy
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import run as runner
report = runner.module('candidate_runtime_report', Path(__file__).with_name('report.py'))


def row(ns, count=1, resolution=1):
    return dict(elapsedNs=ns,applicableParagraphs=count,clockPeriodNs=1,minimumPositiveDeltaNs=resolution)


def summary(a=100000,b=89000,count=1):
    return report.summarize([(row(a,count),row(b,count))]*20,[[i]*20 for i in range(20)])


class RuntimeTests(unittest.TestCase):
    def test_schedule(self):
        s=runner.schedule()
        self.assertEqual(len(s),1848)
        self.assertEqual(sum(e['mode']=='correctness' for e in s),88)
        self.assertEqual(len({e['id'] for e in s}),len(s))
        for a,b in zip(s[::2],s[1::2]):
            self.assertEqual(a['pair'],b['pair'])
            self.assertEqual([a['label'],b['label']],['baseline','retained'] if a['sample']%2==0 else ['retained','baseline'])
        self.assertTrue(all(e['mode']=='timing' for e in s[88:]))

    def test_fixtures(self):
        from workloads import units
        cases=runner.fixtures()
        self.assertEqual(len(cases),22)
        for c in cases:
            if c['id'] in runner.SMALL:
                self.assertEqual(len(c['blocks']),32)
                self.assertEqual(len({id(b) for b in c['blocks']}),32)
                self.assertTrue(all(units(b['text'])==c['size'] for b in c['blocks']))
                self.assertEqual(c['width'],480)
            else:
                with tempfile.TemporaryDirectory() as d:
                    p=Path(d)/(c['id']+'.json');runner.recovery.durable_json(p,runner.manifest(c))
                    self.assertEqual(p.read_bytes(),(runner.BASELINE/p.name).read_bytes())

    def test_statistics(self):
        s=summary()
        self.assertEqual(s['medianDifferenceNs'],-11000)
        self.assertEqual(s['medianImprovement'],.11)
        self.assertEqual(s['intervals95']['medianImprovement'],[.11,.11])
        self.assertEqual(summary(count=32)['overheadPerParagraphNs'],-11000/32)

    def test_timer(self):
        s=summary(9999,9999)
        self.assertFalse(s['timerLimited'])
        s=report.summarize([(row(99),row(98))]*20,[[0]*20]*20)
        self.assertTrue(s['timerLimited']);self.assertIsNone(s['medianImprovement'])
        self.assertEqual(s['overheadPerParagraphNs'],-1)
        s=report.summarize([(row(100),row(100))]*20,[[0]*20]*20)
        self.assertFalse(s['timerLimited'])

    def test_gates(self):
        c=dict(case='long-480-16384',variant='padded',operation='initial',**summary(100000,90000))
        self.assertEqual(report.gate(c),'pass')
        c['medianImprovement']=.099999;self.assertEqual(report.gate(c),'primary-fail')
        c['medianImprovement']=.1;c['intervals95']['medianImprovement'][0]=0
        self.assertEqual(report.gate(c),'primary-fail')
        c['timerLimited']=True;self.assertEqual(report.gate(c),'timer-limited-primary')
        c=dict(case='small-dense-480-64',variant='zero',operation='repeat',**summary(1000,3000))
        self.assertEqual(report.gate(c),'pass')
        c['intervals95']['overheadPerParagraphNs'][1]=2000.01
        self.assertEqual(report.gate(c),'regression-fail')
        c['completedPairs']=19;self.assertEqual(report.gate(c),'incomplete')
        n=report.summarize([(row(None,0),row(None,0))],[])
        self.assertEqual(report.gate(dict(n)),'not-applicable')

    def test_exact_output_rejection(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);c=next(c for c in runner.fixtures() if c['id']=='prose-480-4096')
            runner.recovery.durable_json(root/(c['id']+'.json'),runner.manifest(c))
            runner.recovery.durable_json(root/'environment.json',dict(affinity=[0]))
            raw=root/'raw.jsonl';entry=dict(case=c['id'],variant='padded',mode='correctness',label='baseline')
            rows=runner.old.strict_rows(runner.BASELINE/f"correctness-00-{c['id']}-padded-baseline-sample.jsonl")
            for r in rows:r.update(cpuAffinity=[0],fpEnvironment=dict(rounding=0,mxcsr=8064))
            def write():raw.write_text(''.join(json.dumps(r)+'\n' for r in rows))
            write();runner.validate_output(raw,entry,root)
            rows[0]['geometry'][0]['lines'][0][2]+=1;write()
            with self.assertRaisesRegex(AssertionError,'geometry'):runner.validate_output(raw,entry,root)
            rows[0]['shapeDeltas']=[0];write()
            with self.assertRaisesRegex(AssertionError,'shaping'):runner.validate_output(raw,entry,root)

    def test_missing_samples(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);runner.recovery.durable_json(root/'schedule.json',runner.schedule());(root/'index.jsonl').touch()
            data=report.report(root,resamples=10)
            self.assertFalse(data['retentionEstablished'])
            self.assertTrue(all(c['gate']=='incomplete' for c in data['cells']))

    def test_regression_relative_boundary(self):
        c=dict(case='dense-480-4096',variant='padded',operation='reflow',**summary(100000,105000))
        self.assertEqual(report.gate(c),'pass')
        self.assertEqual(c['overheadLimitPerParagraphNs'],5000)
        c['intervals95']['overheadPerParagraphNs'][1]=5000.001
        self.assertEqual(report.gate(c),'regression-fail')

    def test_resolution_and_pair_alignment(self):
        pairs=[(row(1000+i*100,32,20),row(800+i*100,32,20)) for i in range(20)]
        indices=[[i]*20 for i in range(20)]
        a=report.summarize(pairs,indices)
        self.assertTrue(a['timerLimited'])
        self.assertEqual(a['empiricalResolutionNs'],20)
        self.assertIsNone(a['intervals95']['medianImprovement'])
        self.assertEqual(a['intervals95']['medianDifferenceNs'],[-200,-200])
        # Reusing indices in a second operation preserves pair-level correlations.
        b=report.summarize([(row(x['elapsedNs']*2,32,20),row(y['elapsedNs']*2,32,20)) for x,y in pairs],indices)
        self.assertEqual(b['intervals95']['medianDifferenceNs'],[-400,-400])
        with self.assertRaises(AssertionError):
            report.summarize([(row(100),row(None,0))],[[0]])

    def test_terminal_failure(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);e=runner.schedule()[0];runner.recovery.durable_json(root/(e['case']+'.json'),{})
            def fail(cmd,raw,stderr):
                raw.touch();return dict(status='failed')
            with patch.object(runner.supervisor,'supervise',side_effect=fail) as launch:
                with self.assertRaises(AssertionError):runner.execute(root,root,[])
                self.assertEqual(launch.call_count,1)

if __name__=='__main__':unittest.main()
