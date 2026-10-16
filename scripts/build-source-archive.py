#!/usr/bin/env python3
"""Produce a deterministic library-only archive from an explicit file allowlist."""
import argparse
import gzip
import hashlib
import io
from pathlib import Path
import re
import subprocess
import tarfile

SOURCE = Path(__file__).resolve().parents[1]
ALLOWLIST = SOURCE / 'scripts/release/source-files.txt'


def project_version(text):
    match = re.search(r'project\(QMarkdown VERSION (\d+\.\d+\.\d+)\b', text)
    if not match:
        raise ValueError('Missing numeric CMake project version')
    return match[1]


def build_archive(source, work, commit=None):
    def read(name):
        if commit:
            return subprocess.check_output(['git', 'show', f'{commit}:{name}'], cwd=source)
        return (source / name).read_bytes()

    names = read('scripts/release/source-files.txt').decode().splitlines()
    if not names or len(names) != len(set(names)):
        raise ValueError('Empty or duplicate source allowlist')
    required = {'CMakeLists.txt', 'LICENSE', 'third_party/cmark/COPYING',
                'third_party/cmark/PROVENANCE.md', 'docs/source-install.md'}
    if not required.issubset(names):
        raise ValueError('Missing build instructions or licenses in source allowlist')
    for name in names:
        path = Path(name)
        if (path.is_absolute() or '..' in path.parts or
                not (name in required or name.startswith(('src/QMarkdown/', 'cmake/', 'third_party/cmark/', 'third_party/microtex/', 'third_party/tinyxml2/')))):
            raise ValueError(f'Forbidden source allowlist entry: {name}')
        if not commit and (source / name).is_symlink():
            raise ValueError(f'Symlink in source allowlist: {name}')
    version = project_version(read('CMakeLists.txt').decode())
    work.mkdir(parents=True, exist_ok=True)
    archive = work / f'qmarkdown-{version}.tar.gz'
    if archive.exists() or archive.with_name(archive.name + '.sha256').exists():
        raise ValueError('Source archive output already exists')
    with archive.open('wb') as stream:
        with gzip.GzipFile(filename='', mode='wb', fileobj=stream, mtime=0) as compressed:
            with tarfile.open(fileobj=compressed, mode='w', format=tarfile.USTAR_FORMAT) as output:
                for name in sorted(names):
                    content = read(name)
                    target = 'INSTALL.md' if name == 'docs/source-install.md' else name
                    info = tarfile.TarInfo(f'qmarkdown-{version}/{target}')
                    info.size, info.mode, info.mtime = len(content), 0o644, 0
                    output.addfile(info, io.BytesIO(content))
    archive.with_name(archive.name + '.sha256').write_text(
        hashlib.sha256(archive.read_bytes()).hexdigest() + '  ' + archive.name + '\n')
    return archive


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-commit', default='HEAD')
    parser.add_argument('--work-dir', type=Path, required=True)
    args = parser.parse_args()
    commit = subprocess.check_output(['git', 'rev-parse', '--verify', args.source_commit + '^{commit}'],
                                     cwd=SOURCE, text=True).strip()
    print(build_archive(SOURCE, args.work_dir.resolve(), commit))


if __name__ == '__main__':
    main()
