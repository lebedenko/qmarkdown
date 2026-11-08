#!/usr/bin/env python3
"""Package closed calibration evidence and verify every archived member."""
import gzip
import io
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import run


def package(output, destination):
    run.validate(output, complete=True)
    run.require(not destination.exists(), 'fresh archive destination required')
    files = {str(p.relative_to(output)): p for p in output.rglob('*') if p.is_file()}
    for path in run.HERE.iterdir():
        if path.is_file(): files['tool/' + path.name] = path
    destination.mkdir(parents=True)
    archive = destination / '073-candidate-calibration.tar.gz'
    with archive.open('wb') as raw, gzip.GzipFile(fileobj=raw, mode='wb', filename='', mtime=0) as zipped:
        with tarfile.open(fileobj=zipped, mode='w') as tar:
            for name, path in sorted(files.items()):
                data = path.read_bytes()
                member = tarfile.TarInfo(name)
                member.size = len(data)
                member.mode = 0o644
                tar.addfile(member, io.BytesIO(data))
    run.write(destination / 'archives.json', dict(archives={archive.name: dict(
        sha256=run.digest(archive), bytes=archive.stat().st_size,
        files={name: run.digest(path) for name, path in sorted(files.items())})}))
    with tempfile.TemporaryDirectory() as temporary:
        members = run.diagnosis.verify_archives(destination, Path(temporary))
    for name in ('summary.json', 'report.md'):
        (destination / name).write_bytes((output / name).read_bytes())
    # Recheck every original archive/member/source identity after packaging.
    run.diagnosis.verify_identities(json.loads((output / 'identities.json').read_text()))
    print(f'Archive read-back: {len(members)} members verified; sha256 {run.digest(archive)}; historical identities unchanged.')


if __name__ == '__main__':
    package(Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve())
