#!/usr/bin/env python3
"""Isolated ten-window continuation; execution requires recorded scope approval."""
import argparse
import importlib.util
import json
from pathlib import Path

from handoff import save

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('frozen_scale_study_v2', HERE.parent/'scale-study-v2/study.py')
study = importlib.util.module_from_spec(spec)
spec.loader.exec_module(study)
OriginalJournal = study.Journal


class Journal(OriginalJournal):
    def start(self, cell):
        frozen = study.load(self.directory/'freeze.json')
        allowed = study.load(self.directory/'schedule.json')
        if cell not in allowed or cell['kind'] != 'live':
            raise RuntimeError('attempt outside live-only schedule')
        if len(self.started) >= frozen['maxLaunches']:
            raise RuntimeError('ten-window continuation exhausted')
        super().start(cell)


# Patch the loaded module in this process only; frozen files remain untouched.
study.save = save
study.Journal = Journal


def prepare(prior, directory):
    frozen = study.verify_identities(prior)
    try:
        journal = OriginalJournal(prior)
    except RuntimeError as error:
        if str(error) != 'study stopped terminally':
            raise
    else:
        journal.lock.close()
        raise RuntimeError('expected the preserved terminal study')
    events = [json.loads(line) for line in (prior/'attempts.jsonl').read_text().splitlines()]
    starts = [r for r in events if r['event']=='start']
    finishes = [r for r in events if r['event']=='finish']
    if len(starts)!=674 or len(finishes)!=674:
        raise RuntimeError('unexpected prior launch accounting')
    restoration = study.load(prior/'restoration.json')
    if not restoration['verified'] or restoration['outputs'] != study.load(prior/'original-output.json'):
        raise RuntimeError('original output restoration not verified')
    cells = [c for c in study.load(prior/'schedule.json') if c['kind']=='live']
    if len(cells)!=10 or len({c['id'] for c in cells})!=10:
        raise RuntimeError('invalid continuation schedule')
    for cell in cells:
        cell['command'][4] = str(directory/'attempts'/cell['id'])
    identities = dict(frozen['identities'])
    for path in HERE.iterdir():
        if path.is_file():identities[str(path)]=study.sha(path)
    for path in [prior/'attempts.jsonl', prior/'restoration.json']:
        identities[str(path)] = study.sha(path)
    # Reserve 120 seconds for each interrupted historical harness attempt/pass.
    fresh_seconds = sum(r['seconds'] for r in finishes if int(r['id']) < 680)
    directory.mkdir(parents=True, exist_ok=False)
    save(directory/'schedule.json', cells)
    save(directory/'original-output.json', restoration['outputs'])
    for scale in study.SCALES:
        name=f'fresh-{scale:g}-complete.json'
        save(directory/name, study.load(prior/name))
    save(directory/'freeze.json', dict(build=frozen['build'],identities=identities,
        scheduleHash=study.sha(directory/'schedule.json'),original=frozen['original'],
        maxLaunches=10,priorLaunches=675,totalProposedLaunches=685,totalAuthorizedCeiling=691,
        priorAutomatedSeconds=fresh_seconds+240,automatedSeconds=14400,
        transitions=study.TRANSITIONS,priorStudy=str(prior),approvalAtPreparation='pending'))


def require_approval(directory):
    frozen=study.verify_identities(directory)
    approval=study.load(directory/'approval.json')
    if (approval.get('approved') is not True or
        approval.get('scope')!='ten replacement live windows; no fresh rerun' or
        approval.get('scheduleHash')!=frozen['scheduleHash']):
        raise RuntimeError('live-only replacement scope approval missing or mismatched')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    sub=parser.add_subparsers(dest='action',required=True)
    p=sub.add_parser('prepare');p.add_argument('prior',type=Path);p.add_argument('directory',type=Path)
    p=sub.add_parser('confirm');p.add_argument('directory',type=Path);p.add_argument('scale',type=float);p.add_argument('label')
    p=sub.add_parser('live');p.add_argument('directory',type=Path)
    args=parser.parse_args();directory=args.directory.resolve()
    if args.action=='prepare':prepare(args.prior.resolve(),directory)
    else:
        require_approval(directory)
        if args.action=='confirm':study.confirm(directory,args.scale,args.label)
        else:
            if study.load(directory/'confirmations/live-0.json')['scale']!=2:
                raise RuntimeError('initial 200% handoff missing')
            study.live(directory)


if __name__=='__main__':main()
