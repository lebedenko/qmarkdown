#!/usr/bin/env python3
"""Serial exact-SDK qualification. Retain logs even when gates fail."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import statistics
import subprocess

p = argparse.ArgumentParser()
p.add_argument('baseline', type=Path)
p.add_argument('candidate', type=Path)
p.add_argument('output', type=Path)
p.add_argument('--qt', required=True, choices=['6.8.0', '6.12.0'])
p.add_argument('--native', action='store_true')
p.add_argument('--performance-only', action='store_true')
p.add_argument('--continue-matrix', action='store_true', help='retain failures and continue independent configurations')
p.add_argument('--library-path')
p.add_argument('--resume', action='store_true', help='reuse recorded identical binaries; preserve interrupted attempts')
a = p.parse_args()
a.output.mkdir(parents=True, exist_ok=a.resume)
results_file = a.output / 'runs.json'
results = json.loads(results_file.read_text()) if a.resume and results_file.exists() else []
def run(variant, mode, scale, api='software', pair=None):
    name = f'{api}-{scale}-{variant}-{mode}' + (f'-{pair}' if pair is not None else '')
    binary = (a.baseline if variant == 'baseline' else a.candidate).resolve()
    digest = hashlib.sha256(binary.read_bytes()).hexdigest()
    previous = next((r for r in results if r['name'] == name), None)
    directory = a.output / name
    if previous:
        if previous['binarySha256'] != digest:
            raise SystemExit(f'Refusing to resume changed binary: {binary}')
        rows = [json.loads(line) for line in (directory / 'results.jsonl').read_text().splitlines() if line.startswith('{')]
        print(name, 'REUSED', 'PASS' if previous['valid'] else 'FAIL', flush=True)
        return previous['valid'], rows
    if directory.exists() and a.resume:
        index = 1
        while directory.with_name(name + f'-interrupted-{index}').exists():
            index += 1
        directory.rename(directory.with_name(name + f'-interrupted-{index}'))
    directory.mkdir()
    env = dict(os.environ, QT_QPA_PLATFORM='wayland' if a.native else 'offscreen', QT_SCALE_FACTOR=str(scale), QSG_INFO='1', QT_FORCE_STDERR_LOGGING='1')
    env.pop('QT_QUICK_BACKEND', None)
    env.pop('BLUR_FIXTURE', None)
    if api == 'software':
        env['QT_QUICK_BACKEND'] = 'software'
    else:
        env['QSG_RHI_BACKEND'] = api
    if a.library_path:
        env['LD_LIBRARY_PATH'] = a.library_path
    binary = (a.baseline if variant == 'baseline' else a.candidate).resolve()
    command = [str(binary), mode, str(directory)]
    with (directory / 'results.jsonl').open('w') as out, (directory / 'stderr.log').open('w') as err:
        status = subprocess.run(command, env=env, stdout=out, stderr=err).returncode
    rows = [json.loads(line) for line in (directory / 'results.jsonl').read_text().splitlines() if line.startswith('{')]
    environment = next((r for r in rows if r.get('type') == 'environment'), {})
    expected_dpr = 1.25 if a.native else float(scale)
    expected_api = {'software': 1, 'opengl': 3, 'vulkan': 5}[api]
    fixtures = {r['fixture'] for r in rows if r.get('type') == 'fixture'}
    captures = [r for r in rows if r.get('type') == 'capture']
    complete = (len(fixtures) == 12 and len(captures) == 216) if mode == 'static' else (len(fixtures) == 12 and len([r for r in rows if r.get('type') == 'motionSummary']) == 24) if mode == 'motion' else len([r for r in rows if r.get('type') == 'performance']) == 2
    valid = complete and status == 0 and environment.get('qt') == a.qt and environment.get('dpr') == expected_dpr and environment.get('graphicsApi') == expected_api
    result = dict(name=name, command=command, exit=status, complete=complete, captures=len(captures), environment=environment, valid=valid, binarySha256=hashlib.sha256(binary.read_bytes()).hexdigest(), pair=pair)
    results.append(result)
    (a.output / 'runs.json').write_text(json.dumps(results, indent=2)+'\n')
    print(name, status, 'PASS' if valid else 'FAIL', flush=True)
    return valid, rows

if not a.performance_only:
    for api in (['opengl', 'vulkan'] if a.native else ['software']):
        for scale in ([1] if a.native else [1, 1.25, 1.5, 2]):
            snapshots = {}
            for variant in ['baseline', 'candidate']:
                for mode in ['static', 'motion']:
                    valid, rows = run(variant, mode, scale, api)
                    if not valid and not a.continue_matrix:
                        raise SystemExit('Qualification stopped at failing gate; see retained run logs.')
                    if mode == 'static':
                        snapshots[variant] = {r['fixture']: {k:r[k] for k in ['text', 'width', 'height', 'cells', 'interactions', 'fonts', 'surfaceCount', 'allocationBytes']} for r in rows if r.get('type') == 'fixture'}
                        for fixture in snapshots[variant].values():
                            fixture['fonts'] = sorted({json.dumps(font, sort_keys=True) for font in fixture['fonts']})
            geometry_equal = len(snapshots['baseline']) == 12 and snapshots['baseline'] == snapshots['candidate']
            geometry_name = f'{api}-{scale}-geometry'
            results[:] = [r for r in results if r['name'] != geometry_name]
            results.append(dict(name=geometry_name, valid=geometry_equal))
            (a.output / 'runs.json').write_text(json.dumps(results, indent=2)+'\n')
            if not geometry_equal and not a.continue_matrix:
                raise SystemExit('Baseline/candidate fixture geometry, allocation or fonts differ')

pairs = []
for pair in range(5):
    for variant in (['baseline', 'candidate'] if pair % 2 == 0 else ['candidate', 'baseline']):
        valid, rows = run(variant, 'performance', 1 if a.native else 1.25, 'opengl' if a.native else 'software', pair)
        if not valid:
            raise SystemExit('Performance run failed')
        pairs.extend(dict(r, variant=variant, pair=pair) for r in rows if r.get('type') == 'performance')
summary = []
for dense in [False, True]:
    group = [r for r in pairs if r['dense'] == dense]
    row = {'dense': dense, 'timing': 'capture trace, not display frame rate'}
    for metric in ['preparationNs', 'medianNs', 'p95Ns']:
        ratios = [next(r[metric] for r in group if r['pair']==i and r['variant']=='candidate') / next(r[metric] for r in group if r['pair']==i and r['variant']=='baseline') for i in range(5)]
        row[metric] = dict(pairedRatios=ratios, medianRatio=statistics.median(ratios), repeatableRegression=sum(r>1.1 for r in ratios)>=3)
    row['records'] = group
    row['allocationEqual'] = len({r['allocationBytes'] for r in group}) == 1
    row['surfacesEqual'] = all(r['surfaces'] == 12 for r in group)
    row['zeroPaints'] = all(r['paints'] == 0 for r in group)
    row['fontFilesEqual'] = len({json.dumps(sorted({json.dumps(f,sort_keys=True) for f in r['fonts']})) for r in group}) == 1
    summary.append(row)
(a.output / 'performance.json').write_text(json.dumps(summary, indent=2)+'\n')
if any(not all(r[k] for k in ['allocationEqual','surfacesEqual','zeroPaints','fontFilesEqual']) for r in summary):
    raise SystemExit('Performance identity/counter gate failed')
if any(r[m]['repeatableRegression'] for r in summary for m in ['preparationNs','medianNs','p95Ns']):
    raise SystemExit('Repeatable regression >10%; candidate rejected')

if not all(r['valid'] for r in results):
    raise SystemExit('Qualification failed; independent run results retained')
