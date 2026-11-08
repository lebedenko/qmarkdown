#!/usr/bin/env python3
"""Deterministic analysis archive with exact member/hash read-back."""
import gzip
import io
import json
from pathlib import Path
import sys
import tarfile

from analyze import HERE, digest, require, verify_archives, write
import tempfile


def package(output, destination):
    require(not destination.exists(), 'destination must be new')
    destination.mkdir(parents=True)
    files = {}
    for name in ('cells.json', 'integrity.json', 'preservation.json', 'report.md', 'failed-pairs.md', 'context.md'):
        files[name] = output / name
    for folder in ('binary', 'sources'):
        for path in sorted((output / folder).rglob('*')):
            if path.is_file():
                files[str(path.relative_to(output))] = path
    for name in ('summary.json', 'report.md'):
        files['recomputed/' + name] = output / 'recomputed' / name
    for path in sorted(HERE.iterdir()):
        if path.is_file():
            files['analysis/' + path.name] = path
    for path in sorted((output / 'checks').glob('*')):
        if path.is_file():
            files['checks/' + path.name] = path
    archive = destination / '073-candidate-diagnosis.tar.gz'
    with archive.open('wb') as raw, gzip.GzipFile(fileobj=raw, mode='wb', filename='', mtime=0) as compressed:
        with tarfile.open(fileobj=compressed, mode='w') as tar:
            for name, path in sorted(files.items()):
                data = path.read_bytes()
                member = tarfile.TarInfo(name)
                member.size = len(data)
                member.mode = 0o644
                tar.addfile(member, io.BytesIO(data))
    write(destination / 'archives.json', dict(archives={archive.name: dict(
        sha256=digest(archive), bytes=archive.stat().st_size,
        files={name: digest(path) for name, path in sorted(files.items())})}))
    with tempfile.TemporaryDirectory() as folder:
        actual = verify_archives(destination, Path(folder))
    (destination / 'report.md').write_bytes((output / 'report.md').read_bytes())
    print(f'Archive read-back: {len(actual)} members verified; sha256 {digest(archive)}')


if __name__ == '__main__':
    package(Path(sys.argv[1]), Path(sys.argv[2]))
