#!/usr/bin/env python3
"""Validate Git snapshots without stashing or changing the user's checkout."""
import argparse
import fcntl
import hashlib
import os
from pathlib import Path
import runpy
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
scope = runpy.run_path(str(ROOT / 'scripts/ci-toolchain/check-scope.py'))


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)


def export_tree(tree, destination):
    """Replace only managed snapshot inputs; leave adjacent build outputs intact."""
    entries = git('ls-tree', '-rz', tree).split(b'\0')
    wanted = {}
    for entry in entries:
        if not entry:
            continue
        metadata, name = entry.split(b'\t', 1)
        mode, kind, oid = metadata.split()
        if kind != b'blob':
            raise RuntimeError('Submodules are unsupported by local hooks; run explicit verification')
        wanted[os.fsdecode(name)] = (mode, oid)
    # Drop obsolete files before adding new paths, including file/directory moves.
    if destination.exists():
        for path in sorted(destination.rglob('*'), reverse=True):
            relative = path.relative_to(destination).as_posix()
            if path.is_symlink() or path.is_file():
                if relative not in wanted:
                    path.unlink()
            elif path.is_dir():
                try:
                    path.rmdir()
                except OSError:
                    pass
    destination.mkdir(parents=True, exist_ok=True)
    for name, (mode, oid) in wanted.items():
        path = destination / name
        data = git('cat-file', 'blob', oid.decode())
        if mode == b'120000':
            target = os.fsdecode(data)
            if Path(target).is_absolute() or '..' in Path(target).parts:
                raise RuntimeError(f'Unsafe snapshot symlink: {name}')
            if path.is_symlink() and os.readlink(path) == target:
                continue
            if path.exists() or path.is_symlink():
                path.unlink()
            path.parent.mkdir(parents=True, exist_ok=True)
            path.symlink_to(target)
        else:
            if path.is_symlink():
                path.unlink()
            path.parent.mkdir(parents=True, exist_ok=True)
            if not path.exists() or path.read_bytes() != data:
                path.write_bytes(data)
            path.chmod(0o755 if mode == b'100755' else 0o644)


def run(*command, cwd=None, env=None):
    subprocess.run(command, cwd=cwd or ROOT, env=env, check=True)


def verify(tree, cache, build=False):
    source = cache / 'source'
    export_tree(tree, source)
    run(sys.executable, str(source / 'scripts/check.py'), '--all-files', cwd=source)
    if not build:
        return
    versions = runpy.run_path(str(source / 'scripts/ci-toolchain/qt_versions.py'))['load_versions']()
    qmake = os.environ.get('QMARKDOWN_QMAKE', 'qmake6')
    if not shutil.which(qmake):
        raise RuntimeError('Development Qt missing: install the configured SDK and set QMARKDOWN_QMAKE=/path/to/qmake; hooks never download Qt')
    actual = subprocess.check_output([qmake, '-query', 'QT_VERSION'], text=True).strip()
    if actual != versions['development']:
        raise RuntimeError(f"Expected Qt {versions['development']}, found {actual}; set QMARKDOWN_QMAKE=/path/to/matching/qmake")
    prefix = subprocess.check_output([qmake, '-query', 'QT_INSTALL_PREFIX'], text=True).strip()
    for tool in ('cmake', 'ninja', 'ctest'):
        if not shutil.which(tool):
            raise RuntimeError(f'Missing {tool}; install it before running task check-push')
    build_dir = cache / ('build-' + hashlib.sha256(prefix.encode()).hexdigest()[:12])
    run('cmake', '-G', 'Ninja', '-S', str(source), '-B', str(build_dir),
        f'-DCMAKE_PREFIX_PATH={prefix}', f'-DQt6_DIR={prefix}/lib/cmake/Qt6',
        '-DBUILD_SHARED_LIBS=ON', '-DBUILD_TESTING=ON', '-DQMARKDOWN_BUILD_EXAMPLES=ON')
    run('cmake', '--build', str(build_dir), '--parallel', '2')
    environment = os.environ.copy()
    environment.update(QT_QPA_PLATFORM='offscreen', QT_QUICK_CONTROLS_STYLE='Fusion',
                       QML_IMPORT_PATH=str(build_dir / 'qml'))
    run('ctest', '--parallel', '1', '--test-dir', str(build_dir), '--output-on-failure', env=environment)


def push_commits(lines):
    commits = set()
    for line in lines:
        local_ref, commit, remote_ref, remote_commit = line.split()
        if set(commit) == {'0'}:
            continue
        commits.add(commit)
    return sorted(commits)


def install():
    configured = subprocess.run(['git', 'config', '--get', 'core.hooksPath'], cwd=ROOT,
                                capture_output=True, text=True).stdout.strip()
    if configured and configured != '.githooks':
        raise RuntimeError(f'Existing core.hooksPath={configured}; integrate .githooks scripts manually or explicitly unset that configuration')
    hooks = Path(os.fsdecode(git('rev-parse', '--git-path', 'hooks')).strip())
    if not hooks.is_absolute():
        hooks = ROOT / hooks
    if not configured and hooks.exists() and any(p.is_file() and not p.name.endswith('.sample') for p in hooks.iterdir()):
        raise RuntimeError('Existing Git hooks found; integrate .githooks manually before setting core.hooksPath')
    run('git', 'config', '--local', 'core.hooksPath', '.githooks')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mode', choices=('install', 'pre-commit', 'pre-push', 'check-push'))
    args = parser.parse_args()
    if args.mode == 'install':
        install()
        return
    cache = ROOT / 'build-ci/hooks'
    cache.mkdir(parents=True, exist_ok=True)
    with (cache / 'lock').open('w') as lock:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            raise RuntimeError('Another hook is checking this repository; retry when it finishes')
        if args.mode == 'pre-commit':
            run('git', 'diff', '--cached', '--check')
            tree = git('write-tree').decode().strip()
            verify(tree, cache)
        else:
            commits = push_commits(sys.stdin) if args.mode == 'pre-push' else [git('rev-parse', 'HEAD').decode().strip()]
            for commit in commits:
                try:
                    base = git('merge-base', 'origin/main', commit).decode().strip()
                    requirements = scope['classify'](scope['changed_paths'](base, commit))
                    build = requirements['shipping']
                except subprocess.CalledProcessError:
                    build = True
                verify(commit, cache, build)


if __name__ == '__main__':
    try:
        main()
    except (RuntimeError, subprocess.CalledProcessError) as error:
        print(f'Hook failed: {error}', file=sys.stderr)
        sys.exit(1)
