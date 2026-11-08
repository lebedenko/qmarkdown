#!/usr/bin/env python3
"""Offline regressions; benchmark and subprocess launch APIs are fatal."""
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
        batch=e['configuration']=='batch-96-enabled';dense=e['configuration']=='original-dense'
        records.append(dict(**e,parentStartMonotonicNs=1+e['sequence']*100000000,
            parentEndMonotonicNs=50000001+e['sequence']*100000000,peakObservedRssBytes=10000000,wallSeconds=.05))
        raw[e['id']]=[]
        for phase in (('prepared-reflow',) if batch else ('initial','reflow','repeat','metadata')):
            n=96 if batch else (0 if phase=='metadata' and not dense else 1)
            raw[e['id']].append(dict(phase=phase,applicableParagraphs=n,elapsedNs=n*1000000 if n else None,
                clockPeriodNs=1,minimumPositiveDeltaNs=20))
    return records,raw


def blank(out):
    out.mkdir();p.write(out/'frozen.json',{});p.write(out/'identities.json',{})
    p.write(out/'schedule.json',p.schedule());p.write(out/'environment.json',p.environment())
    for name in ('index.jsonl','attempts.jsonl'):(out/name).touch()


class Calibration(unittest.TestCase):
    def setUp(self):self.guard=p.a.no_launches();self.guard.__enter__()
    def tearDown(self):self.guard.__exit__(None,None,None)
    def test_schedule(self):
        s=p.schedule();es=s['commands'];self.assertEqual(s,p.schedule());self.assertEqual(len(es),483)
        self.assertEqual(len({e['id'] for e in es}),483)
        self.assertEqual([e['configuration'] for e in es[:3]],[c['id'] for c in s['configurations']])
        for c in s['configurations']:
            selected=[e for e in es[3:] if e['configuration']==c['id']]
            self.assertEqual(len(selected),160);self.assertEqual(len({tuple(e['argv']) for e in selected}),1)
            self.assertEqual(len({e['environment'] for e in selected}),1)
            orders=[]
            for rnd in range(40):
                group=[e for e in selected if e['round']==rnd];seq=[e['sequence'] for e in group]
                self.assertEqual(seq,list(range(seq[0],seq[0]+4)));self.assertEqual([e['positionInGroup'] for e in group],[0,1,2,3])
                orders.append(''.join(e['arm'] for e in group))
            for half in (orders[:20],orders[20:]):self.assertEqual(sorted(half),['ABBA']*10+['BAAB']*10)
        self.assertEqual(es[1]['argv'][-4:],['--batch-size','96','--operation-telemetry','enabled'])
    def test_zero_offsets_drift_and_normalization(self):
        rs,raw=synthetic();limits={'prose-480-4096':1,'dense-240-4096':1}
        result=q.analyze(rs,raw,limits);self.assertEqual(result['calibration'],'resolved-prepared-state-control')
        self.assertEqual(len(result['cells']),9)
        self.assertTrue(all(g['D']==0 for c in result['cells'] for g in c['groups'] if g['paragraphs']))
        self.assertIsNone(next(c for c in result['cells'] if c['configuration']=='original-reference' and c['operation']=='metadata')['interval95NsPerParagraph'])
        for e in rs:
            if e['mode']=='timing':
                for row in raw[e['id']]:
                    if row['elapsedNs'] is not None:
                        row['elapsedNs']+=row['applicableParagraphs']*(e['positionInGroup']*1000+(200 if e['arm']=='B' else 0))
        result=q.analyze(rs,raw,limits)
        self.assertEqual(result['calibration'],'unresolved')
        self.assertTrue(all(g['D']==200 for c in result['cells'] for g in c['groups'] if g['paragraphs']))
        self.assertEqual(q.describe([1,2,3,4])['mad'],1)
    def test_bootstrap_and_percentiles(self):
        samples=q.indices();self.assertEqual(samples,q.indices());self.assertEqual(len(samples),10000)
        self.assertTrue(all(len(s)==40 and all(0<=i<40 for i in s) for s in samples))
        # Construct known ordered bootstrap medians, independently fixing both indices.
        known=[[i%40]*40 for i in range(10000)]
        self.assertEqual(q.interval(list(range(40)),known),[0,38])
        self.assertEqual(q.interval([7]*40,samples),[7,7])
    def test_inclusive_boundary_timer_nonveto(self):
        cell=dict(applicable=True,timerLimited=False,interval95NsPerParagraph=[-10,10])
        self.assertTrue(q.envelope(cell,10));cell['interval95NsPerParagraph'][1]+=1e-9;self.assertFalse(q.envelope(cell,10))
        cell.update(interval95NsPerParagraph=[0,0],timerLimited=True);self.assertFalse(q.envelope(cell,10))
        self.assertEqual(q.decision([dict(requiredTarget=True,envelopeFit=True)]*2+[dict(requiredTarget=False,envelopeFit=False)]),'resolved-prepared-state-control')
    def test_ratio_suppression_and_timer_rule(self):
        rows=[dict(elapsedNs=2000,applicableParagraphs=1,clockPeriodNs=1,minimumPositiveDeltaNs=20) for _ in range(4)]
        self.assertEqual(q.group(rows,'ABBA')['relativeDifference'],0)
        rows[0]['elapsedNs']=1999;g=q.group(rows,'ABBA');self.assertIsNone(g['relativeDifference']);self.assertTrue(g['relativeDifferenceReason'])
        groups=[dict(g,order='ABBA' if i%2 else 'BAAB') for i in range(40)]
        groups[1]=dict(groups[1],relativeDifference=0,relativeDifferenceReason=None)
        c=q.summarize(groups,q.indices());self.assertEqual(c['descriptive']['ratios']['available'],1)
        self.assertEqual(c['descriptive']['ratios']['suppressed'],39)
        self.assertTrue(c['timerLimited']);self.assertNotIn('relativeDifference',c.get('interval95NsPerParagraph',{}))
        rows[1]['applicableParagraphs']=2
        with self.assertRaises((AssertionError,ValueError)):q.group(rows,'ABBA')
    def test_malformed_groups(self):
        rs,raw=synthetic()
        for changed in (rs[:-1],rs+[rs[-1]],rs[:3]+[rs[4],rs[3]]+rs[5:]):
            with self.assertRaises((ValueError,AssertionError)):q.analyze(changed,raw,{})
    def test_source_timing_and_telemetry(self):
        source=(p.HERE.parent/'candidate-scan-batching-refinement/trace.cpp').read_text()
        self.assertEqual(source.split('auto start=Clock::now();')[-1].split('auto end=Clock::now();')[0].strip(),'for(auto p:pointers)p->layout(497);')
        self.assertLess(source.index('cache->setChecker({})'),source.rindex('auto start=Clock::now();'))
        self.assertLess(source.index('if(enabled)pre=boundary()'),source.rindex('auto start=Clock::now();'))
        self.assertGreater(source.index('if(enabled)post=boundary()'),source.rindex('auto end=Clock::now();'))
        with patch.object(Path,'glob',return_value=[]),patch('os.getloadavg',side_effect=OSError('unavailable')):
            snap=p.snapshot();self.assertTrue(snap['load']['reason']);self.assertTrue(snap['thermal']['reason'])
    def test_lifecycle_optional_failures_and_deltas(self):
        e=next(e for e in p.schedule()['commands'] if e['configuration']=='batch-96-enabled' and e['mode']=='timing');n=96
        row=dict(phase='prepared-reflow',mode='timing',telemetry='enabled',applicableParagraphs=n,elapsedNs=96000,elapsedPerParagraphNs=1000,
            distinctInstances=True,generationUnchanged=True,sharedFontCollection=True,finalWidth=497,initialWidth=480,cacheCounts=[0,1,1],
            initialGenerations=[1]+[0]*95,finalGenerations=[1]+[0]*95,initialStates=[6]*n,finalStates=[6]*n,
            timerStartMonotonicNs=100,timerEndMonotonicNs=96100,clockPeriodNs=1,minimumPositiveDeltaNs=20,
            boundaryBefore=dict(startMonotonicNs=1,durationNs=1),boundaryAfter=dict(startMonotonicNs=96200,durationNs=1),
            boundaryDeltas=dict(processCpuNs=None,voluntary=None,involuntary=None))
        for k in ('boundaryBefore','boundaryAfter'):
            for m in ('cpu','processCpuNs','voluntary','involuntary'):row[k][m]=dict(value=None,reason='mock unavailable')
        with tempfile.TemporaryDirectory() as tmp:
            path=Path(tmp)/'raw'
            def validate(value):path.write_text(json.dumps(value)+'\n');return p.validate_raw(path,e,Path(tmp))
            validate(row)
            for mutation in ('overlap','reason','generation','state','normalization','cache','delta','negative','elapsed','width'):
                changed=copy.deepcopy(row)
                if mutation=='overlap':changed['boundaryBefore']['durationNs']=100
                elif mutation=='reason':changed['boundaryAfter']['cpu']['reason']=None
                elif mutation=='generation':changed['finalGenerations'][0]+=1
                elif mutation=='state':changed['finalStates'][0]=5
                elif mutation=='normalization':changed['elapsedPerParagraphNs']=1
                elif mutation=='cache':changed['cacheCounts']=[0,2,2]
                elif mutation=='delta':changed['boundaryDeltas']['processCpuNs']=3
                elif mutation=='negative':
                    for k,v in (('boundaryBefore',10),('boundaryAfter',1)):changed[k]['processCpuNs']=dict(value=v,reason=None)
                    changed['boundaryDeltas']['processCpuNs']=-9
                elif mutation=='elapsed':changed['timerEndMonotonicNs']+=1
                else:changed['finalWidth']=498
                with self.assertRaises((AssertionError,ValueError,TypeError)):validate(changed)
    def test_journals_partial_hashes_and_recovery(self):
        with tempfile.TemporaryDirectory() as tmp:
            out=Path(tmp)/'out';blank(out);self.assertEqual(p.validate(out),[])
            resumed=Path(tmp)/'prefix';self.assertEqual(p.resume(out,resumed),[])
            with patch.object(p,'environment',return_value={}):
                with self.assertRaises((ValueError,AssertionError)):p.resume(out,Path(tmp)/'mismatch')
            for text in ('{"id":"ambiguous"}\n','{"id":'):
                (out/'attempts.jsonl').write_text(text)
                with self.assertRaises(Exception):p.resume(out,Path(tmp)/'invalid')
            (out/'attempts.jsonl').write_text('');(out/'000-partial.stderr').write_text('partial')
            with self.assertRaises(Exception):p.validate(out)
            p.write(out/'frozen.json',{'schedule.json':'0'*64})
            with self.assertRaises(Exception):p.validate(out)
    def test_mock_orchestration_correctness_gate_and_completed_resume(self):
        with tempfile.TemporaryDirectory() as tmp:
            out=Path(tmp)/'out';blank(out);launched=[]
            def mock(cmd,raw,err,**kwargs):
                launched.append(cmd);raw.write_text('{}\n');err.write_text('')
                return dict(status='completed',returncode=0,command=cmd,peakObservedRssBytes=1000,wallSeconds=.001)
            with patch.object(p.r.supervisor,'supervise',side_effect=mock),patch.object(p,'validate_raw',return_value=[]),patch.object(p,'snapshot',return_value=dict(durationNs=0,startMonotonicNs=1)):
                p.execute(out,[]);self.assertEqual(launched,[e['argv'] for e in p.schedule()['commands']]);entries=p.validate(out,True)
                prefix=Path(tmp)/'prefix';blank(prefix)
                for name in ('index.jsonl','attempts.jsonl'):
                    (prefix/name).write_text('\n'.join((out/name).read_text().splitlines()[:3])+'\n')
                for entry in entries[:3]:
                    for suffix in ('.jsonl','.stderr'):(prefix/(entry['id']+suffix)).write_bytes((out/(entry['id']+suffix)).read_bytes())
                self.assertEqual(len(p.resume(prefix,Path(tmp)/'prefix-resume')),3)
                with patch.object(p.r.supervisor,'supervise',side_effect=AssertionError('fatal launch')),patch.object(p,'environment',side_effect=AssertionError('completed environment check')):
                    self.assertEqual(len(p.resume(out,Path(tmp)/'completed')),483)
                for mutation in ('missing','duplicate','misordered','failed'):
                    changed=copy.deepcopy(entries)
                    if mutation=='missing':changed.pop()
                    elif mutation=='duplicate':changed[1]=changed[0]
                    elif mutation=='misordered':changed[0],changed[1]=changed[1],changed[0]
                    else:changed[0]['status']='failed'
                    (out/'index.jsonl').write_text(''.join(json.dumps(v)+'\n' for v in changed))
                    with self.assertRaises(Exception):p.validate(out)
            # A real validation failure in the first correctness output stops at one attempt.
            fail=Path(tmp)/'fail';blank(fail);launched.clear()
            with patch.object(p.r.supervisor,'supervise',side_effect=mock),patch.object(p,'validate_raw',side_effect=ValueError('mock invalid')):
                with self.assertRaises(Exception):p.execute(fail,[])
            self.assertEqual(len(launched),1)
            with self.assertRaises(Exception):p.resume(fail,Path(tmp)/'retry')
    def test_terminal_resource_supervision(self):
        with tempfile.TemporaryDirectory() as tmp:
            for status,rss,wall in (('timeout',1000,60.01),('rss-limit',4*1024**3+1,.01),('failed',1000,.01),('completed',0,.01)):
                out=Path(tmp)/status;blank(out);calls=[]
                def mock(cmd,raw,err,**kwargs):
                    calls.append(cmd);raw.write_text('{}\n');err.write_text('')
                    self.assertEqual((kwargs['timeout'],kwargs['rss_limit']),(60,4*1024**3))
                    return dict(status=status,returncode=0 if status=='completed' else -9,command=cmd,peakObservedRssBytes=rss,wallSeconds=wall)
                with patch.object(p.r.supervisor,'supervise',side_effect=mock),patch.object(p,'validate_raw',return_value=[]):
                    with self.assertRaises(Exception):p.execute(out,[])
                self.assertEqual(len(calls),1)
                with self.assertRaises(Exception):p.resume(out,Path(tmp)/(status+'-retry'))

    def test_forensic_archive_readback(self):
        import package
        with tempfile.TemporaryDirectory() as tmp:
            out=Path(tmp)/'out';out.mkdir();(out/'partial.jsonl').write_text('partial')
            fake=Path(tmp)/'binary';fake.write_bytes(b'reused executable stand-in')
            with patch.object(p,'BINARY',fake),patch.object(p.r,'BINARY',fake):
                package.package(out,Path(tmp)/'archive')
            self.assertTrue((Path(tmp)/'archive/archives.json').exists())
            self.assertEqual(json.loads((Path(tmp)/'archive/decision.json').read_text())['calibration'],'incomplete')

    def test_forensic_analysis_failure_preserves_bytes(self):
        with tempfile.TemporaryDirectory() as tmp:
            out=Path(tmp);(out/'attempts.jsonl').write_text('{"id":"attempt"}\n');(out/'partial.jsonl').write_text('partial')
            result=q.report(out);self.assertEqual(result['calibration'],'incomplete');self.assertFalse(result['retentionEstablished'])
            self.assertIn('partial.jsonl',result['preservedFiles']);self.assertEqual((out/'partial.jsonl').read_text(),'partial')
            self.assertTrue(result['validationErrors'])
            with patch.object(p,'validate',return_value=[{'id':'observed'}]*483),patch.object(p,'rows',side_effect=ValueError('analysis failure')):
                result=q.report(out);self.assertEqual(result['calibration'],'incomplete')
                self.assertIn('analysis failure',result['analysisError'])

if __name__=='__main__':unittest.main()
