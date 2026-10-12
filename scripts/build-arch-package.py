#!/usr/bin/env python3
"""Build and verify an exact-commit package in disposable x86_64 Arch containers."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess

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
        result = subprocess.run(command, stdout=output, stderr=subprocess.STDOUT)
    if result.returncode:
        raise RuntimeError(f'Command failed ({result.returncode}); see {log}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-commit', default='HEAD')
    parser.add_argument('--pkgrel', default='1')
    parser.add_argument('--work-dir', type=Path, required=True)
    args = parser.parse_args()
    work = args.work_dir.resolve()
    work.mkdir(parents=True, exist_ok=True)
    if any(work.iterdir()):
        parser.error('--work-dir must be empty')
    commit = subprocess.check_output(['git', 'rev-parse', '--verify', args.source_commit + '^{commit}'], cwd=SOURCE, text=True).strip()
    cmake = subprocess.check_output(['git', 'show', f'{commit}:CMakeLists.txt'], cwd=SOURCE, text=True)
    version = version_from_cmake(cmake)
    # Validate all substituted values before provisioning.
    recipe('', version, args.pkgrel, '6.8.0', '0' * 64)
    archive = work / 'qmarkdown-source.tar.gz'
    subprocess.run(['git', 'archive', '--format=tar.gz', '--prefix=qmarkdown/', '-o', str(archive), commit], cwd=SOURCE, check=True)
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    tooling = work / 'tooling'
    tooling.mkdir()
    for name in ('PKGBUILD.in', 'container.sh', 'verify.py'):
        shutil.copy2(Path(__file__).resolve().parent / 'arch' / name, tooling / name)
    shutil.copy2(Path(__file__).resolve(), tooling / 'build-arch-package.py')
    cache = work / 'pacman-cache'
    cache.mkdir()
    run(['docker', 'pull', '--platform', 'linux/amd64', 'archlinux:base'], work / 'image-pull.log')
    identity = json.loads(subprocess.check_output(['docker', 'image', 'inspect', 'archlinux:base']))[0]
    (work / 'environment.json').write_text(json.dumps({'sourceCommit': commit, 'version': version,
        'pkgrel': args.pkgrel, 'sourceSha256': digest, 'imageId': identity['Id'],
        'repoDigests': identity['RepoDigests'], 'architecture': identity['Architecture'],
        'toolingSha256': {path.name: hashlib.sha256(path.read_bytes()).hexdigest()
                         for path in sorted(tooling.iterdir())}}, indent=2) + '\n')
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
