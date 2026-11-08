#!/usr/bin/env python3
"""Archive the new pilot evidence and verify every member by read-back."""
import gzip
import io
from pathlib import Path
import sys
import tarfile
import tempfile
import run as p


def package(output,destination):
    with p.a.no_launches():
        __import__('report').report(output)
        p.require(not destination.exists(),'fresh evidence directory required')
        files={str(x.relative_to(output)):x for x in output.rglob('*') if x.is_file() and '/inputs/' not in str(x.relative_to(output))}
        files['binary/prepared-reflow']=p.BINARY
        files.update({'tool/'+x.name:x for x in p.HERE.iterdir() if x.is_file()})
        for name in ('requirements.md','design.md','tasks.md','verification.md','candidate-scan-recommendation.md','batching-refinement-protocol.md'):
            files['specification/'+name]=p.FEATURE/name
        destination.mkdir(parents=True)
        archive=destination/'073-candidate-batching-refinement.tar.gz'
        with archive.open('wb') as raw,gzip.GzipFile(fileobj=raw,mode='wb',filename='',mtime=0) as zipped:
            with tarfile.open(fileobj=zipped,mode='w') as tar:
                for name,path in sorted(files.items()):
                    data=path.read_bytes();member=tarfile.TarInfo(name);member.size=len(data);member.mode=0o644
                    tar.addfile(member,io.BytesIO(data))
        p.write(destination/'archives.json',dict(archives={archive.name:dict(sha256=p.digest(archive),bytes=archive.stat().st_size,files={name:p.digest(path) for name,path in sorted(files.items())})}))
        with tempfile.TemporaryDirectory() as tmp:members=p.a.diagnosis.verify_archives(destination,Path(tmp))
        for name in ('analysis.json','selection.json','report.md'):(destination/name).write_bytes((output/name).read_bytes())
        p.a.diagnosis.verify_identities(__import__('json').loads((output/'identities.json').read_text()))
        print('Archive read-back:',len(members),'member hashes verified; historical identities preserved.')

if __name__=='__main__':package(Path(sys.argv[1]).resolve(),Path(sys.argv[2]).resolve())
