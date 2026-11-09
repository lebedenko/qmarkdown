#!/usr/bin/env python3
"""Run an isolated actual-source scan; preserve failures and binary identities."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('executable', type=Path)
p.add_argument('output', type=Path)
p.add_argument('--evidence', type=Path, required=True)
p.add_argument('--backend', choices=['opengl', 'vulkan'], default='opengl')
p.add_argument('--pose', type=float)
p.add_argument('--unaligned', action='store_true')
a = p.parse_args()
a.output.mkdir(parents=True, exist_ok=False)
exe = a.executable.resolve()
files = [exe, *exe.parent.glob('**/libQMarkdown.so*'),
         Path(__file__).with_name('review.cpp'), Path(__file__).with_name('scene.h')]
identities = {str(f): hashlib.sha256(f.read_bytes()).hexdigest() for f in files if f.is_file()}
(a.output / 'identities.json').write_text(json.dumps(identities, indent=2) + '\n')
env = dict(os.environ, QT_QPA_PLATFORM='wayland', QSG_RHI_BACKEND=a.backend,
           QT_SCALE_FACTOR='1', BLUR_SCAN='1')
for name in ('BLUR_POSE', 'BLUR_UNALIGNED'):
    env.pop(name, None)
if a.pose is not None:
    env['BLUR_POSE'] = str(a.pose)
if a.unaligned:
    env['BLUR_UNALIGNED'] = '1'
with (a.output / 'stdout.jsonl').open('w') as out, (a.output / 'stderr.log').open('w') as err:
    result = subprocess.run([str(exe), str(a.evidence.resolve()),
                             str((a.output / 'captures').resolve()), a.backend],
                            env=env, stdout=out, stderr=err, check=False)
(a.output / 'result.json').write_text(json.dumps({'exit': result.returncode,
    'status': 'scan completed; visual classification still required' if result.returncode == 0
    else 'stopped; inspect failing stage before attribution'}, indent=2) + '\n')
raise SystemExit(result.returncode)
