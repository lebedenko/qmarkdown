#!/usr/bin/env python3
"""Read-only completeness and evidence report; missing coverage never becomes a pass."""
import argparse
from collections import Counter
import json
from pathlib import Path
import tarfile
import hashlib
from study import load, sha, save


def report(directory, output):
    cells=load(directory/'schedule.json');frozen=load(directory/'freeze.json')
    events=[json.loads(line) for line in (directory/'attempts.jsonl').read_text().splitlines()] if (directory/'attempts.jsonl').exists() else []
    started={r['id'] for r in events if r['event']=='start'}
    skipped={r['id'] for r in events if r['event']=='skip'}
    rows=[]
    for cell in cells:
        folder=directory/'attempts'/cell['id'];result_path=folder/'result.json'
        result=load(result_path) if result_path.exists() else None
        status=result['status'] if result else 'failed' if cell['id'] in started else 'skipped' if cell['id'] in skipped else 'untested'
        if result:
            for name,h in result.get('captures',{}).items():
                if sha(folder/name)!=h:raise ValueError('capture hash mismatch: '+str(folder/name))
        rows.append(dict(id=cell['id'],kind=cell['kind'],scale=cell['scale'],configuration=cell['config']['id'],
            fixture=cell['fixture'],cycle=cell['cycle'],status=status,
            errors=result.get('errors',[]) if result else ['interrupted attempt'] if cell['id'] in started else []))
    result=dict(cells=rows,counts=dict(Counter(r['status'] for r in rows)),launches=len(started),
        configuration='see per-attempt identities and output records',
        correctness='see per-cell assertions; missing cells untested',
        comparability='untested pending physical mapping review',
        transitions='untested' if not (directory/'live-complete.json').exists() else 'see live checkpoints',
        ownerVisual='untested',restoration=load(directory/'restoration.json') if (directory/'restoration.json').exists() else 'unverified')
    output.mkdir(parents=True,exist_ok=False)
    save(output/'summary.json',result)
    (output/'README.md').write_text('# Four-scale study results\n\n'+
        f"Graphical launches: {len(started)}/690. Cell counts: {result['counts']}.\n\n"+
        'Configuration, correctness, comparability, transitions and visual acceptance are separate.\n'+
        'Missing cells remain untested; failures are retained without replacement.\n'+
        'No performance or production qualification is inferred.\n')
    # Capture every existing study file, with a member hash manifest and read-back.
    paths=[p for p in directory.rglob('*') if p.is_file() and p.name!='lock']
    identities={str(p.relative_to(directory)):sha(p) for p in paths}
    save(output/'members.json',identities)
    archive=output/'study.tar.gz'
    with tarfile.open(archive,'w:gz') as tar:
        for p in paths:tar.add(p,arcname=str(p.relative_to(directory)),recursive=False)
    with tarfile.open(archive) as tar:
        observed={m.name:hashlib.sha256(tar.extractfile(m).read()).hexdigest() for m in tar if m.isfile()}
    if observed!=identities:raise ValueError('archive read-back mismatch')
    save(output/'archive.json',dict(sha256=sha(archive),members=len(identities),readBack=True))


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('directory',type=Path);p.add_argument('output',type=Path)
    a=p.parse_args();report(a.directory,a.output)
