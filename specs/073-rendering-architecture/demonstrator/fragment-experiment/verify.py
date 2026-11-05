#!/usr/bin/env python3
"""Verify the isolated boundary blocker, not fragment-aware layout support."""
import argparse
import copy
import gzip
import hashlib
import json
from pathlib import Path
import subprocess

HERE = Path(__file__).resolve().parent


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    if args.output.exists():
        parser.error('output must be new; evidence is never overwritten')
    args.output.mkdir(parents=True)
    source = json.loads((HERE.parent / 'fixtures.json').read_text())
    scan = dict(source, fixtures=[])
    for fixture in source['fixtures']:
        widths = (range(60, 241) if fixture['id'] in
                  ('wrapping', 'adjacent', 'bidi-objects', 'unicode-code') else
                  (fixture['width']-1, fixture['width'], fixture['width']+1))
        scan['fixtures'].extend(dict(fixture, id=fixture['id']+'@'+str(w), width=w) for w in widths)
    assert len(scan['fixtures']) == 751
    scan_path = args.output / 'scan.json'
    scan_path.write_text(json.dumps(scan, ensure_ascii=False, indent=2)+'\n')
    inputs = [HERE.parent / 'fixtures.json', HERE / 'minimal.json', HERE / 'fragment-edges.patch',
              HERE / 'patch-pin.json', HERE / 'main.cpp', HERE / 'CMakeLists.txt', HERE / 'build.py',
              HERE / 'verify.py', HERE / 'historical_objects.cpp', HERE.parent / 'skia/main.cpp', args.build / 'build.json',
              args.build / 'control/historical-objects',
              *[HERE.parents[3] / 'build-ci/073-skia-demo' / name for name in ('skia-compare', 'skia-selection')],
              *[args.build / name / 'fragment-preflight' for name in ('control', 'patched')],
              *map(Path, source['fonts'].values())]
    before = {str(p.resolve()): digest(p) for p in inputs}
    commands = []
    def run(name, executable, manifest, enabled=False, images=False):
        command = [str(executable.resolve()), str(manifest.resolve()), '--enabled' if enabled else '--disabled']
        if images:
            command.append(str((args.output / (images if isinstance(images, str) else 'images')).resolve()))
        process = subprocess.run(command, capture_output=True, timeout=180)
        commands.append(dict(name=name, argv=command, exit=process.returncode))
        with gzip.open(args.output / (name+'.jsonl.gz'), 'wb') as stream:
            stream.write(process.stdout)
        (args.output / (name+'.stderr')).write_bytes(process.stderr)
        return process, [json.loads(line) for line in process.stdout.splitlines()]
    control = args.build / 'control/fragment-preflight'
    patched = args.build / 'patched/fragment-preflight'
    a, ar = run('control-disabled', control, scan_path)
    b, br = run('patched-disabled', patched, scan_path)
    assert a.returncode == b.returncode == 0, (a.stderr, b.stderr)
    assert ar == br, 'disabled-padding behavior differs'
    minimal = HERE / 'minimal.json'
    c, cr = run('minimal-control', control, minimal, images='control-images')
    d, dr = run('minimal-patched', patched, minimal, True, True)
    assert c.returncode == 0 and d.returncode == 1
    for image in (args.output / 'control-images').glob('*.png'):
        assert image.read_bytes() == (args.output / 'images' / image.name).read_bytes(), 'diagnostic patch changed paint'
    assert [{k: v for k, v in row.items() if k not in ('issues', 'status')} for row in dr] == cr
    first, whole, lam, alef = dr
    assert first['status'] == 'blocked-intracluster-edge'
    assert [r['identity'] for r in first['issues']] == [0, 1]
    assert all(r['edgeByte'] == 2 and r['clusterBytes'] == [0, 4] and r['glyphCount'] == 1 for r in first['issues'])
    assert not whole['issues'] and first['runs'] == whole['runs']
    assert first['sourceToBytes'] == [0, 2, 4]
    assert first['runs'][0]['glyphs'] != lam['runs'][0]['glyphs'] + alef['runs'][0]['glyphs']
    # Same metadata with padding disabled must not require a new shaping boundary.
    zero = json.loads(minimal.read_text());zero['padding'][0] = 0
    zero_path = args.output / 'zero-padding.json';zero_path.write_text(json.dumps(zero)+'\n')
    e, er = run('zero-padding', patched, zero_path, True)
    assert e.returncode == 0 and all(not row['issues'] for row in er)
    invalid = [
        ('combining', 'e\u0301', [dict(start=0, length=1, kind='code')], [], 'grapheme-splitting'),
        ('surrogate', '\U0001f600', [dict(start=0, length=1, kind='code')], [], 'grapheme-splitting'),
        ('fractional-offset', 'ab', [dict(start=0.5, length=1, kind='code')], [], 'noninteger interval'),
        ('negative', 'ab', [dict(start=-1, length=1, kind='code')], [], 'invalid interval'),
        ('out-of-range', 'ab', [dict(start=0, length=3, kind='code')], [], 'invalid interval'),
        ('overlap', 'abc', [dict(start=0, length=2, kind='code'),dict(start=1, length=2, kind='code')], [], 'overlapping'),
        ('partial-object', 'abcd', [dict(start=0, length=2, kind='code')],
         [dict(start=1,length=2,width=10,ascent=8,descent=2)], 'partial object'),
    ]
    validations = []
    for name, text, spans, objects, expected in invalid:
        data = copy.deepcopy(source)
        data['fixtures'] = [dict(id=name, width=190, blocks=[dict(text=text,spans=spans,objects=objects)])]
        path = args.output / (name+'.json');path.write_text(json.dumps(data)+'\n')
        proc, rows = run('invalid-'+name, patched, path, True)
        assert proc.returncode == 2 and not rows and expected in proc.stderr.decode()
        validations.append(name)
    object_rows = [row for row in br if row['fixture'] == 'objects@679']
    (args.output / 'object-diagnostic.json').write_text(json.dumps(object_rows, indent=2)+'\n')
    command = [str((args.build / 'control/historical-objects').resolve()), str((HERE.parent / 'fixtures.json').resolve())]
    historical = subprocess.run(command, capture_output=True, timeout=30)
    assert historical.returncode == 0
    (args.output / 'historical-objects.jsonl').write_bytes(historical.stdout)
    (args.output / 'historical-objects.stderr').write_bytes(historical.stderr)
    commands.append(dict(name='historical-objects', argv=command, exit=historical.returncode))
    diagnosis = json.loads(historical.stdout.splitlines()[-1])
    assert diagnosis['objects'][0]['containedAtTolerance005'] is False
    summary = dict(outcome='blocked', regressionPassed=True,
        mechanism='source-span edge preflight against unchanged upstream shaped clusters',
        disabledEquivalence=dict(fixtures=751, blocks=len(ar), exactJSON=True,
            scope='diagnostic patch only; no padding formatter implemented'),
        minimal=dict(source=first['source'], issues=first['issues'],
            joinedGlyphs=first['runs'][0]['glyphs'],
            separatelyShapedGlyphs=lam['runs'][0]['glyphs']+alef['runs'][0]['glyphs']),
        invalidRangesRejected=validations, objectContainmentTolerance=0.05,
        disabledObjectFailures=sum(not obj['contained'] for row in br for obj in row['objects']),
        historicalObjectDiagnostic=diagnosis,
        gates=dict(unmodified='pass for preflight patch', geometry='blocked before padded candidate fit',
            consistency='blocked: adjacent span edge inside shaped glyph', native='not run', ownerVisual='pending'),
        commands=commands, inputsBefore=before,
        inputsAfter={str(p.resolve()): digest(p) for p in inputs})
    assert summary['inputsBefore'] == summary['inputsAfter']
    summary['outputHashes'] = {p.relative_to(args.output).as_posix(): digest(p) for p in sorted(args.output.rglob('*')) if p.is_file()}
    (args.output / 'summary.json').write_text(json.dumps(summary, ensure_ascii=False, indent=2)+'\n')
    print(json.dumps({k: summary[k] for k in ('outcome','regressionPassed','disabledEquivalence','minimal','disabledObjectFailures')}, ensure_ascii=False))


if __name__ == '__main__':
    main()
