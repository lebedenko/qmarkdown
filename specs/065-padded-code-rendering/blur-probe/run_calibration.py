#!/usr/bin/env python3
"""Focused native-only control calibration; stop at the first unresolved stage."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

p = argparse.ArgumentParser()
p.add_argument('baseline', type=Path)
p.add_argument('candidate', type=Path)
p.add_argument('output', type=Path)
p.add_argument('--qt', required=True, choices=['6.8.0', '6.12.0'])
p.add_argument('--backend', choices=['software', 'opengl', 'vulkan'], default='software')
p.add_argument('--scale', type=float, default=1.25)
p.add_argument('--library-path')
a = p.parse_args()
a.output.mkdir(parents=True, exist_ok=False)
def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()
results = []
for variant, binary in [('baseline', a.baseline.resolve()), ('candidate', a.candidate.resolve())]:
    directory = (a.output / variant).resolve()
    directory.mkdir()
    env = dict(os.environ, QT_QPA_PLATFORM='offscreen' if a.backend == 'software' else 'wayland',
               QT_SCALE_FACTOR=str(a.scale), QSG_INFO='1', QT_FORCE_STDERR_LOGGING='1')
    for key in ['BLUR_FIXTURE', 'BLUR_CALIBRATION', 'QT_QUICK_BACKEND', 'QSG_RHI_BACKEND']:
        env.pop(key, None)
    env['QT_QUICK_BACKEND' if a.backend == 'software' else 'QSG_RHI_BACKEND'] = a.backend
    if a.library_path:
        env['LD_LIBRARY_PATH'] = a.library_path
    with (directory / 'results.jsonl').open('w') as out, (directory / 'stderr.log').open('w') as err:
        status = subprocess.run([str(binary), 'calibrate', str(directory)], env=env, stdout=out, stderr=err).returncode
    rows = [json.loads(line) for line in (directory / 'results.jsonl').read_text().splitlines() if line.startswith('{')]
    environment = next((r for r in rows if r.get('type') == 'environment'), {})
    source = environment.get('source', {})
    raw = [r for r in rows if r.get('type') == 'rawCalibration']
    captures = [r for r in rows if r.get('type') == 'calibration']
    identity = (source.get('policy') == 'native-only-v2' and source.get('variant') == variant
                and source.get('commit') == '43d6bba842bb5b7ab23889e49d5f138c6f22a7fb'
                and environment.get('qt') == a.qt
                and environment.get('graphicsApi') == {'software': 1, 'opengl': 3, 'vulkan': 5}[a.backend]
                and environment.get('dpr') == (a.scale if a.backend == 'software' else 1.25))
    complete = len(raw) == 3 and len(captures) == 48
    valid = identity and complete and status == 0 and all(r['equal'] for r in raw + captures)
    stage = ('identity-or-environment' if not identity else 'unsupported-paint-device' if any(not r['imageDevice'] for r in raw) else 'raw-raster' if any(not r['equal'] for r in raw)
             else 'same-coordinate-framebuffer' if any(not r['equal'] for r in captures)
             else 'complete' if valid else 'missing-evidence-or-counter-guard')
    libraries = list(binary.parent.glob('qmarkdown/src/QMarkdown/libQMarkdown.so*'))
    results.append(dict(variant=variant, exit=status, valid=valid, stage=stage, complete=complete,
                        environment=environment, binarySha256=digest(binary),
                        harnessSha256=digest(Path(__file__).with_name('motion.cpp')),
                        runnerSha256=digest(Path(__file__)),
                        libraries={str(f): digest(f) for f in libraries if f.is_file()},
                        files={f.name: digest(f) for f in directory.iterdir() if f.is_file()}))
    (a.output / 'summary.json').write_text(json.dumps(results, indent=2) + '\n')
    print(variant, stage, 'PASS' if valid else 'FAIL', flush=True)
    if not valid:
        raise SystemExit('Control calibration stopped qualification; downstream checks not reached.')
