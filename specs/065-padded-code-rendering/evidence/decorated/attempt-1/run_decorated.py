#!/usr/bin/env python3
"""Run ordered decorated-content stages, retaining all processes on mismatch."""
import argparse
import gzip
import hashlib
import json
import os
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('build', type=Path)
parser.add_argument('fonts', type=Path)
parser.add_argument('evidence', type=Path, help='Shared gate root for both SDKs and all backends')
parser.add_argument('stage', choices=['cpu', 'static', 'scroll', 'lifecycle'])
parser.add_argument('--qt', choices=['6.8.0', '6.12.0'], required=True)
parser.add_argument('--api', choices=['software', 'opengl', 'vulkan'], default='software')
parser.add_argument('--qt-root', type=Path)
args = parser.parse_args()
args.build, args.fonts, args.evidence = (p.resolve() for p in (args.build, args.fonts, args.evidence))
root = args.evidence
out = root / ('qt-' + args.qt) / args.api
out.mkdir(parents=True, exist_ok=True)


def require(path):
    if not path.exists():
        raise SystemExit(f'Refusing progression: missing {path}')
    report = json.loads(path.read_text())
    if isinstance(report, list):
        passed = len(report) == 2 and all(r.get('exit') == 0 and r.get('failures') == 0 for r in report)
    else:
        passed = report.get('complete') and not report.get('failed')
    if not passed:
        raise SystemExit(f'Refusing progression: {path} did not qualify')


if args.stage == 'cpu':
    for qt in ['6.8.0', '6.12.0']:
        for stage in ['basic', 'extended']:
            require(root / 'prerequisite' / ('qt-' + qt) / (stage + '-summary.json'))
else:
    for qt in ['6.8.0', '6.12.0']:
        require(root / ('qt-' + qt) / 'software' / 'cpu-summary.json')
    if args.stage != 'static':
        prior = 'static' if args.stage == 'scroll' else 'scroll'
        for qt in ['6.8.0', '6.12.0']:
            for api in ['software', 'opengl', 'vulkan']:
                require(root / ('qt-' + qt) / api / (prior + '-summary.json'))

font_names = ['DejaVuSans.ttf', 'DejaVuSans-Oblique.ttf', 'DejaVuSansMono.ttf', 'LiberationSans-Regular.ttf']
font_hashes = {name: hashlib.sha256((args.fonts / name).read_bytes()).hexdigest() for name in font_names}
env = {k: v for k, v in os.environ.items() if not k.startswith(('QT_', 'QSG_', 'QML_'))}
env.update(QT_QPA_PLATFORM='offscreen' if args.api == 'software' else 'wayland',
           QT_QPA_PLATFORMTHEME='', QT_SCALE_FACTOR_ROUNDING_POLICY='PassThrough',
           QT_QPA_UPDATE_IDLE_TIME='0', QSG_INFO='1')
if args.api == 'software':
    env['QT_QUICK_BACKEND'] = 'software'
else:
    env.update(QSG_RHI_BACKEND=args.api, QSG_RENDER_LOOP='threaded', QSG_NO_VSYNC='1')
if args.qt_root:
    env['QT_PLUGIN_PATH'] = str(args.qt_root.resolve() / 'plugins')
records = []


def execute(stage, dpr, order, scale):
    name = f'{stage}-{dpr}-{order}'
    work = out / name
    work.mkdir(parents=True, exist_ok=True)
    command = [str(args.build / 'decorated-content-gate'), str(args.fonts), stage, args.api, str(dpr), order, str(work)]
    local = dict(env, QT_SCALE_FACTOR=str(scale))
    try:
        result = subprocess.run(command, cwd=work, env=local, capture_output=True, timeout=1800)
        stdout, stderr, code = result.stdout, result.stderr, result.returncode
    except subprocess.TimeoutExpired as error:
        stdout, stderr, code = error.stdout or b'', error.stderr or b'', 124
    with gzip.open(out / (name + '.jsonl.gz'), 'wb') as stream:
        stream.write(stdout)
    (out / (name + '.stderr.txt')).write_bytes(stderr)
    record = dict(name=name, command=command, exit=code,
                  environment={k: v for k, v in local.items() if k.startswith(('QT_', 'QSG_')) or k == 'LD_LIBRARY_PATH'})
    records.append(record)
    return record, [json.loads(line) for line in stdout.splitlines()]


def validate_identity(rows, dpr=None):
    identity = [r for r in rows if r['type'] == 'identity']
    if len(identity) != 1 or identity[0]['qt'] != args.qt:
        raise ValueError('Wrong exact SDK identity')
    fonts = [r for r in rows if r['type'] == 'font']
    if len(fonts) != 4 or {Path(r['path']).name: r['sha256'] for r in fonts} != font_hashes:
        raise ValueError('Wrong explicit font identities')
    if not any(r['type'] == 'detector' and r['passed'] for r in rows):
        raise ValueError('Missing exact detector control')
    if args.stage != 'cpu':
        windows = [r for r in rows if r['type'] == 'window']
        if not windows:
            raise ValueError('No observed window')
        for window in windows:
            if window['api'] != args.api or (dpr is not None and abs(window['dpr'] - dpr) > 1e-6):
                raise ValueError('Wrong observed backend/DPR')
            if args.api != 'software' and not window['device'].get('hardware'):
                raise ValueError('Native device is incomplete')
        return windows[0]['dpr']
    return 1


complete, failed, natural = True, False, 1
if args.stage != 'cpu':
    record, rows = execute('calibrate', 0, 'forward', 1)
    try:
        natural = validate_identity(rows)
        if record['exit'] or not rows[-1]['balanced']:
            raise ValueError('Calibration failed')
    except (ValueError, KeyError, IndexError) as error:
        record['validationError'] = str(error)
        complete, failed = False, True

if complete:
    for dpr in [1, 1.25, 1.5, 2]:
        for order in ['forward', 'reverse']:
            record, rows = execute(args.stage, dpr, order, dpr / natural)
            cases = [r for r in rows if r['type'] == 'case']
            expected = {'cpu': 45*16, 'static': 45*16*2, 'scroll': 45*156, 'lifecycle': 45*12}[args.stage]
            try:
                validate_identity(rows, dpr)
                if len(cases) != expected or rows[-1]['type'] != 'summary' or rows[-1]['cases'] != expected:
                    raise ValueError('Incomplete fixed corpus/phase matrix')
                keys = {(r['id'], r.get('background'), r.get('clipped')) for r in cases}
                if len(keys) != expected:
                    raise ValueError('Duplicate cases')
                for case in cases:
                    components = case['components'] if args.stage == 'cpu' else case['runs']
                    count = case.get('componentCount', case.get('runCount'))
                    if len(components) != count or [r['ordinal'] for r in components] != list(range(count)):
                        raise ValueError('Component count/order mismatch')
                    runs = [r for r in components if r['component'] == 'run']
                    backgrounds = [r for r in components if r['component'] == 'background']
                    if components != backgrounds + runs or len(backgrounds) != case['coverage']['backgrounds']:
                        raise ValueError('Background order/count mismatch')
                    if len(runs) != case['coverage']['preparedRuns'] or sum(len(r['glyphs']) for r in runs) != case['glyphCount']:
                        raise ValueError('Run/glyph occurrence mismatch')
                    for run in runs:
                        if any('sourceIndex' not in glyph for glyph in run['glyphs']) or run['fontFile'] not in range(4):
                            raise ValueError('Missing source/font metadata')
                    run = runs[0]
                    actual = [(a+b)*dpr % 1 for a, b in zip(run['drawOrigin'], run['glyphs'][0]['position'])]
                    if any(min(abs(a-b), abs(a-b-1), abs(a-b+1)) > 1e-8 for a, b in zip(actual, case['phase'])):
                        raise ValueError('Physical XY phase mismatch')
                    if args.stage != 'cpu':
                        for path in case['resources']:
                            if [r['ordinal'] for r in path] != list(range(count)) or any(r['atlas'] for r in path):
                                raise ValueError('Resource occurrence/order/atlas mismatch')
                if args.stage in ['scroll', 'lifecycle']:
                    teardown = [r for r in rows if r['type'] == 'teardown']
                    if len(teardown) != 45*(3 if args.stage == 'lifecycle' else 1) or not all(r['balanced'] for r in teardown):
                        raise ValueError('Incomplete resource teardown')
                    kind, count = ('returned', 45*16) if args.stage == 'scroll' else ('recreated', 45*12)
                    checks = [r for r in rows if r['type'] == kind]
                    if len(checks) != count or not all(r['equal'] for r in checks):
                        raise ValueError('Incomplete or unequal retained-output checks')
                summary = rows[-1]
                if record['exit'] != bool(summary['failures']) or not summary['balanced']:
                    raise ValueError('Summary/exit/ownership mismatch')
                metrics = ['shippingFull', 'shippingTile', 'shippingCrop', 'tileCrop'] if args.stage == 'cpu' else ['tileCrop', 'tileFull', 'cropFull', 'freshPainterDiagnostic']
                record.update(cases=len(cases), failures=summary['failures'], coverageFailures=sum(not c['coverage']['passed'] for c in cases),
                              changed={key: sum(c[key]['changed'] for c in cases) for key in metrics},
                              croppedInk=sum(c['croppedInk'] for c in cases), borderInk=sum(c['borderInk'] for c in cases))
                failed |= bool(summary['failures'])
            except (ValueError, KeyError, IndexError) as error:
                record['validationError'] = str(error)
                complete, failed = False, True
            print(json.dumps(record), flush=True)

report = dict(stage=args.stage, qt=args.qt, api=args.api, complete=complete, failed=failed, fonts=font_hashes, records=records)
(out / (args.stage + '-summary.json')).write_text(json.dumps(report, indent=2) + '\n')
raise SystemExit(1 if failed else 0)
