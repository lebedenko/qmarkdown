#!/usr/bin/env python3
"""Check an extracted distribution with focused CMake build/install checks."""
import argparse
from pathlib import Path
import subprocess
import tarfile


def verify(archive, qt, work, backend="OFF", kde_prefix=None):
    if work.exists() and any(work.iterdir()):
        raise ValueError('Distribution check directory must be empty')
    work.mkdir(parents=True, exist_ok=True)
    with tarfile.open(archive) as source:
        members = source.getmembers()
        roots = {Path(member.name).parts[0] for member in members}
        if len(roots) != 1:
            raise ValueError('Expected one distribution root')
        for member in members:
            path = Path(member.name)
            if (path.is_absolute() or '..' in path.parts or not member.isfile()
                    or len(path.parts) < 2 or path.parts[1] not in
                    {'CMakeLists.txt', 'LICENSE', 'INSTALL.md', 'src', 'cmake', 'third_party'}):
                raise ValueError(f'Unexpected distribution entry: {member.name}')
        # All members are validated regular files; supports Python 3.9 too.
        source.extractall(work / 'source')
    extracted = work / 'source' / roots.pop()
    build, prefix = work / 'build', work / 'install'
    commands = [
        ['cmake', '-S', extracted, '-B', build, '-DCMAKE_BUILD_TYPE=Release',
         f'-DCMAKE_PREFIX_PATH={qt};{kde_prefix or qt}',
         f'-DQMARKDOWN_WITH_KSYNTAXHIGHLIGHTING={backend}', f'-DQt6_DIR={qt}/lib/cmake/Qt6',
         f'-DCMAKE_INSTALL_PREFIX={prefix}', '-DCMAKE_INSTALL_LIBDIR=lib'],
        ['cmake', '--build', build, '--parallel', '2'],
        ['cmake', '--install', build],
    ]
    for command in commands:
        subprocess.run(list(map(str, command)), check=True)
    for name in ('QMarkdownConfig.cmake', 'QMarkdownConfigVersion.cmake', 'QMarkdownTargets.cmake'):
        if not (prefix / 'lib/cmake/QMarkdown' / name).is_file():
            raise ValueError(f'Missing installed CMake metadata: {name}')
    for name in ('LICENSE', 'cmark/COPYING', 'cmark/PROVENANCE.md'):
        if not (prefix / 'share/licenses/QMarkdown' / name).is_file():
            raise ValueError(f'Missing installed license: {name}')
    for name in ('BlockSequence.qml', 'TextBlock.qml', 'SelectableText.qml',
                 'ImageBlock.qml', 'ListBlock.qml', 'HeadingSeparator.qml',
                 'CodeSurface.qml', 'TaskIndicatorHost.qml', 'CodeActionHost.qml', 'TableBlock.qml'):
        if not list(prefix.rglob(name)):
            raise ValueError(f'Missing installed private QML component: {name}')
    cache = (build / 'CMakeCache.txt').read_text()
    if 'BUILD_TESTING:BOOL=OFF' not in cache or 'QMARKDOWN_BUILD_EXAMPLES:BOOL=OFF' not in cache:
        raise ValueError('Source archive must default to library-only')
    if list(prefix.rglob('*.h')):
        raise ValueError('Private headers were installed')
    print('PASS: source distribution build and install')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path, required=True)
    parser.add_argument('--qt-root', type=Path, required=True)
    parser.add_argument('--work-dir', type=Path, required=True)
    parser.add_argument('--syntax-highlighting', choices=('OFF', 'ON'), default='OFF')
    parser.add_argument('--kde-prefix', type=Path)
    args = parser.parse_args()
    verify(args.archive.resolve(), args.qt_root.resolve(), args.work_dir.resolve(), args.syntax_highlighting, args.kde_prefix)


if __name__ == '__main__':
    main()
