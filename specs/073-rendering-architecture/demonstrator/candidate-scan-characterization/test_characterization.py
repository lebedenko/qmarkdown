#!/usr/bin/env python3
"""Offline arithmetic, schedule and fail-closed integrity checks."""
import copy
import json
import hashlib
import io
import tarfile
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest.mock import patch
import analyze as a


def records(starts=(100, 120, 150, 190), duration=10, order='ABBA'):
    return [dict(id=f'p{i}', order=i, case='prose', sample=0, pair='g0', mode='timing', label=arm,
                 parentStartMonotonicNs=start, parentEndMonotonicNs=start+duration)
            for i, (start, arm) in enumerate(zip(starts, order))]


def raw(value, n=1):
    return dict(elapsedNs=value, applicableParagraphs=n, clockPeriodNs=1, minimumPositiveDeltaNs=1)


class ArithmeticTests(unittest.TestCase):
    def test_timestamp_arithmetic_unequal_spacing(self):
        rs = records()
        self.assertEqual(a.process_times(rs[0], 100)['processMidpointElapsedNs'], 5)
        self.assertEqual(a.process_times(rs[0], 100)['processDurationNs'], 10)
        g = a.group_times(rs, 100)
        self.assertEqual([g[k] for k in ('groupSpanNs','gap1Ns','gap2Ns','gap3Ns')], [100,10,20,30])
        self.assertEqual(g['midpointImbalanceNs'], -10)
        self.assertEqual(g['groupMidpointElapsedNs'], 45)
        huge = 10**16
        shifted = records(tuple(huge+x for x in (100,120,150,190)), duration=11)
        self.assertEqual(a.process_times(shifted[0], huge+100)['processMidpointElapsedNs'], 5.5)
        self.assertEqual(a.group_times(shifted, huge+100)['midpointImbalanceNs'], -10)

    def test_group_identity_and_order(self):
        for mutate in (lambda r:r.pop(), lambda r:r.reverse(),
                       lambda r:r[1].update(id=r[0]['id']), lambda r:r[1].update(case='other'),
                       lambda r:r[1].update(pair='other'), lambda r:r[1].update(label='A'),
                       lambda r:r[1].update(parentStartMonotonicNs=105)):
            rs=records(); mutate(rs)
            with self.assertRaises(ValueError): a.group_times(rs,100)
        with self.assertRaises(ValueError): a.process_times(dict(parentStartMonotonicNs=4,parentEndMonotonicNs=3),0)

    def test_ranks_ties_constants_nulls(self):
        self.assertEqual(a.ranks([30,10,10,20]), [4,1.5,1.5,3])
        self.assertAlmostEqual(a.spearman([1,1,2],[1,2,3]), 3**.5/2)
        self.assertEqual(a.spearman([1,2,3],[3,2,1]), -1)
        self.assertIsNone(a.spearman([1,1],[2,3]))
        self.assertIsNone(a.spearman([None,None],[2,3]))
        with self.assertRaises(ValueError): a.spearman([None,1],[2,3])
        with self.assertRaises(ValueError): a.spearman([1],[2,3])

    def test_offset_drift_normalization(self):
        for order in ('ABBA','BAAB'):
            rows=[raw(100+10*i+(20 if arm=='B' else 0),2) for i,arm in enumerate(order)]
            self.assertEqual(a.report.group_value(rows,order)['overheadPerParagraphNs'],10)
        rs=records()
        # Linear drift in real process time does not cancel unequal spacing.
        times=[a.process_times(r,100)['processMidpointElapsedNs'] for r in rs]
        self.assertEqual(a.report.group_value([raw(t+100) for t in times],'ABBA')['differenceNs'],-10)
        self.assertIsNone(a.report.group_value([raw(None,0)]*4,'ABBA')['overheadPerParagraphNs'])
        with self.assertRaises(ValueError): a.report.group_value([raw(None,0),raw(2),raw(2),raw(2)],'ABBA')

    def test_complete_matrix_nulls_timer_flags(self):
        observations, cells = [], []
        for ci,case in enumerate(a.revision.CASES):
            for phase in a.report.PHASES:
                differences=[]
                for group in range(40):
                    order='ABBA' if group%2==0 else 'BAAB'
                    rs=records(tuple(100+(ci*160+group*4+i)*100 for i in range(4)),order=order)
                    for i,r in enumerate(rs):
                        r.update(id=f'{case}-{group}-{i}',case=case,sample=group,pair=f'{case}-{group}',wallSeconds=.01,peakObservedRssBytes=100)
                    null=ci==0 and phase=='metadata'
                    rows=[raw(None,0) if null else raw(100+i+group) for i in range(4)]
                    value=a.report.group_value(rows,order)
                    differences.append(value['overheadPerParagraphNs'])
                    observations.append(dict(case=case,operation=phase,group=group,order=order,processes=rs,raw=rows,**value))
                cells.append(dict(case=case,operation=phase,applicable=not null,timerLimited=phase=='repeat',groupDifferences=differences))
        data=a.matrix(observations,cells)
        self.assertEqual([len(data[k]) for k in ('processes','groups','cells')],[1280,320,8])
        self.assertTrue(data['cells'][2]['timerLimited'])
        self.assertIsNone(data['cells'][3]['correlations']['operationVsPosition'])
        self.assertIsNone(data['cells'][3]['descriptive']['overheadPerParagraphNs'])
        self.assertEqual(data,a.matrix(observations,cells))
        with self.assertRaises(ValueError): a.matrix(observations[:-1],cells)


class ScheduleTests(unittest.TestCase):
    def test_budget_balance_contiguity_identical_commands(self):
        schedule=a.pilot_schedule(); cmds=schedule['commands']
        self.assertEqual(schedule,a.pilot_schedule())
        self.assertEqual(len(cmds),297)
        self.assertEqual([c['mode'] for c in cmds[:9]],['correctness']*9)
        self.assertEqual([c['configuration'] for c in cmds[:9]],[c['id'] for c in schedule['configurations']])
        for config in schedule['configurations']:
            rows=[c for c in cmds[9:] if c['configuration']==config['id']]
            self.assertEqual(len(rows),32)
            self.assertEqual(len({tuple(c['argv']) for c in rows}),1)
            self.assertEqual(len({c['environment'] for c in rows}),1)
            orders=[]
            for round_index in range(8):
                group=[c for c in rows if c['round']==round_index]
                self.assertEqual([c['positionInGroup'] for c in group],list(range(4)))
                self.assertEqual([c['sequence'] for c in group],list(range(group[0]['sequence'],group[0]['sequence']+4)))
                orders.append(''.join(c['arm'] for c in group))
            for half in (orders[:4],orders[4:]):
                self.assertEqual(sorted(half),['ABBA','ABBA','BAAB','BAAB'])
        for rnd in range(8):
            self.assertEqual(len([c for c in cmds if c['mode']=='timing' and c['round']==rnd]),36)


class IntegrityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory()
        cls.base=Path(cls.temp.name)/'base'; cls.base.mkdir()
        r=a.revision
        for case in r.CASES:
            shutil.copyfile(r.HISTORICAL/(case+'.json'),cls.base/(case+'.json'))
            name=f'correctness-00-{case}-zero-baseline-sample.jsonl'
            target=cls.base/'inputs/continuation'/name; target.parent.mkdir(parents=True,exist_ok=True)
            shutil.copyfile(r.HISTORICAL/name,target)
        for name,value in {'identities.json':{},'schedule.json':r.schedule(),
                           'commands.json':[r.command(e) for e in r.schedule()],
                           'environment.json':{'affinity':r.continuation.old.strict_rows(target)[0]['cpuAffinity']}}.items():
            a.write(cls.base/name,value)
        r.recovery.checkpoint(cls.base,'prepared',[cls.base/name for name in ('identities.json','schedule.json','commands.json')])
        rows,attempts=[],[]
        for i,e in enumerate(r.schedule()):
            name=e['id']+'-sample.jsonl'; err=e['id']+'-sample.stderr'
            shutil.copyfile(r.HISTORICAL/f"{e['mode']}-00-{e['case']}-zero-baseline-sample.jsonl",cls.base/name)
            (cls.base/err).write_text('')
            rows.append(dict(**e,command=r.command(e),status='completed',returncode=0,wallSeconds=.1,
                peakObservedRssBytes=1000,parentStartMonotonicNs=1_000_000_000+i*1_000_000_000,
                parentEndMonotonicNs=1_100_000_000+i*1_000_000_000,raw=name,stderr=err,
                rawSha256=a.digest(cls.base/name),stderrSha256=a.digest(cls.base/err),
                fixtureSha256=a.digest(cls.base/(e['case']+'.json'))))
            attempts.append(dict(id=e['id'],command=r.command(e)))
        for name,values in (('index.jsonl',rows),('attempts.jsonl',attempts)):
            (cls.base/name).write_text(''.join(json.dumps(v)+'\n' for v in values))

    @classmethod
    def tearDownClass(cls): cls.temp.cleanup()

    def setUp(self):
        self.temp=tempfile.TemporaryDirectory(); self.root=Path(self.temp.name)/'data'
        shutil.copytree(self.base,self.root)
        self.sentinel=a.no_launches(); self.sentinel.__enter__()

    def tearDown(self):
        self.sentinel.__exit__(None,None,None); self.temp.cleanup()

    def test_valid_closed_records_no_launch(self):
        self.assertEqual(len(a.revision.validate(self.root,complete=True)),322)

    def test_failed_missing_duplicate_misordered(self):
        original=(self.root/'index.jsonl').read_text()
        for mutate in (lambda r:r.pop(),lambda r:r[3].update(status='failed'),
                       lambda r:r.__setitem__(4,copy.deepcopy(r[3])),
                       lambda r:r.__setitem__(slice(3,5),r[3:5][::-1])):
            rows=[json.loads(line) for line in original.splitlines()]; mutate(rows)
            (self.root/'index.jsonl').write_text(''.join(json.dumps(r)+'\n' for r in rows))
            with self.assertRaises(ValueError): a.revision.validate(self.root,complete=True)

    def test_ambiguous_journals(self):
        for name in ('index.jsonl','attempts.jsonl'):
            path=self.root/name; before=path.read_bytes(); path.write_bytes(before[:-1])
            with self.assertRaises(ValueError): a.revision.validate(self.root,complete=True)
            path.write_bytes(before)
        with (self.root/'attempts.jsonl').open('a') as f:f.write('{}\n')
        with self.assertRaises(ValueError): a.revision.validate(self.root,complete=True)

    def test_raw_identity_schedule_hash_rejection(self):
        row=a.revision.journal(self.root/'index.jsonl')[0]
        for name in (row['raw'],row['stderr'],row['case']+'.json','schedule.json','commands.json','identities.json'):
            path=self.root/name; before=path.read_bytes(); path.write_bytes(before+b' ')
            with self.assertRaises(ValueError): a.revision.validate(self.root,complete=True)
            path.write_bytes(before)
        with self.assertRaises(ValueError): a.diagnosis.verify_identities({str(self.root/row['raw']):'altered'})
        with self.assertRaises(ValueError): a.diagnosis.verify_identities({str(self.root/'missing'):'missing'})

    def test_archive_member_and_archive_hash_rejection(self):
        folder=self.root/'archive'; folder.mkdir()
        path=folder/'test.tar.gz'
        with tarfile.open(path,'w:gz') as tar:
            member=tarfile.TarInfo('raw.json'); member.size=2
            tar.addfile(member,io.BytesIO(b'{}'))
        record=dict(sha256=a.digest(path),bytes=path.stat().st_size,
                    files={'raw.json':hashlib.sha256(b'{}').hexdigest()})
        a.write(folder/'archives.json',{'archives':{path.name:record}})
        self.assertEqual(a.diagnosis.verify_archives(folder,self.root/'unpacked'),record['files'])
        record['files']['raw.json']='altered'
        a.write(folder/'archives.json',{'archives':{path.name:record}})
        with self.assertRaises(ValueError): a.diagnosis.verify_archives(folder,self.root/'bad-member')
        path.write_bytes(path.read_bytes()+b'altered')
        with self.assertRaises(ValueError): a.diagnosis.verify_archives(folder,self.root/'bad-archive')

    def test_fatal_sentinels(self):
        import subprocess
        with self.assertRaises(AssertionError): subprocess.Popen(['must-never-exist'])
        with self.assertRaises(AssertionError): a.revision.supervisor.supervise([],None,None)


if __name__ == '__main__':
    with a.no_launches(): unittest.main()
