#!/usr/bin/env python3
"""Execute a complete SDK/backend stage in fresh DPR/order processes."""
import argparse
import gzip
import json
import os
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('build', type=Path)
parser.add_argument('fonts', type=Path)
parser.add_argument('evidence', type=Path)
parser.add_argument('stage', choices=['static', 'scroll', 'lifecycle'])
parser.add_argument('api', choices=['software', 'opengl', 'vulkan'])
parser.add_argument('--qt-root', type=Path, help='Existing relocated SDK for native platform plugins')
parser.add_argument('--qt', required=True, choices=['6.8.0', '6.12.0'])
args = parser.parse_args()
args.build, args.fonts, args.evidence = (p.resolve() for p in (args.build, args.fonts, args.evidence))
args.evidence.mkdir(parents=True, exist_ok=True)
# evidence is ROOT/qt-X/API. Do not advance until every required prior target passes.
gate_root = args.evidence.parent.parent
if args.stage != 'static':
    prior = 'static' if args.stage == 'scroll' else 'scroll'
    for qt in ['6.8.0', '6.12.0']:
        for api in ['software', 'opengl', 'vulkan']:
            path = gate_root/f'qt-{qt}'/api/(prior+'-summary.json')
            if not path.exists():
                raise SystemExit(f'Refusing progression: missing {qt}/{api}/{prior} evidence')
            report = json.loads(path.read_text())
            if not report['complete'] or report['failed']:
                raise SystemExit(f'Refusing progression: {qt}/{api}/{prior} did not qualify')

env = {k:v for k,v in os.environ.items() if not k.startswith(('QT_', 'QSG_', 'QML_'))}
env.update(QT_QPA_PLATFORM='offscreen' if args.api == 'software' else 'wayland',
           QT_QPA_PLATFORMTHEME='', QT_SCALE_FACTOR_ROUNDING_POLICY='PassThrough',
           QT_QPA_UPDATE_IDLE_TIME='0', QSG_INFO='1', QT_LOGGING_RULES='qt.rhi.*=true;qt.scenegraph.general=true')
if args.api == 'software':
    env['QT_QUICK_BACKEND'] = 'software'
else:
    env['QSG_RHI_BACKEND'] = args.api
    env['QSG_RENDER_LOOP'] = 'threaded'
    env['QSG_NO_VSYNC'] = '1'
if args.qt_root:
    env['QT_PLUGIN_PATH'] = str(args.qt_root.resolve()/'plugins')
records = []

def execute(name, stage, dpr, order, scale):
    work = args.evidence/name
    work.mkdir(parents=True, exist_ok=True)
    command = [str(args.build/'scenegraph-run-gate'), str(args.fonts), stage, args.api, str(dpr), order, str(work)]
    local_env = dict(env, QT_SCALE_FACTOR=str(scale))
    try:
        result = subprocess.run(command, cwd=work, env=local_env, capture_output=True, timeout=1200)
        stdout, stderr, code = result.stdout, result.stderr, result.returncode
    except subprocess.TimeoutExpired as error:
        stdout, stderr, code = error.stdout or b'', error.stderr or b'', 124
    with gzip.open(args.evidence/(name+'.jsonl.gz'), 'wb') as stream:
        stream.write(stdout)
    (args.evidence/(name+'.stderr.txt')).write_bytes(stderr)
    record = dict(name=name, command=command, environment={k:local_env[k] for k in local_env if k.startswith(('QT_', 'QSG_')) or k == 'LD_LIBRARY_PATH'}, exit=code)
    records.append(record)
    rows = [json.loads(line) for line in stdout.splitlines()]
    return record, rows

def identity(rows, requested=None):
    detectors = [r for r in rows if r['type'] == 'detector']
    if len(detectors) != 1 or detectors[0] != dict(type='detector', knownChanged=2, knownMaxChannelDelta=40, passed=True):
        raise ValueError('Exact pixel detector self-test missing or wrong')
    identities = [r for r in rows if r['type'] == 'identity']
    windows = [r for r in rows if r['type'] == 'window']
    if len(identities) != 1 or identities[0]['qt'] != args.qt or not windows:
        raise ValueError('Missing or wrong runtime SDK/window identity')
    for window in windows:
        if window['api'] != args.api:
            raise ValueError('Runtime backend mismatch')
        if requested is not None and abs(window['dpr']-requested) > 1e-6:
            raise ValueError('Runtime DPR mismatch')
        if args.api != 'software' and not window['device'].get('hardware'):
            raise ValueError('Native hardware was not confirmed')
        if args.api != 'software' and window['platform'] != 'wayland':
            raise ValueError('Native Wayland platform was not confirmed')
    return windows[0]['dpr']

calibration, rows = execute(args.stage+'-calibration', 'calibrate', 0, 'forward', 1)
try:
    natural = identity(rows)
    if calibration['exit'] != 0 or rows[-1]['type'] != 'summary' or not rows[-1]['balanced']:
        raise ValueError('Calibration did not complete')
except (ValueError, KeyError, IndexError) as error:
    report = dict(stage=args.stage, qt=args.qt, api=args.api, complete=False, failed=True,
                  reason=str(error), records=records)
    (args.evidence/(args.stage+'-summary.json')).write_text(json.dumps(report, indent=2)+'\n')
    raise SystemExit(2)

failed = False
complete = True
for dpr in [1, 1.25, 1.5, 2]:
    for order in ['forward', 'reverse']:
        record, rows = execute(f'{args.stage}-{dpr}-{order}', args.stage, dpr, order, dpr/natural)
        cases = [r for r in rows if r['type'] == 'case']
        expected = (610 if dpr in [1, 1.25] else 608) if args.stage == 'static' else 936 if args.stage == 'scroll' else 12
        try:
            identity(rows, dpr)
            if len(cases) != expected or rows[-1]['type'] != 'summary' or rows[-1]['cases'] != expected:
                raise ValueError('Incomplete matrix')
            distinct = {(r['id'], r['background'], r['clipped']) for r in cases}
            if len(distinct) != expected:
                raise ValueError('Duplicate matrix cases')
            for case in cases:
                if case['runCount'] != len(case['runs']) or case['glyphCount'] != sum(len(r['glyphs']) for r in case['runs']):
                    raise ValueError('Occurrence metadata mismatch')
                if [r['ordinal'] for r in case['runs']] != list(range(case['runCount'])):
                    raise ValueError('Prepared run order mismatch')
                for path in case['resources']:
                    if [r['ordinal'] for r in path] != list(range(case['runCount'])) or any(r['atlas'] for r in path):
                        raise ValueError('Resource count/order/atlas mismatch')
                if not case['historical']:
                    run = case['runs'][0]
                    actual = [(a+b)*dpr % 1 for a,b in zip(run['drawOrigin'],run['glyphs'][0]['position'])]
                    if any(min(abs(a-b),abs(a-b-1),abs(a-b+1)) > 1e-8 for a,b in zip(actual,case['phase'])):
                        raise ValueError('Prepared physical phase mismatch')
                required_failure = any(case[k]['changed'] for k in ['tileCrop','tileFull','cropFull']) or case['preparationChanged'] or case['croppedInk'] or case['clipLeaks'] or not case['positionMatches'] or not case['stableResources'] or not case['orderedCounts']
                if bool(required_failure) != case['failed']:
                    raise ValueError('Inconsistent case acceptance')
            if args.stage == 'scroll':
                returned = [r for r in rows if r['type'] == 'returned']
                if len(returned) != 96 or not all(r['equal'] for r in returned):
                    raise ValueError('Missing or unequal return-to-origin checks')
            if args.stage == 'lifecycle':
                recreated = [r for r in rows if r['type'] == 'recreated']
                if len(recreated) != 12 or not all(r['equal'] for r in recreated):
                    raise ValueError('Missing or unequal recreated captures')
            if args.stage in ['scroll', 'lifecycle']:
                teardown = [r for r in rows if r['type'] == 'teardown']
                if len(teardown) != (6 if args.stage == 'scroll' else 3) or not all(r['balanced'] for r in teardown):
                    raise ValueError('Missing or unbalanced teardown checks')
            summary = rows[-1]
            if record['exit'] != bool(summary['failures']):
                raise ValueError('Exit status disagrees with summary')
            if not summary['balanced']:
                raise ValueError('Unbalanced final resources')
            record.update(cases=len(cases), failures=summary['failures'],
                          changed={k:sum(r[k]['changed'] for r in cases) for k in ['tileCrop','tileFull','cropFull','freshPainterDiagnostic']},
                          maxChannelDelta={k:max(r[k]['maxChannelDelta'] for r in cases) for k in ['tileCrop','tileFull','cropFull','freshPainterDiagnostic']},
                          borderInk=sum(r['borderInk'] for r in cases), croppedInk=sum(r['croppedInk'] for r in cases),
                          clipLeaks=sum(r['clipLeaks'] for r in cases))
            failed |= bool(summary['failures'])
        except (ValueError, KeyError, IndexError) as error:
            record['validationError'] = str(error)
            complete = False
            failed = True
        print(json.dumps({k:v for k,v in record.items() if k not in ['command','environment']}), flush=True)
        # A mismatch never discards the rest of this stage or the reverse process.
report = dict(stage=args.stage, qt=args.qt, api=args.api, complete=complete, failed=failed,
              naturalDpr=natural, records=records)
(args.evidence/(args.stage+'-summary.json')).write_text(json.dumps(report, indent=2)+'\n')
raise SystemExit(1 if failed else 0)
