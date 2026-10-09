#!/usr/bin/env python3
"""Verify archive integrity, installer lifecycle and installed Qt consumers."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile
import tempfile

SOURCE = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SOURCE / 'scripts/release'))
from installer import describe


def run(*command, env=None, success=True):
    print('+', ' '.join(map(str, command)), flush=True)
    result = subprocess.run(list(map(str, command)), env=env)
    if (result.returncode == 0) != success:
        raise RuntimeError(f'Unexpected exit {result.returncode}: {command}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path, required=True)
    parser.add_argument('--qt-root', type=Path, required=True)
    parser.add_argument('--work-dir', type=Path, required=True)
    parser.add_argument('--real-install', action='store_true', help='Only in a disposable container')
    args = parser.parse_args()
    work = args.work_dir.resolve()
    work.mkdir(parents=True, exist_ok=True)
    if any(work.iterdir()):
        parser.error('--work-dir must be empty')
    archive = args.archive.resolve()
    expected, name = archive.with_suffix(archive.suffix + '.sha256').read_text().split()
    if name != archive.name or hashlib.sha256(archive.read_bytes()).hexdigest() != expected:
        raise RuntimeError('Archive checksum mismatch')
    with tarfile.open(archive) as package_tar:
        members = package_tar.getmembers()
        enclosing = archive.name.removesuffix('.tar.gz')
        for member in members:
            path = Path(member.name)
            if (path.is_absolute() or '..' in path.parts or path.parts[0] != enclosing
                    or not (member.isfile() or member.isdir() or member.issym())):
                raise RuntimeError(f'Unsafe archive member: {member.name}')
            if member.issym():
                target = os.path.normpath(str(path.parent / member.linkname))
                if Path(member.linkname).is_absolute() or not target.startswith(enclosing + '/'):
                    raise RuntimeError('Escaping archive link')
        # Archives are built locally; validated member types and paths exclude escapes.
        package_tar.extractall(work / 'extracted', filter='data')
    package = work / 'extracted' / enclosing
    for script in ('install.sh', 'uninstall.sh'):
        if (package / script).stat().st_mode & 0o777 != 0o755:
            raise RuntimeError(f'Incorrect executable permissions: {script}')
    manifest = json.loads((package / 'manifest.json').read_text())
    if any(Path(path).suffix in ('.h', '.hpp', '.a', '.o') or 'libQt' in path for path in manifest):
        raise RuntimeError('Forbidden archive payload')
    metadata = json.loads((package / 'metadata.json').read_text())
    if metadata['architecture'] != 'x86_64' or metadata['os'] != 'Ubuntu 24.04':
        raise RuntimeError('Unexpected metadata')
    root = work / 'root'
    record = root / 'usr/share/qmarkdown/installed.json'
    def install(success=True):
        run(package / 'install.sh', '--destdir', root, success=success)
    def uninstall():
        run(package / 'uninstall.sh', '--destdir', root)
    def snapshot():
        return {p.relative_to(root).as_posix(): describe(p)
                for p in root.rglob('*') if p.is_file() or p.is_symlink()}
    install()
    if json.loads(record.read_text()) != manifest:
        raise RuntimeError('Incorrect ownership record')
    before = snapshot()
    install()
    assert snapshot() == before
    # A synthetic old package owns a now-obsolete file and old content.
    obsolete = root / 'usr/share/qmarkdown/obsolete.txt'
    obsolete.write_text('old package')
    owned = json.loads(record.read_text())
    owned[obsolete.relative_to(root).as_posix()] = describe(obsolete)
    regular = next(key for key, value in manifest.items() if value['kind'] == 'file')
    target = root / regular
    target.write_bytes(b'previous release')
    owned[regular] = describe(target)
    record.write_text(json.dumps(owned))
    install()
    assert not obsolete.exists() and describe(target) == manifest[regular]
    target.write_bytes(b'local modification')
    before = snapshot()
    install(success=False)
    assert snapshot() == before
    uninstall()
    assert target.read_bytes() == b'local modification'
    assert json.loads(record.read_text()) == {regular: manifest[regular]}
    # Remove only test-created content, then verify a clean install/uninstall.
    target.unlink()
    uninstall()
    assert not record.exists()
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text('unrelated')
    before = snapshot()
    install(success=False)
    assert snapshot() == before
    target.unlink()
    # Directory conflicts and symlink ancestors also fail before mutation.
    target.mkdir()
    before = snapshot()
    install(success=False)
    assert snapshot() == before
    target.rmdir()
    evil = root / 'usr/lib'
    if evil.exists():
        shutil.rmtree(evil)  # Private test root only.
    evil.symlink_to(work)
    install(success=False)
    evil.unlink()
    install()
    consume(root, args.qt_root.resolve(), work / 'consumers')
    uninstall()
    assert not record.exists()
    if args.real_install:
        if not Path('/.dockerenv').exists() or os.geteuid() != 0:
            parser.error('--real-install requires root inside a disposable Docker container')
        run(package / 'install.sh')
        run(package / 'install.sh')
        for path in manifest:
            assert Path('/' + path).lstat().st_uid == 0
        consume(Path('/'), args.qt_root.resolve(), work / 'real-consumers')
        run(package / 'uninstall.sh')
        assert not Path('/usr/share/qmarkdown/installed.json').exists()
    print('PASS: archive, installer lifecycle and installed consumers', flush=True)


def consume(root, qt, work):
    prefix = root / 'usr'
    env = os.environ.copy()
    env.update(QT_QPA_PLATFORM='offscreen', QML_IMPORT_PATH=str(prefix / 'lib/x86_64-linux-gnu/qt6/qml'),
               LD_LIBRARY_PATH=str(qt / 'lib'))
    env.pop('QML2_IMPORT_PATH', None)
    env.pop('QT_PLUGIN_PATH', None)
    for link in ('ON', 'OFF'):
        build = work / link
        run('cmake', '-S', SOURCE / 'tests/installed-consumer', '-B', build,
            f'-DCMAKE_PREFIX_PATH={prefix};{qt}', f'-DQt6_DIR={qt}/lib/cmake/Qt6',
            f'-DQMarkdown_DIR={prefix}/lib/x86_64-linux-gnu/cmake/QMarkdown',
            f'-DQMARKDOWN_CONSUMER_LINK_MODULE={link}')
        run('cmake', '--build', build, '--parallel', '2')
        executable = build / 'qmarkdown-consumer'
        if link == 'OFF':
            dynamic = subprocess.check_output(['readelf', '-d', executable], text=True)
            if 'libQMarkdown' in dynamic:
                raise RuntimeError('Plugin-only consumer links the backing library')
        run(executable, env=env)


if __name__ == '__main__':
    main()
