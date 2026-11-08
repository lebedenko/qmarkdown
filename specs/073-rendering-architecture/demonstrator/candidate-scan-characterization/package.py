#!/usr/bin/env python3
"""Archive offline sources, derived observations, identities, protocol and check logs."""
import gzip
import io
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import analyze as a
from verify import ARTIFACTS


def package(output, destination):
    with a.no_launches():
        a.require(not destination.exists() or {p.name for p in destination.iterdir()} == {'README.md'},
                  'fresh evidence destination (optional README only) required')
        identities=json.loads((output/'input-identities.json').read_text())
        a.diagnosis.verify_identities(identities)
        files={name:output/name for name in ARTIFACTS}
        files.update({'analysis/'+p.name:p for p in a.HERE.iterdir() if p.is_file()})
        files.update({'checks/'+p.name:p for p in (output/'checks').iterdir()
                      if p.is_file() and p.name != 'archive-verification.log'})
        for name in ('requirements.md','design.md','tasks.md','verification.md','candidate-scan-recommendation.md'):
            files['specification/'+name]=a.FEATURE/name
        # Retain the exact reproduced estimates without duplicating the historical archives.
        for name in ('summary.json','observations.json','report.md'):
            files['reproduced-revision/'+name]=output/'recomputed'/name
        destination.mkdir(parents=True, exist_ok=True)
        archive=destination/'073-candidate-characterization.tar.gz'
        with archive.open('wb') as raw, gzip.GzipFile(fileobj=raw,mode='wb',filename='',mtime=0) as zipped:
            with tarfile.open(fileobj=zipped,mode='w') as tar:
                for name,path in sorted(files.items()):
                    data=path.read_bytes(); member=tarfile.TarInfo(name)
                    member.size=len(data); member.mode=0o644
                    tar.addfile(member,io.BytesIO(data))
        a.write(destination/'archives.json',dict(archives={archive.name:dict(sha256=a.digest(archive),
            bytes=archive.stat().st_size,files={name:a.digest(path) for name,path in sorted(files.items())})}))
        with tempfile.TemporaryDirectory() as tmp:
            members=a.diagnosis.verify_archives(destination,Path(tmp))
        for name in ('matrix.json','report.md','pilot-schedule.json','processes.csv','groups.csv','cells.csv'):
            (destination/name).write_bytes((output/name).read_bytes())
        a.diagnosis.verify_identities(identities)
        print(f'Archive read-back: {len(members)} member hashes verified; SHA-256 {a.digest(archive)}; historical identities unchanged.')


if __name__ == '__main__': package(Path(sys.argv[1]).resolve(),Path(sys.argv[2]).resolve())
