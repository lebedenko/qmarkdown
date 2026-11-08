#!/usr/bin/env python3
"""Synthetic preflight only: benchmark launch APIs are fatal throughout."""
import copy
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import run as p
import report as q


def synthetic():
    records=[];raw={}
    for e in p.schedule()['commands']:
        n=1 if e['configuration']=='original-reference' else int(e['configuration'].split('-')[1])
        records.append(dict(**e,parentStartMonotonicNs=1+e['sequence']*100000000,parentEndMonotonicNs=50000001+e['sequence']*100000000,
                            peakObservedRssBytes=10000000,wallSeconds=.05))
        phases=('initial','reflow','repeat','metadata') if e['configuration']=='original-reference' else ('prepared-reflow',)
        raw[e['id']]=[dict(phase=phase,applicableParagraphs=0 if phase=='metadata' else n,elapsedNs=None if phase=='metadata' else n*1000000,
                          clockPeriodNs=1,minimumPositiveDeltaNs=20,
                          boundaryBefore=dict(durationNs=10) if e['configuration'].endswith('enabled') else None,
                          boundaryAfter=dict(durationNs=10) if e['configuration'].endswith('enabled') else None) for phase in phases]
    return records,raw


class Pilot(unittest.TestCase):
    def setUp(self):
        self.guard=p.a.no_launches();self.guard.__enter__()
    def tearDown(self):self.guard.__exit__(None,None,None)
    def test_schedule(self):
        schedule=p.schedule();entries=schedule['commands'];self.assertEqual(len(entries),297)
        draft=p.a.pilot_schedule()['commands']
        for original,e in zip(draft,entries):
            expected=copy.deepcopy(original)
            if expected['argv'][0].startswith('<'):expected['argv'][0]=str(p.BINARY)
            self.assertEqual({k:v for k,v in e.items() if k!='id'},expected)
        self.assertEqual(len({e['id'] for e in entries}),297)
        self.assertEqual([e['configuration'] for e in entries[:9]],[c['id'] for c in schedule['configurations']])
        for c in schedule['configurations']:
            selected=[e for e in entries[9:] if e['configuration']==c['id']]
            self.assertEqual(len(selected),32)
            self.assertEqual(len({tuple(e['argv']) for e in selected}),1)
            orders=[]
            for rnd in range(8):
                group=[e for e in selected if e['round']==rnd];seq=[e['sequence'] for e in group]
                self.assertEqual(seq,list(range(seq[0],seq[0]+4)));orders.append(''.join(e['arm'] for e in group))
            for half in (orders[:4],orders[4:]):self.assertEqual(sorted(half),['ABBA','ABBA','BAAB','BAAB'])
    def test_selection_boundaries(self):
        cells={f'batch-{n}-enabled':dict(batchTiming=dict(count=32,median=10000000,mad=1000000),maximumParentDurationNs=29999999999,maximumSampledRssBytes=1073741823) for n in (1,16,64,256)}
        self.assertEqual(q.select(cells,True),1);self.assertIsNone(q.select(cells,False))
        for key,value in (('median',9999999),('mad',1000001),('count',31)):
            changed=copy.deepcopy(cells);changed['batch-1-enabled']['batchTiming'][key]=value
            self.assertEqual(q.select(changed,True),16)
        for key,value in (('maximumParentDurationNs',30000000000),('maximumSampledRssBytes',1073741824)):
            changed=copy.deepcopy(cells)
            for c in changed.values():c[key]=value
            self.assertIsNone(q.select(changed,True))
    def test_analysis_nulls_normalization_drift_offsets(self):
        rs,raw=synthetic();result=q.analyze(rs,raw,True)
        self.assertEqual(result['selection'],16)
        self.assertIsNone(result['cells']['original-reference']['operations']['metadata']['timing'])
        self.assertTrue(all(g['D']==0 for g in result['groups'] if g['paragraphs']))
        self.assertTrue(all(g['meanNsPerParagraph']==1000000 for g in result['groups'] if g['paragraphs']))
        for e in rs:
            if e['mode']=='timing' and e['configuration']!='original-reference':
                raw[e['id']][0]['elapsedNs']+=e['positionInGroup']*10000+(100000 if e['configuration'].endswith('enabled') else 0)
        result=q.analyze(rs,raw,True)
        self.assertTrue(all(g['D']==0 for g in result['groups'] if g['configuration']!='original-reference'))
        self.assertTrue(all(v['differenceNs']==100000 for c in result['telemetryComparisons'] for v in c['rounds']))
        self.assertEqual(q.describe([1,2,3,4])['mad'],1)
        self.assertFalse(q.qualified([1999],20));self.assertTrue(q.qualified([2000],20));self.assertFalse(q.qualified([None],20))
    def test_groups_missing_duplicate_misordered(self):
        rs,raw=synthetic()
        for altered in (rs[:10]+rs[11:],rs+[rs[9]],rs[:9]+[rs[10],rs[9]]+rs[11:]):
            with self.assertRaises((AssertionError,ValueError)):q.analyze(altered,raw,True)
    def test_mock_telemetry_and_failures(self):
        with patch.object(Path,'glob',return_value=[]),patch('os.getloadavg',side_effect=OSError('unavailable')):
            s=p.snapshot();self.assertIsNone(s['load']['value']);self.assertTrue(s['load']['reason']);self.assertTrue(s['thermal']['reason']);self.assertGreaterEqual(s['durationNs'],0)
    def test_timing_source_boundaries(self):
        source=(p.HERE/'trace.cpp').read_text();timed=source.split('auto start=Clock::now();')[-1].split('auto end=Clock::now();')[0]
        self.assertEqual(timed.strip(),'for(auto p:pointers)p->layout(497);')
        self.assertLess(source.index('cache->setChecker({})'),source.rindex('auto start=Clock::now();'))
        self.assertLess(source.index('if(enabled)pre=boundary()'),source.rindex('auto start=Clock::now();'))
        self.assertGreater(source.index('if(enabled)post=boundary()'),source.rindex('auto end=Clock::now();'))
    def test_attempt_journals_and_hashes(self):
        with tempfile.TemporaryDirectory() as tmp:
            out=Path(tmp)/'original';out.mkdir();p.write(out/'frozen.json',{});p.write(out/'identities.json',{});p.write(out/'schedule.json',p.schedule())
            (out/'index.jsonl').touch();(out/'attempts.jsonl').touch()
            self.assertEqual(p.validate(out),[])
            with self.assertRaises((AssertionError,ValueError)):p.validate(out,True)
            p.r.supervisor.append(out/'attempts.jsonl',dict(id='ambiguous'))
            with self.assertRaises((AssertionError,ValueError)):p.validate(out)
            (out/'attempts.jsonl').write_text('');p.write(out/'frozen.json',{'schedule.json':'0'*64})
            with self.assertRaises((AssertionError,ValueError)):p.validate(out)
    def test_raw_lifecycle_optional_nulls_and_timer_overlap(self):
        e=next(e for e in p.schedule()['commands'] if e['configuration']=='batch-16-enabled' and e['mode']=='timing')
        row=dict(phase='prepared-reflow',mode='timing',telemetry='enabled',applicableParagraphs=16,elapsedNs=16000,elapsedPerParagraphNs=1000,
                 distinctInstances=True,generationUnchanged=True,sharedFontCollection=True,finalWidth=497,initialWidth=480,
                 cacheCounts=[0,1,1],initialGenerations=[1]+[0]*15,finalGenerations=[1]+[0]*15,initialStates=[6]*16,finalStates=[6]*16,
                 timerStartMonotonicNs=100,timerEndMonotonicNs=16100,clockPeriodNs=1,minimumPositiveDeltaNs=20,boundaryDeltas={},
                 boundaryBefore=dict(startMonotonicNs=1,durationNs=1),boundaryAfter=dict(startMonotonicNs=16200,durationNs=1))
        for key in ('boundaryBefore','boundaryAfter'):
            for metric in ('cpu','processCpuNs','voluntary','involuntary'):row[key][metric]=dict(value=None,reason='mock unavailable')
        with tempfile.TemporaryDirectory() as tmp:
            raw=Path(tmp)/'raw';p.write(raw,row) # JSONL requires a single physical line.
            raw.write_text(json.dumps(row)+'\n');p.validate_raw(raw,e,Path(tmp))
            for mutation in ('overlap','null reason','generation','state','normalization','telemetry'):
                changed=copy.deepcopy(row)
                if mutation=='overlap':changed['boundaryBefore']['durationNs']=100
                elif mutation=='null reason':changed['boundaryAfter']['cpu']['reason']=None
                elif mutation=='generation':changed['finalGenerations'][0]+=1
                elif mutation=='state':changed['finalStates'][0]=5
                elif mutation=='normalization':changed['elapsedPerParagraphNs']=999
                else:changed['boundaryBefore']=None
                raw.write_text(json.dumps(changed)+'\n')
                with self.assertRaises((AssertionError,ValueError,TypeError)):p.validate_raw(raw,e,Path(tmp))

    def test_full_mock_orchestration_and_terminal_failure(self):
        with tempfile.TemporaryDirectory() as tmp:
            out=Path(tmp)/'original';out.mkdir();p.write(out/'frozen.json',{});p.write(out/'identities.json',{});p.write(out/'schedule.json',p.schedule());p.write(out/'environment.json',p.environment())
            (out/'index.jsonl').touch();(out/'attempts.jsonl').touch()
            launched=[]
            def mock(cmd,raw,err,**kwargs):
                launched.append(cmd);raw.write_text('{}\n');err.write_text('')
                return dict(status='completed',returncode=0,command=cmd,peakObservedRssBytes=1000,wallSeconds=.001)
            with patch.object(p.r.supervisor,'supervise',side_effect=mock),patch.object(p,'validate_raw',return_value=[]),patch.object(p,'snapshot',return_value=dict(durationNs=0,startMonotonicNs=1)):
                p.execute(out,[])
                self.assertEqual(launched,[e['argv'] for e in p.schedule()['commands']])
                p.validate(out,True)
                entries=p.rows(out/'index.jsonl')
                # Fully validated completed-run resume never reaches the supervisor.
                resumed=Path(tmp)/'resume'
                with patch.object(p.r.supervisor,'supervise',side_effect=AssertionError('forbidden resume launch')):
                    self.assertEqual(len(p.resume(out,resumed)),297)
                for mutation in ('missing','duplicate','misordered','failed'):
                    changed=copy.deepcopy(entries)
                    if mutation=='missing':changed.pop()
                    elif mutation=='duplicate':changed[1]=changed[0]
                    elif mutation=='misordered':changed[0],changed[1]=changed[1],changed[0]
                    else:changed[0]['status']='failed'
                    (out/'index.jsonl').write_text(''.join(json.dumps(v)+'\n' for v in changed))
                    with self.assertRaises((AssertionError,ValueError)):p.validate(out)
                (out/'index.jsonl').write_text(''.join(json.dumps(v)+'\n' for v in entries))
                (out/(entries[0]['id']+'.jsonl')).write_text('altered')
                with self.assertRaises((AssertionError,ValueError)):p.validate(out)

    def test_completed_resume_no_launch(self):
        with tempfile.TemporaryDirectory() as tmp:
            src=Path(tmp)/'src';src.mkdir();(src/'preserved').write_text('bytes')
            with patch.object(p,'validate',return_value=[{}]*297),patch.object(p,'environment',side_effect=AssertionError('unexpected environment requirement')):
                self.assertEqual(len(p.resume(src,Path(tmp)/'out')),297)
            self.assertEqual((Path(tmp)/'out/preserved').read_text(),'bytes')

if __name__=='__main__':unittest.main()
