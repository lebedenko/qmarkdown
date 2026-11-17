#!/usr/bin/env python3
"""Approved final matrix using frozen supervisor and atomic publication."""
import argparse
import importlib.util
import math
from pathlib import Path
import diagnose
study=diagnose.previous
OriginalJournal=study.Journal
original_validate=study.validate
study.save=diagnose.handoff.save
HERE=Path(__file__).resolve().parent

def mapping_assertions(rows):
    latest=None;failures=[]
    for row in rows:
        if row.get('stage')=='scale-contract':
            if latest and row['generation']<=latest['generation']:failures.append('non-increasing mapping generation')
            latest=row
        if row.get('stage') in ('capture-mapping','checkpoint'):
            if not latest or row.get('generation')!=latest['generation']:
                failures.append('missing/stale capture generation');continue
            if row.get('buffer')!=latest['buffer']:failures.append('capture buffer differs from mapping')
            if row.get('stage')=='capture-mapping' and (row.get('window')!=latest['window'] or row.get('zoom')!=latest['zoom']):
                failures.append('capture geometry differs from mapping')
    if not latest:failures.append('missing mapping contract')
    return failures

def validate(process,manifest,live=False):
    result=original_validate(process,manifest,live)
    failures=mapping_assertions(process.records)
    result['mappingErrors']=failures
    result['errors']+=failures
    if failures:result['status']='failed'
    result['lifecycle']={'exit':process.process.returncode,'passed':process.process.returncode==0}
    result['correctnessErrors']=[e for e in result['errors'] if e not in ('nonzero process exit','missing exit record')]
    return result
study.validate=validate

class Journal(OriginalJournal):
    def start(self,cell):
        frozen=study.load(self.directory/'freeze.json')
        if cell not in study.load(self.directory/'schedule.json'):raise RuntimeError('unscheduled launch')
        if len(self.started)>=690:raise RuntimeError('final reserve exhausted')
        super().start(cell)
study.Journal=Journal

def freeze(build,directory,diagnostics):
    study.load(build/'preflight.json');study.load(build/'layout-check.json')
    events=[study.load(p/'result.json') for p in diagnostics.glob('d*') if p.is_dir()]
    starts=[r for r in (diagnostics/'attempts.jsonl').read_text().splitlines() if '"event": "start"' in r]
    if len(starts)!=len(events) or len(starts)>60:raise RuntimeError('unfinished/excess diagnostics')
    identities=study.load(build/'identities.json')
    for p in HERE.iterdir():
        if p.is_file():identities[str(p)]=study.sha(p)
    for p in [Path(study.__file__),HERE.parent/'scale-study-live-recovery/handoff.py',diagnostics/'attempts.jsonl']:
        identities[str(p)]=study.sha(p)
    for p,h in identities.items():
        if study.sha(Path(p))!=h:raise RuntimeError('identity drift: '+p)
    outputs=study.read_outputs();original=study.load(diagnostics/'original-output.json')[0]
    study.output_scale(outputs,original['scale'],original)
    directory.mkdir(parents=True,exist_ok=False)
    study.save(directory/'original-output.json',study.load(diagnostics/'original-output.json'))
    study.save(directory/'schedule.json',study.schedule(build,directory))
    study.save(directory/'freeze.json',dict(build=str(build),identities=identities,
        scheduleHash=study.sha(directory/'schedule.json'),original=original,maxLaunches=690,
        diagnosticLaunches=len(starts),totalAuthorizedLaunches=750,
        priorAutomatedSeconds=sum(r['seconds'] for r in events),automatedSeconds=14400,
        transitions=study.TRANSITIONS,approval='Owner supplied implementation plan 2026-11-09'))

def main():
    p=argparse.ArgumentParser();sub=p.add_subparsers(dest='action',required=True)
    f=sub.add_parser('freeze');f.add_argument('build',type=Path);f.add_argument('directory',type=Path);f.add_argument('diagnostics',type=Path)
    c=sub.add_parser('confirm');c.add_argument('directory',type=Path);c.add_argument('scale',type=float);c.add_argument('label')
    f=sub.add_parser('fresh');f.add_argument('directory',type=Path);f.add_argument('scale',type=float)
    f=sub.add_parser('live');f.add_argument('directory',type=Path)
    a=p.parse_args();directory=a.directory.resolve()
    if a.action=='freeze':freeze(a.build.resolve(),directory,a.diagnostics.resolve())
    elif a.action=='confirm':study.confirm(directory,a.scale,a.label)
    elif a.action=='fresh':study.fresh(directory,a.scale)
    else:study.live(directory)
if __name__=='__main__':main()
