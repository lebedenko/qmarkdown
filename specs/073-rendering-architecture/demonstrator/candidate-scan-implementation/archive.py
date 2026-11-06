#!/usr/bin/env python3
"""Archive immutable diagnostic evidence, including preparation/correction history."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import tarfile
import run as runner

HERE=Path(__file__).resolve().parent

def archive(workspace,evidence,destination):
    assert runner.recovery.phase_complete(evidence,'finished')
    identities=json.loads((evidence/'identities.json').read_text())
    for name,expected in identities.items():assert runner.digest(Path(name))==expected,name
    for folder in (HERE,HERE.parent/'candidate-scan-experiment',HERE.parent/'performance-experiment',HERE.parent/'boundary-experiment',HERE.parent/'ownership-experiment',HERE.parent/'runtime-comparison'):
        target=evidence/'reproduction'/folder.name;target.mkdir(parents=True)
        for f in folder.iterdir():
            if f.is_file():shutil.copy2(f,target/f.name)
    target=evidence/'build-config';target.mkdir()
    for variant in ('clean','prototype'):
        for f in (workspace/variant/'out/comparison').rglob('*.ninja'):
            out=target/variant/f.relative_to(workspace/variant/'out/comparison');out.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(f,out)
        for f in (workspace/variant/'out/comparison/args.gn',workspace/(variant+'-build')/'build.ninja',workspace/(variant+'-build')/'CMakeCache.txt'):
            out=target/variant/f.name;out.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(f,out)
    for folder in ('ownership-experiment','runtime-comparison'):
        shutil.copy2(HERE.parents[1]/'evidence'/folder/'archives.json',evidence/(folder+'-archives.json'))
    files=sorted(p for p in evidence.rglob('*') if p.is_file() and not p.is_symlink())
    groups=[];current=[];size=0
    for f in files:
        if current and size+f.stat().st_size>64*1024**2:groups.append(current);current=[];size=0
        current.append(f);size+=f.stat().st_size
    if current:groups.append(current)
    destination.mkdir(parents=True,exist_ok=True);records={}
    for i,group in enumerate(groups,1):
        path=destination/f'073-candidate-implementation-part-{i:03}.tar.gz';assert not path.exists()
        expected={str(f.relative_to(evidence)):runner.digest(f) for f in group}
        with tarfile.open(path,'w:gz') as tar:
            for f in group:tar.add(f,arcname=str(f.relative_to(evidence)),recursive=False)
        with tarfile.open(path,'r:gz') as tar:
            actual={}
            for member in tar:
                assert member.isfile()
                actual[member.name]=hashlib.sha256(tar.extractfile(member).read()).hexdigest()
            assert actual==expected
        records[path.name]=dict(sha256=runner.digest(path),bytes=path.stat().st_size,files=expected)
    (destination/'archives.json').write_text(json.dumps(dict(sourceDirectory=evidence.name,archives=records),indent=2)+'\n')
    for name,r in records.items():assert runner.digest(destination/name)==r['sha256']
    print(f'Verified {len(files)} files in {len(groups)} archives')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('workspace',type=Path);p.add_argument('evidence',type=Path);p.add_argument('destination',type=Path);a=p.parse_args();archive(a.workspace.resolve(),a.evidence.resolve(),a.destination.resolve())
