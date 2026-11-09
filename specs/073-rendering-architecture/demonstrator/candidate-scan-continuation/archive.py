#!/usr/bin/env python3
"""Package final runtime evidence; verify every archived member and archive hash."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import tarfile

HERE=Path(__file__).resolve().parent


def digest(path):
    h=hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda:stream.read(1024*1024),b''):h.update(block)
    return h.hexdigest()


def archive(workspace, evidence, destination):
    rows=[json.loads(line) for line in (evidence/'index.jsonl').read_text().splitlines()]
    assert (evidence/'finished-checkpoint.json').is_file() or any(r['status']!='completed' for r in rows), 'only closed complete or terminal-failed studies may be archived'
    # Record reporting/verification sources added after the immutable timing build.
    supplemental={}
    for folder in (HERE,HERE.parent/'performance-experiment',HERE.parent/'boundary-experiment',HERE.parent/'ownership-experiment',HERE.parent/'runtime-comparison',HERE.parent/'candidate-scan-implementation',HERE.parent/'candidate-scan-experiment'):
        target=evidence/'reproduction'/folder.name
        target.mkdir(parents=True,exist_ok=True)
        for f in folder.iterdir():
            if f.is_file():
                shutil.copy2(f,target/f.name)
                supplemental[str(f)]=digest(f)
    identities=json.loads((evidence/'identities.json').read_text())
    for name, expected in identities.items():assert digest(Path(name))==expected,name
    for label in ('baseline','retained'):
        target=evidence/'build-config'/label
        target.mkdir(parents=True,exist_ok=True)
        stage=workspace/label
        for f in (stage/'out/comparison').rglob('*.ninja'):
            out=target/'skia'/f.relative_to(stage/'out/comparison')
            out.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(f,out)
        for f in [stage/'out/comparison/args.gn',workspace/(label+'-build')/'build.ninja',workspace/(label+'-build')/'CMakeCache.txt']:
            shutil.copy2(f,target/f.name)
    root=HERE.parents[3]
    for f in [root/'build-ci/073-performance-v2/build.json',root/'build-ci/073-performance-recovery/recovery-manifest.json']:
        shutil.copy2(f,evidence/('input-'+f.name))
    for folder in ('phase-experiment','ownership-experiment','candidate-scan-experiment','candidate-scan-implementation','runtime-comparison'):
        shutil.copy2(HERE.parents[1]/'evidence'/folder/'archives.json',evidence/(folder+'-archives.json'))
    (evidence/'supplemental-identities.json').write_text(json.dumps(supplemental,indent=2)+'\n')
    destination.mkdir(parents=True,exist_ok=True)
    # Independent tar parts keep each reviewable; each member appears in one part.
    files=sorted(p for p in evidence.rglob('*') if p.is_file() and not p.is_symlink())
    groups=[];current=[];size=0
    for f in files:
        if size+f.stat().st_size>64*1024**2 and current:
            groups.append(current);current=[];size=0
        current.append(f);size+=f.stat().st_size
    if current:groups.append(current)
    records={}
    for index,group in enumerate(groups,1):
        path=destination/f'073-candidate-runtime-part-{index:03}.tar.gz'
        assert not path.exists(),path
        expected={str(f.relative_to(evidence)):digest(f) for f in group}
        with tarfile.open(path,'w:gz') as tar:
            for f in group:tar.add(f,arcname=str(f.relative_to(evidence)),recursive=False)
        with tarfile.open(path,'r:gz') as tar:
            actual={}
            for member in tar:
                assert member.isfile()
                h=hashlib.sha256()
                with tar.extractfile(member) as stream:
                    for block in iter(lambda:stream.read(1024*1024),b''):h.update(block)
                actual[member.name]=h.hexdigest()
            assert actual==expected
        records[path.name]=dict(sha256=digest(path),bytes=path.stat().st_size,files=expected)
    (destination/'archives.json').write_text(json.dumps(dict(archives=records,sourceDirectory=evidence.name),indent=2)+'\n')
    for name,record in records.items():assert digest(destination/name)==record['sha256']
    print(f'Verified {len(files)} files in {len(groups)} archives')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('workspace',type=Path);p.add_argument('evidence',type=Path);p.add_argument('destination',type=Path)
    a=p.parse_args();archive(a.workspace.resolve(),a.evidence.resolve(),a.destination.resolve())
