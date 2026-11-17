#!/usr/bin/env python3
"""Post-run deterministic analysis, terminal rejection and archive tamper checks."""
import io
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import run as p
import report


def verify(output):
    with p.a.no_launches():
        before={n:(output/n).read_bytes() for n in ('analysis.json','decision.json','report.md')}
        report.report(output)
        for name,data in before.items():p.require((output/name).read_bytes()==data,'non-deterministic reanalysis: '+name)
        # The historical verifier is also used by packaging; prove both archive
        # and member identities reject corruption without launching an executable.
        with tempfile.TemporaryDirectory() as tmp:
            folder=Path(tmp)/'archive';folder.mkdir();archive=folder/'tiny.tar.gz'
            data=b'preserved observation\n'
            with tarfile.open(archive,'w:gz') as tar:
                member=tarfile.TarInfo('observation');member.size=len(data);tar.addfile(member,io.BytesIO(data))
            identity=dict(archives={archive.name:dict(sha256=p.digest(archive),bytes=archive.stat().st_size,files={'observation':__import__('hashlib').sha256(data).hexdigest()})})
            p.write(folder/'archives.json',identity)
            p.a.diagnosis.verify_archives(folder,Path(tmp)/'valid')
            identity['archives'][archive.name]['files']['observation']='0'*64
            p.write(folder/'archives.json',identity)
            try:p.a.diagnosis.verify_archives(folder,Path(tmp)/'bad-member')
            except (AssertionError,ValueError):pass
            else:raise AssertionError('corrupt member accepted')
            identity['archives'][archive.name]['sha256']='0'*64
            p.write(folder/'archives.json',identity)
            try:p.a.diagnosis.verify_archives(folder,Path(tmp)/'bad-archive')
            except (AssertionError,ValueError):pass
            else:raise AssertionError('corrupt archive accepted')
        if not json.loads((output/'decision.json').read_text())['complete']:
            with tempfile.TemporaryDirectory() as tmp:
                try:p.resume(output,Path(tmp)/'must-not-resume')
                except Exception:pass
                else:raise AssertionError('terminal run accepted for recovery')
    print('Byte-identical reanalysis; corrupt archives/members rejected; zero executable launches.')


if __name__=='__main__':verify(Path(sys.argv[1]).resolve())
