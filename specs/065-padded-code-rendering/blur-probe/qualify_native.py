#!/usr/bin/env python3
"""Ordered, fresh-evidence native-only qualification. Stop at the failing stage."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

p = argparse.ArgumentParser()
p.add_argument('minimum_baseline', type=Path)
p.add_argument('minimum_candidate', type=Path)
p.add_argument('development_baseline', type=Path)
p.add_argument('development_candidate', type=Path)
p.add_argument('output', type=Path)
p.add_argument('--minimum-library-path')
a = p.parse_args()
a.output.mkdir(parents=True, exist_ok=False)
root = Path(__file__).resolve().parent
stages = []
identities = {str(f.resolve()): hashlib.sha256(f.read_bytes()).hexdigest() for f in
              [a.minimum_baseline, a.minimum_candidate, a.development_baseline, a.development_candidate]}
(a.output / 'binaries.json').write_text(json.dumps(identities, indent=2) + '\n')
def run(name, command):
    with (a.output / (name + '.log')).open('w') as log:
        status = subprocess.run([sys.executable, *map(str, command)], stdout=log, stderr=subprocess.STDOUT).returncode
    stages.append(dict(name=name, command=list(map(str, command)), exit=status))
    (a.output / 'stages.json').write_text(json.dumps(stages, indent=2) + '\n')
    print(name, 'PASS' if status == 0 else 'FAIL', flush=True)
    if status:
        raise SystemExit('Qualification stopped; subsequent stages not reached.')

targets = [('6.8.0', a.minimum_baseline, a.minimum_candidate),
           ('6.12.0', a.development_baseline, a.development_candidate)]
for qt, baseline, candidate in targets:
    libraries = ['--library-path', a.minimum_library_path] if qt == '6.8.0' and a.minimum_library_path else []
    for backend, scale in [('software', 1.25), ('software', 1.5), ('vulkan', 1)]:
        name = f'calibration-{qt}-{backend}-{scale}'
        run(name, [root / 'run_calibration.py', baseline, candidate, a.output / name,
                   '--qt', qt, '--backend', backend, '--scale', scale, *libraries])
for qt, baseline, candidate in targets:
    libraries = ['--library-path', a.minimum_library_path] if qt == '6.8.0' and a.minimum_library_path else []
    for backend in ['vulkan', 'opengl', 'software']:
        name = f'correctness-{qt}-{backend}'
        run(name, [root / 'run_motion.py', baseline, candidate, a.output / name, '--qt', qt,
                   '--backend', backend, '--correctness-only', *(['--native'] if backend != 'software' else []), *libraries])
for qt, baseline, candidate in targets:
    libraries = ['--library-path', a.minimum_library_path] if qt == '6.8.0' and a.minimum_library_path else []
    for backend in ['software', 'opengl', 'vulkan']:
        name = f'performance-{qt}-{backend}'
        run(name, [root / 'run_motion.py', baseline, candidate, a.output / name, '--qt', qt,
                   '--backend', backend, '--performance-only', *(['--native'] if backend != 'software' else []), *libraries])
print('Automated stages passed. Explicit owner motion-quality review remains required.')
