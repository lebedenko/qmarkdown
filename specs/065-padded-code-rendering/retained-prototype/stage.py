#!/usr/bin/env python3
"""Export committed source, then apply only the diagnostic snapshot patch."""
import argparse
from pathlib import Path
import subprocess
import tarfile
import io

parser = argparse.ArgumentParser()
parser.add_argument('destination', type=Path)
args = parser.parse_args()
root = Path(__file__).resolve().parents[3]
args.destination.mkdir(parents=True, exist_ok=False)
archive = subprocess.check_output(['git', 'archive', 'HEAD'], cwd=root)
with tarfile.open(fileobj=io.BytesIO(archive)) as source:
    source.extractall(args.destination, filter='data')
patch = Path(__file__).with_name('prepared-runs.patch').read_bytes()
subprocess.run(['git', 'apply', '--unsafe-paths', '-'], input=patch, cwd=args.destination, check=True)
print(subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip())
