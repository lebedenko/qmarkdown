#!/usr/bin/env python3
"""Run complete software stages in fresh processes; retain raw metrics and commands."""
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
parser.add_argument('stage', choices=['historical', 'basic', 'extended'])
args = parser.parse_args()
args.build = args.build.resolve()
args.fonts = args.fonts.resolve()
args.evidence = args.evidence.resolve()
args.evidence.mkdir(parents=True, exist_ok=True)
env = dict(os.environ, QT_QPA_PLATFORM='offscreen', QT_QUICK_BACKEND='software')
records = []

def execute(name, command, extra=None):
    work = args.evidence / name
    work.mkdir(parents=True, exist_ok=True)
    result = subprocess.run([str(x) for x in command], env=dict(env, **(extra or {})), cwd=work, capture_output=True)
    (args.evidence / (name+'.stderr.txt')).write_bytes(result.stderr)
    with gzip.open(args.evidence / (name+'.jsonl.gz'), 'wb') as stream:
        stream.write(result.stdout)
    records.append(dict(name=name, command=[str(x) for x in command], environment=extra or {}, exit=result.returncode))
    return result

failed = False
for order in ['forward', 'reverse']:
    name = args.stage+'-'+order
    result = execute(name, [args.build/'whole-run-gate', args.fonts/'DejaVuSans-Oblique.ttf',
                           args.fonts/'DejaVuSansMono.ttf', args.stage, order, args.evidence/name])
    rows = [json.loads(line) for line in result.stdout.splitlines()]
    cases = [row for row in rows if row['type'] == 'case']
    expected = dict(historical=2, basic=256, extended=960)[args.stage]
    assert len(cases) == expected and len({c['id'] for c in cases}) == expected
    assert rows[-1]['cases'] == expected
    for case in cases:
        assert case['runCount'] == len(case['runs']) > 0
        assert case['glyphCount'] == sum(len(run['glyphs']) for run in case['runs']) == case['glyphEmissions']
        assert [run['ordinal'] for run in case['runs']] == list(range(case['runCount']))
        for path in ['direct', 'tile', 'crop']:
            assert case[path+'Emissions'] == case['runCount']
        if args.stage != 'historical':
            run = case['runs'][0]
            actual = [(a+b)*case['dpr'] % 1 for a,b in zip(run['drawOrigin'],run['glyphs'][0]['position'])]
            assert all(min(abs(a-b), abs(a-b-1), abs(a-b+1)) < 1e-8 for a,b in zip(actual,case['phase']))
    failures = sum(c['failed'] for c in cases)
    assert rows[-1]['failures'] == failures and result.returncode == bool(failures)
    records[-1].update(cases=expected, failures=failures,
                       tileChanged=sum(c['tileChanged'] for c in cases),
                       cropChanged=sum(c['cropChanged'] for c in cases),
                       glyphChanged=sum(c['glyphChanged'] for c in cases),
                       croppedInk=sum(c['croppedInk'] for c in cases),
                       borderInk=sum(c['borderInk'] for c in cases))
    failed |= bool(failures)

if args.stage == 'historical':
    # Original environment-resolved historical binary, independent fresh processes.
    for repeat in range(2):
        for dpr in ['1', '1.25']:
            name = f'legacy-{repeat}-{dpr}'
            result = execute(name, [args.build/'retained-glyph-gate'], {'GATE_DPR':dpr})
            # This control has plain-text stdout, retained under its own name.
            (args.evidence/(name+'.txt')).write_bytes(result.stdout)
    for order in ['forward', 'reverse']:
        execute('diagnostic-'+order, [args.build/'retained-glyph-diagnostic'],
                {'DIAG_REVERSE':str(int(order=='reverse')), 'DIAG_CAPTURES':str(args.evidence/('diagnostic-'+order))})
(args.evidence/(args.stage+'-summary.json')).write_text(json.dumps(records,indent=2)+'\n')
raise SystemExit(1 if failed else 0)
