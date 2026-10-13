#!/usr/bin/env python3
"""Classify PR inputs; release preparation always requires complete verification."""
import argparse
import subprocess


def full_verification(names):
    for name in names:
        if name == 'CHANGELOG.md' or name.startswith('docs/release-notes-'):
            return True
        if not (name.endswith('.md') and (name in {'README.md', 'AGENTS.md'}
                or name.startswith(('docs/', 'specs/')))):
            return True
    return False


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--base', required=True)
    args = parser.parse_args()
    names = subprocess.check_output(['git', 'diff', '--name-only', args.base, 'HEAD'], text=True).splitlines()
    print('true' if full_verification(names) else 'false')
