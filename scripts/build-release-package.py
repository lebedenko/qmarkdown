#!/usr/bin/env python3
"""Build and test the Release shared library, then package its CMake install tree."""
import argparse
import gzip
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tarfile

sys.path.insert(0, str(Path(__file__).resolve().parent / 'release'))
from installer import describe, entries_valid

SOURCE = Path(__file__).resolve().parents[1]


def run(*args, **kwargs):
    print('+', ' '.join(map(str, args)), flush=True)
    subprocess.run(list(map(str, args)), check=True, **kwargs)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--qt-root', type=Path, required=True)
    parser.add_argument('--work-dir', type=Path, required=True)
    parser.add_argument('--source-commit', required=True)
    args = parser.parse_args()
    work = args.work_dir.resolve()
    work.mkdir(parents=True, exist_ok=True)
    if any(work.iterdir()):
        parser.error('--work-dir must be empty')
    qt = args.qt_root.resolve()
    qt_version = subprocess.check_output([qt / 'bin/qmake', '-query', 'QT_VERSION'], text=True).strip()
    version = re.search(r'project\(QMarkdown VERSION (\d+\.\d+\.\d+)',
                        (SOURCE / 'CMakeLists.txt').read_text())[1]
    if not re.fullmatch('[0-9a-f]{40}', args.source_commit):
        parser.error('--source-commit must be a full Git commit SHA')
    os_release = Path('/etc/os-release').read_text()
    if 'ID=ubuntu\n' not in os_release or 'VERSION_ID="24.04"' not in os_release:
        parser.error('Package build requires Ubuntu 24.04')
    if os.uname().machine != 'x86_64':
        parser.error('Package build requires x86_64')
    name = f'qmarkdown-{version}-ubuntu24.04-x86_64-qt{qt_version.rsplit(".", 1)[0]}'
    package = work / name
    payload = package / 'payload'
    build = work / 'build'
    run('cmake', '-S', SOURCE, '-B', build, '-DCMAKE_BUILD_TYPE=Release',
        '-DBUILD_SHARED_LIBS=ON', '-DBUILD_TESTING=ON', '-DQMARKDOWN_BUILD_EXAMPLES=OFF',
        '-DQMARKDOWN_BUILD_BENCHMARKS=OFF', '-DCMAKE_INSTALL_PREFIX=/usr',
        '-DCMAKE_INSTALL_LIBDIR=lib/x86_64-linux-gnu',
        '-DQMARKDOWN_QML_INSTALL_DIR=lib/x86_64-linux-gnu/qt6/qml',
        f'-DQt6_DIR={qt}/lib/cmake/Qt6', f'-DCMAKE_PREFIX_PATH={qt}')
    run('cmake', '--build', build, '--parallel', '2')
    run('ctest', '--test-dir', build, '--output-on-failure')
    run('cmake', '--build', build, '--target', 'all_qmllint')
    run('python3', SOURCE / 'scripts/verify-cmark-symbols.py', build)
    run('cmake', '--install', build, env={**os.environ, 'DESTDIR': str(payload)})
    manifest = {}
    for path in sorted(payload.rglob('*')):
        if path.is_dir() and not path.is_symlink():
            continue
        relative = path.relative_to(payload).as_posix()
        if path.suffix in ('.h', '.hpp', '.a', '.o') or 'libQt' in path.name:
            raise RuntimeError(f'Forbidden payload: {relative}')
        if path.suffix in ('.cmake', '.qmltypes') or path.name == 'qmldir':
            content = path.read_text()
            for forbidden in (str(SOURCE), str(build), str(qt), str(payload)):
                if forbidden in content:
                    raise RuntimeError(f'Leaked path in {relative}: {forbidden}')
        if path.is_file() and not path.is_symlink() and path.read_bytes()[:4] == b'\x7fELF':
            dynamic = subprocess.check_output(['readelf', '-d', path], text=True)
            for line in dynamic.splitlines():
                if 'RPATH' in line or 'RUNPATH' in line:
                    search = line.split('[', 1)[1].split(']', 1)[0]
                    if any(not item.startswith('$ORIGIN') for item in search.split(':')):
                        raise RuntimeError(f'Unsafe runtime search path: {line}')
            header = subprocess.check_output(['readelf', '-h', path], text=True)
            if 'Advanced Micro Devices X86-64' not in header:
                raise RuntimeError(f'Unexpected ELF architecture: {relative}')
            run('ldd', path, env={**os.environ, 'LD_LIBRARY_PATH': f'{payload}/usr/lib/x86_64-linux-gnu:{qt}/lib'})
            dependencies = subprocess.check_output(['ldd', path], text=True,
                env={**os.environ, 'LD_LIBRARY_PATH': f'{payload}/usr/lib/x86_64-linux-gnu:{qt}/lib'})
            if 'not found' in dependencies:
                raise RuntimeError(f'Unresolved dependencies: {relative}')
        manifest[relative] = describe(path)
    entries_valid(manifest, require_targets=True)
    required = ['usr/lib/x86_64-linux-gnu/libQMarkdown.so',
                'usr/lib/x86_64-linux-gnu/qt6/qml/QMarkdown/libQMarkdownPlugin.so',
                'usr/share/licenses/QMarkdown/LICENSE', 'usr/share/licenses/QMarkdown/cmark/COPYING',
                'usr/lib/x86_64-linux-gnu/cmake/QMarkdown/QMarkdownConfig.cmake']
    if any(item not in manifest for item in required):
        raise RuntimeError('Incomplete installed payload')
    (package / 'manifest.json').write_text(json.dumps(manifest, indent=2, sort_keys=True) + '\n')
    (package / 'metadata.json').write_text(json.dumps({
        'version': version, 'sourceCommit': args.source_commit, 'architecture': 'x86_64',
        'qtVersion': qt_version, 'buildType': 'Release', 'os': 'Ubuntu 24.04',
        'compiler': subprocess.check_output(['c++', '--version'], text=True).splitlines()[0],
        'osRelease': os_release,
    }, indent=2) + '\n')
    shutil.copy2(SOURCE / 'scripts/release/installer.py', package / 'installer.py')
    shutil.copy2(SOURCE / 'docs/release-install.md', package / 'INSTALL.md')
    for operation in ('install', 'uninstall'):
        script = package / f'{operation}.sh'
        script.write_text('#!/bin/sh\nset -eu\nexec python3 "$(dirname "$(readlink -f "$0")")/installer.py" '
                          + operation + ' "$@"\n')
        script.chmod(0o755)
    archive = work / f'{name}.tar.gz'
    def normalized(info):
        info.uid = info.gid = 0
        info.uname = info.gname = 'root'
        info.mtime = 0
        return info
    with archive.open('wb') as stream:
        with gzip.GzipFile(filename='', mode='wb', fileobj=stream, mtime=0) as compressed:
            with tarfile.open(fileobj=compressed, mode='w', format=tarfile.PAX_FORMAT) as output:
                output.add(package, arcname=name, filter=normalized)
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    archive.with_suffix(archive.suffix + '.sha256').write_text(f'{digest}  {archive.name}\n')
    print(f'PASS: {archive}', flush=True)


if __name__ == '__main__':
    main()
