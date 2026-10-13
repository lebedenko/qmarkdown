#!/usr/bin/env python3
"""Pacman lifecycle checks; invoked only by the disposable container runner."""
import os
from pathlib import Path
import re
import subprocess
import tarfile


def output(*args):
    return subprocess.check_output(list(map(str, args)), text=True)


def run(*args):
    print('+', ' '.join(map(str, args)), flush=True)
    subprocess.run(list(map(str, args)), check=True)


def allowed_payload(path):
    module = Path('/usr/lib/qt6/qml/QMarkdown')
    return (
        (path.parent == Path('/usr/lib') and path.name.startswith('libQMarkdown.so'))
        or (path.parent == module and
            (path.name.startswith('libQMarkdownPlugin.so') or path.suffix == '.qml'))
        or (path.parent in (module, module / 'Private')
            and (path.name == 'qmldir' or path.suffix == '.qmltypes'))
        or (path.parent == Path('/usr/lib/cmake/QMarkdown') and path.suffix == '.cmake')
        or str(path).startswith('/usr/share/licenses/qmarkdown/'))


def main():
    if not Path('/.dockerenv').exists() or os.geteuid() != 0:
        raise RuntimeError('Verification requires a disposable root Docker container')
    evidence = Path('/evidence')
    version, revision = os.environ['PKG_VERSION'], os.environ['PKG_RELEASE']
    package = evidence / f'qmarkdown-{version}-{revision}-x86_64.pkg.tar.zst'
    metadata = output('bsdtar', '-xOf', package, '.PKGINFO')
    (evidence / 'package-info.txt').write_text(metadata)
    if f'pkgver = {version}-{revision}\n' not in metadata or 'arch = x86_64\n' not in metadata:
        raise RuntimeError('Incorrect package identity')
    qt = re.search(r'depend = qt6-base>=(\d+\.\d+\.\d+)\n', metadata)
    if not qt or f'depend = qt6-declarative>={qt[1]}\n' not in metadata:
        raise RuntimeError('Missing Qt build-version lower bounds')
    for dependency in ('glibc', 'gcc-libs'):
        if f'depend = {dependency}\n' not in metadata:
            raise RuntimeError(f'Missing dependency: {dependency}')
    for target in (evidence / 'PKGBUILD', package):
        result = output('namcap', target)
        print(result, flush=True)
        (evidence / (target.name + '.namcap.log')).write_text(result)
        if re.search(r'\bE:', result):
            raise RuntimeError('namcap errors')
    # Keep the complete listing, and use pacman's installed list for ownership/removal.
    (evidence / 'payload.txt').write_text(output('bsdtar', '-tvf', package))
    run('pacman', '-U', '--noconfirm', package)
    info = output('pacman', '-Qi', 'qmarkdown')
    (evidence / 'installed-info.txt').write_text(info)
    paths = [Path(line) for line in output('pacman', '-Qlq', 'qmarkdown').splitlines()]
    files = [path for path in paths if not path.is_dir() or path.is_symlink()]
    required = [Path('/usr/lib/libQMarkdown.so'), Path('/usr/lib/qt6/qml/QMarkdown/libQMarkdownPlugin.so'),
                Path('/usr/lib/cmake/QMarkdown/QMarkdownConfig.cmake'),
                Path('/usr/share/licenses/qmarkdown/LICENSE'), Path('/usr/share/licenses/qmarkdown/cmark/COPYING')]
    if not all(path in files for path in required):
        raise RuntimeError('Incomplete package payload')
    for path in files:
        name = str(path)
        if not allowed_payload(path):
            raise RuntimeError(f'Unexpected payload: {path}')
        if path.suffix in ('.a', '.o', '.h', '.hpp') or 'libQt' in path.name or path.name in ('installed.json', 'install.sh', 'uninstall.sh'):
            raise RuntimeError(f'Forbidden payload: {path}')
        if path.lstat().st_uid != 0:
            raise RuntimeError(f'Installed file is not root-owned: {path}')
        if output('pacman', '-Qoq', path).strip() != 'qmarkdown':
            raise RuntimeError(f'Wrong file owner: {path}')
        if path.is_symlink():
            if not path.resolve().exists() or not str(path.resolve()).startswith('/usr/lib/'):
                raise RuntimeError(f'Invalid link: {path}')
            continue
        data = path.read_bytes()
        if any(value in data for value in (b'/work/', b'/evidence/', b'/qmarkdown-source/')):
            raise RuntimeError(f'Leaked build path: {path}')
        if data.startswith(b'\x7fELF'):
            dynamic = output('readelf', '-d', path)
            if 'RPATH' in dynamic or 'RUNPATH' in dynamic:
                raise RuntimeError(f'Installed RPATH: {path}')
            if 'Advanced Micro Devices X86-64' not in output('readelf', '-h', path):
                raise RuntimeError('Wrong ELF architecture')
            if 'not found' in output('ldd', path):
                raise RuntimeError(f'Unresolved dependencies: {path}')
    integrity = output('pacman', '-Qkk', 'qmarkdown')
    print(integrity)
    if '0 altered files' not in integrity:
        raise RuntimeError('Package integrity failure')
    with tarfile.open(evidence / 'qmarkdown-source.tar.gz') as source:
        source.extractall('/work', filter='data')
    for link, styling, actions, tasks, strike, autolinks in (
            ('ON', 'OFF', 'OFF', 'OFF', 'OFF', 'OFF'), ('OFF', 'OFF', 'OFF', 'OFF', 'OFF', 'OFF'),
            ('ON', 'ON', 'OFF', 'OFF', 'OFF', 'OFF'), ('OFF', 'ON', 'OFF', 'OFF', 'OFF', 'OFF'),
            ('ON', 'OFF', 'ON', 'OFF', 'OFF', 'OFF'), ('OFF', 'OFF', 'ON', 'OFF', 'OFF', 'OFF'),
            ('ON', 'OFF', 'OFF', 'ON', 'OFF', 'OFF'), ('OFF', 'OFF', 'OFF', 'ON', 'OFF', 'OFF'),
            ('ON', 'OFF', 'OFF', 'OFF', 'ON', 'OFF'), ('OFF', 'OFF', 'OFF', 'OFF', 'ON', 'OFF'),
            ('ON', 'OFF', 'OFF', 'OFF', 'OFF', 'ON'), ('OFF', 'OFF', 'OFF', 'OFF', 'OFF', 'ON')):
        build = Path('/work') / f'consumer-{link}-{styling}-{actions}-{tasks}-{strike}-{autolinks}'
        run('cmake', '-S', '/work/qmarkdown/tests/installed-consumer', '-B', build,
            '-G', 'Ninja', f'-DQMARKDOWN_CONSUMER_LINK_MODULE={link}',
            f'-DQMARKDOWN_STYLING_CONSUMER={styling}', f'-DQMARKDOWN_ACTIONS_CONSUMER={actions}',
            f'-DQMARKDOWN_TASKS_CONSUMER={tasks}', f'-DQMARKDOWN_STRIKE_CONSUMER={strike}',
            f'-DQMARKDOWN_AUTOLINK_CONSUMER={autolinks}',
            f'-DQMARKDOWN_REQUEST_VERSION={"1.5.0" if autolinks == "ON" else "1.4.0" if strike == "ON" else "1.3.0" if tasks == "ON" else "1.2" if actions == "ON" else "1.0"}',
            f'-DQMARKDOWN_REQUEST_EXACT={autolinks}')
        run('cmake', '--build', build, '--parallel', '2')
        executable = build / 'qmarkdown-consumer'
        if link == 'OFF' and 'libQMarkdown' in output('readelf', '-d', executable):
            raise RuntimeError('Plugin-only consumer links backing library')
        run(executable)
    # Repackage the actual payload with only a higher pkgrel; never export it.
    upgrade = Path('/work/upgrade')
    upgrade.mkdir()
    run('bsdtar', '-xf', package, '-C', upgrade)
    # Generate the temporary revision with makepkg as an unprivileged user,
    # including fresh package metadata and mtree. Reuse only the installed payload.
    original = (evidence / 'PKGBUILD').read_text().split('\nbuild() {', 1)[0]
    recipe = original.replace(f'pkgrel={revision}\n', f'pkgrel={int(revision) + 1}\n')
    recipe += '\nsource=()\nsha256sums=()\npackage() { cp -a /work/upgrade/usr "$pkgdir/"; }\n'
    (upgrade / 'PKGBUILD').write_text(recipe)
    run('chown', '-R', 'builder:builder', upgrade)
    run('runuser', '-u', 'builder', '--', 'bash', '-ec', 'cd /work/upgrade; makepkg --noconfirm --log')
    upgraded = upgrade / f'qmarkdown-{version}-{int(revision) + 1}-x86_64.pkg.tar.zst'
    run('pacman', '-U', '--noconfirm', upgraded)
    if output('pacman', '-Q', 'qmarkdown').strip() != f'qmarkdown {version}-{int(revision) + 1}':
        raise RuntimeError('Upgrade registration failed')
    for path in files:
        if output('pacman', '-Qoq', path).strip() != 'qmarkdown':
            raise RuntimeError('Upgrade ownership failed')
    run('pacman', '-R', '--noconfirm', 'qmarkdown')
    if subprocess.run(['pacman', '-Q', 'qmarkdown']).returncode == 0:
        raise RuntimeError('Removal registration failed')
    if any(path.exists() or path.is_symlink() for path in files):
        raise RuntimeError('Removal left package files')
    print('PASS: Arch metadata, payload, namcap, consumers and pacman lifecycle')


if __name__ == '__main__':
    main()
