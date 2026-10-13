#!/usr/bin/env python3
"""Build and verify an exact-commit package in disposable x86_64 Arch containers."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import tarfile

SOURCE = Path(__file__).resolve().parents[1]


def version_from_cmake(text):
    match = re.search(r'project\(QMarkdown VERSION (\d+\.\d+\.\d+)\b', text)
    if not match:
        raise ValueError('Missing numeric CMake project version')
    return match[1]


def recipe(template, version, pkgrel, qt, checksum):
    if not re.fullmatch(r'\d+\.\d+\.\d+', version) or not re.fullmatch(r'\d+\.\d+\.\d+', qt):
        raise ValueError('Expected numeric X.Y.Z versions')
    if not re.fullmatch(r'[1-9]\d*', str(pkgrel)) or not re.fullmatch(r'[0-9a-f]{64}', checksum):
        raise ValueError('Invalid package revision or source checksum')
    for key, value in dict(VERSION=version, PKGREL=str(pkgrel), QT_VERSION=qt, SHA256=checksum).items():
        template = template.replace('@' + key + '@', value)
    return template


def run(command, log):
    with log.open('w') as output:
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        for line in process.stdout:
            output.write(line)
            output.flush()
            print(line, end='', flush=True)
        status = process.wait()
    if status:
        raise RuntimeError(f'Command failed ({status}); see {log}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-archive', type=Path, required=True)
    parser.add_argument('--pkgrel', default='1')
    parser.add_argument('--work-dir', type=Path, required=True)
    args = parser.parse_args()
    work = args.work_dir.resolve()
    work.mkdir(parents=True, exist_ok=True)
    if any(work.iterdir()):
        parser.error('--work-dir must be empty')
    source_archive = args.source_archive.resolve()
    with tarfile.open(source_archive) as source:
        roots = {Path(member.name).parts[0] for member in source.getmembers()}
        if len(roots) != 1:
            parser.error('Expected one source root')
        source_root = roots.pop()
        cmake = source.extractfile(source_root + '/CMakeLists.txt').read().decode()
    version = version_from_cmake(cmake)
    if source_archive.name != f'qmarkdown-{version}.tar.gz' or source_root != f'qmarkdown-{version}':
        parser.error('Source archive identity must match CMake version')
    recipe('', version, args.pkgrel, '6.8.0', '0' * 64)
    archive = work / source_archive.name
    shutil.copy2(source_archive, archive)
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    checksum = source_archive.with_name(source_archive.name + '.sha256')
    if checksum.read_text() != digest + '  ' + source_archive.name + '\n':
        parser.error('Source checksum mismatch')
    # Installed-consumer tooling is development input, never part of the distribution.
    tooling = work / 'tooling'
    tooling.mkdir()
    for name in ('PKGBUILD.in', 'container.sh', 'verify.py'):
        shutil.copy2(Path(__file__).resolve().parent / 'arch' / name, tooling / name)
    shutil.copy2(Path(__file__).resolve(), tooling / 'build-arch-package.py')
    shutil.copytree(SOURCE / 'tests/installed-consumer', tooling / 'installed-consumer')
    cache = work / 'pacman-cache'
    cache.mkdir()
    run(['docker', 'pull', '--platform', 'linux/amd64', 'archlinux:base'], work / 'image-pull.log')
    identity = json.loads(subprocess.check_output(['docker', 'image', 'inspect', 'archlinux:base']))[0]
    (work / 'environment.json').write_text(json.dumps({'version': version,
        'pkgrel': args.pkgrel, 'sourceSha256': digest, 'imageId': identity['Id'],
        'repoDigests': identity['RepoDigests'], 'architecture': identity['Architecture'],
        'toolingSha256': {path.relative_to(tooling).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
                         for path in sorted(tooling.rglob('*')) if path.is_file()}}, indent=2) + '\n')
    for mode in ('build', 'verify'):
        print(f'Arch {mode}: evidence in {work / (mode + ".log")}', flush=True)
        run(['docker', 'run', '--rm', '--platform', 'linux/amd64',
             '--mount', f'type=bind,src={work},dst=/evidence',
             '--mount', f'type=bind,src={tooling},dst=/tools,readonly',
             '--mount', f'type=bind,src={cache},dst=/var/cache/pacman/pkg',
             '--env', f'PKG_VERSION={version}', '--env', f'PKG_RELEASE={args.pkgrel}',
             '--env', f'SOURCE_SHA256={digest}', identity['Id'],
             'bash', '/tools/container.sh', mode], work / (mode + '.log'))
    package = work / f'qmarkdown-{version}-{args.pkgrel}-x86_64.pkg.tar.zst'
    package.with_name(package.name + '.sha256').write_text(
        hashlib.sha256(package.read_bytes()).hexdigest() + '  ' + package.name + '\n')
    print(f'PASS: {package}')


if __name__ == '__main__':
    main()
