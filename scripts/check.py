#!/usr/bin/env python3
"""Qt-independent repository checks, optionally restricting evidence to a diff."""
import argparse
import ast
import gzip
import json
from pathlib import Path
import runpy
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--base')
    parser.add_argument('--all-files', action='store_true')
    args = parser.parse_args()
    subprocess.run([sys.executable, 'scripts/check-docs.py'], cwd=ROOT, check=True)
    if args.all_files:
        paths = [p.relative_to(ROOT).as_posix() for p in ROOT.rglob('*') if p.is_file()]
    elif args.base:
        paths = runpy.run_path(str(ROOT / 'scripts/ci-toolchain/check-scope.py'))['changed_paths'](args.base)
    else:
        paths = subprocess.check_output(['git', 'ls-files', '--cached', '--others', '--exclude-standard', '-z'], cwd=ROOT).decode().split('\0')
    for name in paths:
        path = ROOT / name
        if not path.is_file():
            continue
        if name.endswith('.sh') or name.startswith('.githooks/'):
            subprocess.run(['bash', '-n', str(path)], check=True)
        if name.endswith('.py'):
            ast.parse(path.read_text(), filename=name)
        if name.endswith(('.yaml', '.yml')):
            try:
                import yaml
            except ImportError:
                parser.error('YAML validation requires PyYAML: install python3-yaml or python3 -m pip install PyYAML==6.0.3')
            yaml.safe_load(path.read_text())
        if name.endswith(('.json', '.jsonl', '.jsonl.gz')):
            opener = gzip.open if name.endswith('.gz') else open
            with opener(path, 'rt', encoding='utf-8') as stream:
                if '.jsonl' in name:
                    for line in stream:
                        if line.strip():
                            json.loads(line)
                else:
                    json.load(stream)
    subprocess.run([sys.executable, '-m', 'unittest', 'discover', '-s', 'tests', '-p', 'test_*.py'], cwd=ROOT, check=True)
    return 0


if __name__ == '__main__':
    sys.exit(main())
