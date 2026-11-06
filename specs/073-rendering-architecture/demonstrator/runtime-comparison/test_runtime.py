#!/usr/bin/env python3
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import run
spec=run.importlib.util.spec_from_file_location('runtime_report',Path(__file__).with_name('report.py'))
report=run.importlib.util.module_from_spec(spec);spec.loader.exec_module(report)
summarize=report.summarize


class RuntimeTests(unittest.TestCase):
    def test_schedule(self):
        entries=run.schedule()
        self.assertEqual(len(entries),528)
        self.assertEqual(len({e['id'] for e in entries}),528)
        self.assertEqual(sum(e['mode']=='correctness' for e in entries),48)
        for i in range(0,len(entries),2):
            a,b=entries[i:i+2]
            self.assertEqual(a['pair'],b['pair'])
            self.assertEqual((a['order'],b['order']),(0,1))
            self.assertEqual(a['label'],'baseline' if a['sample']%2==0 else 'optimized')
        for case in run.IDS:
            rows=[e for e in entries if e['case']==case and e['variant']=='zero' and e['mode']=='timing' and e['order']==0]
            self.assertEqual(sum(e['label']=='baseline' for e in rows),5)

    def test_timing_boundaries(self):
        source=(run.HERE/'trace.cpp').read_text()
        start=source.index('auto start=Clock::now();')
        stop=source.index('auto elapsed=')
        interval=source[start:stop]
        self.assertNotIn('experimentalSpans()',interval)
        self.assertNotIn('geometry(',interval)
        self.assertNotIn('experimentalShapeGeneration()',interval)
        self.assertLess(source.index('copies.back()[0].identity+='),start)
        self.assertIn('if(correctness)geometries.push_back',source)
        self.assertIn('applicable.empty()?json(nullptr)',source)
        self.assertEqual(interval.count('experimentalSetSpans'),1)

    def test_partial_and_malformed(self):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)/'raw'
            for data in ('{}','{}\n','bad\n','{"phase":"initial"}\n'):
                p.write_text(data)
                with self.assertRaises((AssertionError,ValueError,KeyError)):run.strict_rows(p)

    def test_paired_bootstrap(self):
        def row(n):return dict(elapsedNs=n,clockPeriodNs=1,minimumPositiveDeltaNs=10)
        pairs=[(row(10000+i*1000),row(5000+i*500)) for i in range(10)]
        a=summarize(pairs,resamples=1000)
        self.assertEqual(a,summarize(pairs,resamples=1000))
        self.assertEqual(a['medianRatio'],2)
        self.assertEqual(a['medianDifferenceNs'],-7250)
        self.assertEqual(a['intervals95']['medianRatio'],[2,2])
        self.assertEqual(summarize([])['completedPairs'],0)
        self.assertIsNone(summarize([(row(None),row(None))])['baselineMedianNs'])
        limited=summarize([(row(100),row(50))],resamples=10)
        self.assertTrue(limited['timerLimited'])
        self.assertIsNone(limited['medianRatio'])
        self.assertIsNone(limited['intervals95']['medianRatio'])

    def test_terminal_cell_serial(self):
        # A failed process terminates that fixture/variant across subsequent pairs.
        schedule=[dict(id=str(i),mode='timing',sample=i//2,case='x',variant='zero',label='baseline' if i%2==0 else 'optimized',pair=str(i//2),order=i%2) for i in range(4)]
        for status in ('failed','timeout','rss-limit','interrupted'):
            with self.subTest(status=status),tempfile.TemporaryDirectory() as tmp:
                out=Path(tmp);(out/'x.json').write_text('{}')
                (out/'identities.json').write_text('{}')
                calls=[]
                def supervise(command,raw,err):
                    calls.append(command);raw.write_text('partial');err.write_text('')
                    return dict(status=status)
                with patch.object(run,'schedule',return_value=schedule),patch.object(run.recovery,'phase_complete',return_value=True),patch.object(run.supervisor,'supervise',side_effect=supervise):
                    run.execute(out,out,out,[])
                self.assertEqual(len(calls),1)
                self.assertEqual(len(run.recovery.rows(out/'index.jsonl')),1)

    def test_operation_output_validation(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);fixture=root/'case.json';fixture.write_text('{}')
            entry=dict(case='case',variant='zero',mode='timing')
            expected=[dict(phase=p,shapeDeltas=[0 if p in ('reflow','repeat','metadata') else 1],geometry=[]) for p in run.PHASES]
            baseline=root/'case-zero-headline-trace.jsonl'
            baseline.write_text(''.join(json.dumps(r)+'\n' for r in expected))
            rows=[dict(phase=p,elapsedNs=None if p=='metadata' else 1000,
                       applicableParagraphs=0 if p=='metadata' else 1,
                       shapeDeltas=e['shapeDeltas'],clockPeriodNs=1,
                       minimumPositiveDeltaNs=15,highWaterRssBytes=4096) for p,e in zip(run.PHASES,expected)]
            raw=root/'raw'
            def write():raw.write_text(''.join(json.dumps(r)+'\n' for r in rows))
            write();run.validate_output(raw,entry,fixture,root)
            rows[-1]['elapsedNs']=0;write()
            with self.assertRaises(AssertionError):run.validate_output(raw,entry,fixture,root)
            rows[-1]['elapsedNs']=None;rows[1]['shapeDeltas']=[1];write()
            with self.assertRaisesRegex(AssertionError,'shaping'):run.validate_output(raw,entry,fixture,root)
            rows[1]['shapeDeltas']=[0];entry['mode']='correctness'
            for row in rows:row['geometry']=[]
            rows[0]['geometry']=[1];write()
            with self.assertRaisesRegex(AssertionError,'geometry'):run.validate_output(raw,entry,fixture,root)

    def test_missing_pairs_report(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            entry=dict(mode='timing',case='x',variant='zero',sample=0,label='baseline',status='completed',raw='raw')
            (root/'index.jsonl').write_text(json.dumps(entry)+'\n')
            (root/'schedule.json').write_text(json.dumps([entry]))
            (root/'raw').write_text(json.dumps(dict(highWaterRssBytes=4096))+'\n')
            result=report.report(root)
            self.assertEqual(len(result['cells']),4)
            for cell in result['cells']:
                self.assertEqual(cell['completedPairs'],0)
                self.assertEqual(cell['incompleteSamples'],list(range(10)))

    def test_gate_journal_cannot_change_build_checkpoint(self):
        with tempfile.TemporaryDirectory() as tmp:
            out=Path(tmp)
            with patch.object(run.subprocess,'run'):
                run.run_command(['build'],out,'build.log')
                run.recovery.checkpoint(out,'build',[out/'build-commands.jsonl'])
                run.run_command(['gate'],out,'gate.log',journal='gate-commands.jsonl')
            self.assertTrue(run.recovery.phase_complete(out,'build'))
            self.assertEqual(len(run.recovery.rows(out/'build-commands.jsonl')),1)
            self.assertEqual(len(run.recovery.rows(out/'gate-commands.jsonl')),1)

    def test_resume_rejects_changed_inputs_and_existing_destination(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);prior=root/'prior';prior.mkdir();out=root/'out'
            input_file=root/'input';input_file.write_text('old')
            (prior/'identities.json').write_text(json.dumps({str(input_file):run.digest(input_file)}))
            (prior/'schedule.json').write_text(json.dumps(run.schedule()))
            (prior/'index.jsonl').touch()
            run.resume(prior,out)
            with self.assertRaises(AssertionError):run.resume(prior,out)
            input_file.write_text('changed')
            with self.assertRaises(AssertionError):run.resume(prior,root/'out2')

    def test_invariant_failures_remain_terminal_on_resume(self):
        for error,diagnostic in [('geometry invariant failed',''),('shaping invariant failed',''),(None,'shaping invariant failed')]:
            with self.subTest(error=error,diagnostic=diagnostic),tempfile.TemporaryDirectory() as tmp:
                prior=Path(tmp)/'prior';prior.mkdir()
                (prior/'identities.json').write_text('{}')
                (prior/'schedule.json').write_text(json.dumps(run.schedule()))
                entry=run.schedule()[0]
                row=dict(**entry,status='failed',raw='raw.jsonl',validationError=error)
                (prior/'raw.stderr').write_text(diagnostic)
                (prior/'index.jsonl').write_text(json.dumps(row)+'\n')
                with self.assertRaisesRegex(AssertionError,'study stopped'):
                    run.resume(prior,Path(tmp)/'new')
                self.assertFalse((Path(tmp)/'new').exists())

    def test_resume_unacknowledged_raw_is_terminal(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);prior=root/'prior';prior.mkdir();out=root/'out'
            entry=run.schedule()[0]
            (prior/'identities.json').write_text('{}')
            (prior/'schedule.json').write_text(json.dumps(run.schedule()))
            (prior/'index.jsonl').write_text('{"interrupted":')
            (prior/(entry['case']+'.json')).write_text('{}')
            (prior/(entry['id']+'-sample.jsonl')).write_text('partial')
            rows=run.resume(prior,out)
            self.assertEqual(rows[0]['status'],'interrupted')
            self.assertEqual((out/'inherited-index.jsonl').read_text(),'{"interrupted":')

if __name__=='__main__':unittest.main()
