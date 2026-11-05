#!/usr/bin/env python3
"""Summarize native-only stages without turning missing evidence into success."""
import argparse
import gzip
import hashlib
import json
from pathlib import Path
import shutil

p = argparse.ArgumentParser()
p.add_argument('source', type=Path)
p.add_argument('destination', type=Path)
a = p.parse_args()
a.destination.mkdir(parents=True, exist_ok=False)
stages = json.loads((a.source / 'stages.json').read_text())
expected = [f'calibration-{qt}-{backend}-{scale}' for qt in ['6.8.0', '6.12.0']
            for backend, scale in [('software', 1.25), ('software', 1.5), ('vulkan', 1)]]
expected += [f'{mode}-{qt}-{backend}' for mode in ['correctness', 'performance']
             for qt in ['6.8.0', '6.12.0'] for backend in
             (['vulkan', 'opengl', 'software'] if mode == 'correctness' else ['software', 'opengl', 'vulkan'])]
summary = dict(policy='native-only-v2', ownerReview='pending', qualified=False,
               source='43d6bba842bb5b7ab23889e49d5f138c6f22a7fb',
               timing='capture-trace, not display frame rate', stages=stages,
               artifactGaps=json.loads((a.source / 'artifact-gaps.json').read_text()) if (a.source / 'artifact-gaps.json').exists() else [],
               notReached=[name for name in expected if name not in {s['name'] for s in stages}],
               binaries=json.loads((a.source / 'binaries.json').read_text()),
               host=json.loads((a.source / 'host.json').read_text()) if (a.source / 'host.json').exists() else None, evidence=[])
for stage in stages:
    directory = a.source / stage['name']
    entry = dict(name=stage['name'])
    for name in ['summary.json', 'runs.json', 'performance.json']:
        path = directory / name
        if path.exists():
            entry[name] = json.loads(path.read_text())
    entry['failureCategory'] = None
    if stage['exit']:
        runs = entry.get('runs.json', entry.get('summary.json', []))
        messages = ' '.join(str(r.get('failingStage', r.get('stage', ''))) for r in runs if not r.get('valid', True))
        entry['failureCategory'] = ('performance' if stage['name'].startswith('performance-')
            else 'replay' if 'replay-failure' in messages
            else 'control-calibration' if 'control-calibration' in messages or any(r.get('stage') in ['raw-raster', 'same-coordinate-framebuffer', 'unsupported-paint-device'] for r in runs)
            else 'missing-evidence' if 'identity-or-environment' in messages or any(r.get('exit', 0) < 0 or not r.get('complete', True) and not r.get('failingStage') for r in runs)
            else 'renderer')
    entry['traces'] = []
    for path in sorted(directory.glob('*/results.jsonl')):
        rows = [json.loads(line) for line in path.read_text().splitlines() if line.startswith('{')]
        counts = {kind: sum(r.get('type') == kind for r in rows) for kind in {r.get('type') for r in rows}}
        failed = [r for r in rows if r.get('equal') is False and r.get('type') in ['rawCalibration', 'calibration', 'replay', 'poseControl']]
        observations = [r for r in rows if r.get('type') == 'translatedDiagnostic' and r.get('equal') is False]
        trace = dict(path=str(path), sha256=hashlib.sha256(path.read_bytes()).hexdigest(), counts=counts, failures=failed, translatedDiagnosticDifferences=observations,
                     fonts=[json.loads(f) for f in sorted({json.dumps(font, sort_keys=True) for r in rows for font in r.get('fonts', [])})])
        stderr = path.with_name('stderr.log')
        if stderr.exists():
            trace['backendLogSha256'] = hashlib.sha256(stderr.read_bytes()).hexdigest()
            trace['backendDetails'] = [line for line in stderr.read_text().splitlines() if any(key in line.lower() for key in ['device', 'driver', 'opengl', 'vulkan', 'vendor', 'renderer'])]
        entry['traces'].append(trace)
        if failed or stage['exit']:
            stem = stage['name'] + '-' + path.parent.name
            (a.destination / (stem + '.jsonl.gz')).write_bytes(gzip.compress(path.read_bytes(), mtime=0))
            stderr = path.with_name('stderr.log')
            if stderr.exists():
                shutil.copyfile(stderr, a.destination / (stem + '.stderr.log'))
            for capture in path.parent.glob('*.png'):
                if capture.name.endswith(('-source.png', '-control.png', '-live.png', '-replay.png')):
                    shutil.copyfile(capture, a.destination / (stem + '-' + capture.name))
    summary['evidence'].append(entry)
(a.destination / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
print('Recorded', len(stages), 'stages;', len(summary['notReached']), 'not reached; owner review pending.')
