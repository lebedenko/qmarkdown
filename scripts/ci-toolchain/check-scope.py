#!/usr/bin/env python3
"""Select additive verification coverage from every path in a Git diff."""
import argparse
import json
import subprocess


def classify(names, force=False):
    profile = 'light'
    syntax = math = shipping = False
    for name in names:
        if name.startswith('specs/'):
            continue  # Standalone experiments own their exact-Qt local execution.
        if (name == 'CHANGELOG.md' or name.startswith('docs/release-notes-')
                or name.startswith(('benchmarks/', 'tests/installed-consumer/', 'cmake/',
                                    '.github/', 'scripts/', 'third_party/'))
                or name in {'Taskfile.yml', '.gitignore'} or name.rsplit('/', 1)[-1] == 'CMakeLists.txt'
                or (name.startswith('tests/') and 'benchmark' in name.lower())
                or name.endswith('.cmake') or (name.startswith('tests/') and name.endswith('.py'))):
            profile = 'full'
        elif name.startswith(('src/', 'qml/', 'tests/', 'examples/')):
            if profile != 'full':
                profile = 'core'
        elif not (name.startswith(('specs/', 'docs/')) or name in {'README.md', 'AGENTS.md'}):
            profile = 'full'
            shipping = True  # Unrecognized inputs require conservative local coverage.
        if (not name.endswith('.py') and name.startswith(('src/', 'qml/', 'tests/', 'examples/', 'cmake/', 'third_party/')) or name == 'CMakeLists.txt' or name.endswith('.cmake')):
            shipping = True
        lower = name.lower()
        syntax |= any(word in lower for word in ('syntax', 'highlight', 'provider', 'codeblock', 'codesurface'))
        math |= 'math' in lower or 'microtex' in lower
    return {'profile': 'full' if force else profile, 'syntax': syntax, 'math': math,
            'shipping': shipping}


def changed_paths(base, head='HEAD'):
    # --no-renames expands moves into deletion/addition, including both paths.
    data = subprocess.check_output(['git', 'diff', '--no-renames', '--name-only', '-z', base, head])
    return [p.decode('utf-8', 'surrogateescape') for p in data.split(b'\0') if p]


def full_verification(names):
    return classify(names)['profile'] == 'full'


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--base', required=True)
    parser.add_argument('--head', default='HEAD')
    parser.add_argument('--force-full', action='store_true')
    args = parser.parse_args()
    print(json.dumps(classify(changed_paths(args.base, args.head), args.force_full)))
