#!/usr/bin/env python3
"""Run boundary, equivalence and geometry gates; retain failures, never qualify native early."""
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
    args.output.mkdir(parents=True, exist_ok=False)
    source = json.loads((HERE.parent / 'fixtures.json').read_text())
    commands, failures = [], []
    inputs = [*HERE.glob('*.*'), *map(Path, source['fonts'].values()),
              args.build/'patched/fragment-preflight', args.build/'control/fragment-preflight',
              HERE.parent/'fixtures.json']
    before = {str(p.resolve()): digest(p) for p in inputs if p.is_file()}

    def run(name, data, enabled=True, control=False, expected=0):
        path = args.output/(name+'.json')
        path.write_text(json.dumps(data, ensure_ascii=True, indent=2)+'\n')
        command = [str((args.build/('control' if control else 'patched')/'fragment-preflight').resolve()),
                   str(path.resolve()), '--enabled' if enabled else '--disabled']
        proc = subprocess.run(command, capture_output=True, timeout=180)
        commands.append(dict(name=name, argv=command, exit=proc.returncode))
        with gzip.open(args.output/(name+'.jsonl.gz'), 'wb') as stream:
            stream.write(proc.stdout)
        (args.output/(name+'.stderr')).write_bytes(proc.stderr)
        rows = [json.loads(line) for line in proc.stdout.splitlines()]
        if proc.returncode != expected:
            failures.append(dict(gate=name, exit=proc.returncode, stderr=proc.stderr.decode()))
        return rows

    minimal = json.loads((HERE/'minimal.json').read_text())
    boundary = copy.deepcopy(minimal); boundary['padding'] = [0, 0]
    rows = run('boundary-minimal', boundary)
    assert len(rows) == 4 and all(not row['issues'] for row in rows)
    assert [r['glyphs'] for r in rows[0]['runs']] == [[1141], [1117]]
    assert rows[1]['runs'][0]['glyphs'] == [3254]
    regressions = copy.deepcopy(source)
    def case(name, text, ranges, width=190, objects=None):
        return dict(id=name,width=width,blocks=[dict(text=text,
            spans=[dict(start=a,length=n,kind='code') for a,n in ranges],objects=objects or [])])
    regressions['fonts']['mono'] = regressions['fonts']['regular']
    regressions['fixtures'] = [
        case('latin-adjacent','ffi',[(0,1),(1,2)]),
        case('latin-whole','ffi',[(0,3)]),
        case('joining-adjacent','بت',[(0,1),(1,1)]),
        case('joining-whole','بت',[(0,2)]),
        case('mixed-bidi','a אב b',[(0,3),(3,3)]),
        case('combining','e\u0301x',[(0,2),(2,1)]),
        case('surrogate','🙂x',[(0,2),(2,1)]),
        case('fallback','x漢🙂y',[(0,5)]),
        case('object-edges','a[obj]b',[(0,1),(1,5),(6,1)],objects=[dict(start=1,length=5,width=48,ascent=28,descent=8)]),
        case('oversized','🙂',[(0,2)],width=5),
        case('zero-width','ab',[(0,2)],width=0),
        case('hard-break','a\nb',[(0,3)],width=30),
        case('whitespace','   ',[(0,3)],width=30),
        case('trailing-whitespace','a   b ',[(0,6)],width=30),
    ]
    regression_zero = dict(regressions,padding=[0,0])
    regression_baseline = run('regression-baseline',regression_zero)
    regression_control = run('regression-control',regression_zero,False,True)
    assert [r['breakFlags'] for r in regression_control] == [r['breakFlags'] for r in regression_baseline]
    reg = {r['fixture']:r for r in regression_baseline}
    assert [g for r in reg['latin-adjacent']['runs'] for g in r['glyphs']] != [g for r in reg['latin-whole']['runs'] for g in r['glyphs']]
    assert [g for r in reg['joining-adjacent']['runs'] for g in r['glyphs']] != [g for r in reg['joining-whole']['runs'] for g in r['glyphs']]
    regression_padded = run('regression-padded',regressions)
    for row in regression_baseline:
        assert not row['issues'], row
    if len(regression_padded) != len(regression_baseline):
        failures.append(dict(gate='regressions',reason='incomplete output'))
    for base,row in zip(regression_baseline,regression_padded):
        if base['runs'] != row['runs'] or base['breakFlags'] != row['breakFlags']:
            failures.append(dict(fixture=row['fixture'],reason='padding changed shaping or breaks'))
    scan = dict(source, fixtures=[])
    for fixture in source['fixtures']:
        widths = (range(60, 241) if fixture['id'] in
                  ('wrapping', 'adjacent', 'bidi-objects', 'unicode-code') else
                  (fixture['width']-1, fixture['width'], fixture['width']+1))
        scan['fixtures'].extend(dict(fixture, id=fixture['id']+'@'+str(w), width=w) for w in widths)
    assert len(scan['fixtures']) == 751
    control = run('control-disabled', scan, False, True)
    disabled = run('patched-disabled', scan, False)
    assert control == disabled, 'metadata-free control differs'
    zero = dict(scan, padding=[0, 0])
    baseline = run('boundary-baseline', zero)
    padded = run('padded-scan', scan)
    assert len(baseline) == len(padded) == len(control)
    assert [r['breakFlags'] for r in baseline] == [r['breakFlags'] for r in control]
    manifests = {f['id']: f for f in scan['fixtures']}
    def validate(baseline, padded, manifests):
        for base, row in zip(baseline, padded):
            def fail(reason, **details):
                failures.append(dict(fixture=row['fixture'], block=row['block'], reason=reason, **details))
            if row['issues']:
                fail('cluster crosses shaping boundary', issues=row['issues'])
            if base['breakFlags'] != row['breakFlags']:
                fail('padding changed Unicode breaks')
            if base['runs'] != row['runs']:
                fail('padding changed shaped runs')
            if row['sourceToBytes'] != base['sourceToBytes'] or row['source'] != base['source']:
                fail('source changed')
            for cell in row['ranges']:
                for box in cell['boxes']:
                    expected = [cell['sourceStart'],cell['sourceEnd']]
                    if box['rtl']:
                        expected.reverse()
                    if box['rect'][2] > box['rect'][0] and box['sourceHits'] != expected:
                        fail('caret/source mismatch', cell=cell, expected=expected)
                    if box['rect'][2] > box['rect'][0]:
                        text=row['source'].encode('utf-16-le')
                        wanted=text[2*cell['sourceStart']:2*cell['sourceEnd']]
                        for anchors in (box['sourceHits'],list(reversed(box['sourceHits']))):
                            a,z=sorted(anchors)
                            if text[2*a:2*z] != wanted:
                                fail('directional copy differs',cell=cell,anchors=anchors)
            for index,line in enumerate(row['lines']):
                slices = [f for f in row['fragments'] if f['line']==index]
                prior = 0.0
                for visual in line['visualRuns']:
                    fragment = next((f for f in slices if f['run']==visual['run']),None)
                    if fragment is None:
                        continue
                    left,top,right,bottom = visual['rect']
                    if abs(left-prior-fragment['before']) > .05:
                        fail('fragment translation mismatch', line=index, visual=visual, fragment=fragment, prior=prior)
                    prior = right+fragment['after']
                if line['visualRuns'] and abs(prior-line['width']) > .05:
                    fail('fragment width mismatch',line=index,actual=prior,expected=line['width'])
            for index, obj in enumerate(row['objects']):
                if not obj['paintContained']:
                    fail('painted object outside paragraph',object=index,bounds=obj['pixelPaintBoundsDpr1'])
                if not obj['contained']:
                    fail('object outside paragraph', object=index, bounds=obj['rect'], height=row['height'])
            for index, line in enumerate(row['lines']):
                if line['overflow'] > .05:
                    # Overflow is allowed only if no smaller indivisible unit can fit.
                    clusters = [c for c in row['clusters'] if c['range'][0] >= line['range'][0]
                                and c['range'][1] <= line['range'][1] and c['range'][0] != c['range'][1]]
                    if len(clusters) > 1:
                        fail('multi-unit line overflows', line=index, geometry=line)
            # Every code glyph cell must lie inside the line's content area with both edges.
            block = manifests[row['fixture']]['blocks'][row['block']]
            for span in block['spans']:
                if span['kind'] != 'code':
                    continue
                for cell in row['ranges']:
                    if not span['start'] <= cell['sourceStart'] < span['start']+span['length']:
                        continue
                    for box in cell['boxes']:
                        x0, y0, x1, y1 = box['rect']
                        line = next((l for l in row['lines'] if l['offset'][1]-.05 <= (y0+y1)/2 <= l['offset'][1]+l['height']+.05), None)
                        if line and row['sourceToBytes'][cell['sourceStart']] >= line['trimmedRange'][1]:
                            continue  # Upstream ghost whitespace is selectable but not painted.
                        if line and (x0 < source['padding'][0]-.05 or x1 > line['width']-source['padding'][0]+.05):
                            fail('code cell lacks outer fragment padding', sourceStart=cell['sourceStart'], box=box, geometry=line)
    validate(baseline,padded,manifests)
    validate(regression_baseline,regression_padded,{f['id']:f for f in regressions['fixtures']})
    invalid = [
        ('combining','e\u0301',0,1,[], 'grapheme-splitting'),
        ('surrogate','🙂',0,1,[], 'grapheme-splitting'),
        ('fractional','ab',0.5,1,[], 'noninteger'),
        ('negative','ab',-1,1,[], 'invalid interval'),
        ('reversed','ab',1,-1,[], 'invalid interval'),
        ('out-of-range','ab',0,3,[], 'invalid interval'),
        ('partial-object','abcd',0,2,[dict(start=1,length=2,width=10,ascent=8,descent=2)], 'partial object'),
    ]
    invalid.append(('malformed-surrogate',chr(0xd800),0,1,[], 'surrogate'))
    for name,text,start,length,objects,message in invalid:
        data=dict(source,fixtures=[case(name,text,[(start,length)],objects=objects)])
        rejected=run('invalid-'+name,data,expected=2)
        assert not rejected and message in (args.output/('invalid-'+name+'.stderr')).read_text()
    data=dict(source,fixtures=[case('overlap','abc',[(0,2),(1,2)])])
    assert not run('invalid-overlap',data,expected=2)
    assert 'overlapping' in (args.output/'invalid-overlap.stderr').read_text()
    # Refine every observed line-break-signature transition to a 1/64-pixel bracket.
    def signature(rows):
        return [[line['range'] for line in row['lines']] for row in rows]
    grouped = {}
    for fixture in scan['fixtures']:
        name,width = fixture['id'].rsplit('@',1)
        grouped.setdefault(name,[]).append((float(width),fixture,
            signature([r for r in padded if r['fixture']==fixture['id']])))
    transitions = []
    for name,items in grouped.items():
        for left,right in zip(items,items[1:]):
            if left[2] != right[2]:
                transitions.append(dict(name=name,low=left[0],high=right[0],
                    lowSignature=left[2],fixture=left[1]))
    for iteration in range(6):
        fixtures = [dict(t['fixture'],id='transition-'+str(i),width=(t['low']+t['high'])/2)
                    for i,t in enumerate(transitions)]
        probes = run('transition-refine-'+str(iteration),dict(source,fixtures=fixtures))
        for i,t in enumerate(transitions):
            current = signature([r for r in probes if r['fixture']=='transition-'+str(i)])
            middle=(t['low']+t['high'])/2
            if current == t['lowSignature']:
                t['low']=middle
            else:
                t['high']=middle
    fixtures=[]
    for i,t in enumerate(transitions):
        assert t['high']-t['low'] <= 1/64
        for label,delta in [('below',-1/64),('at',0),('above',1/64)]:
            fixtures.append(dict(t['fixture'],id='edge-'+str(i)+'-'+label,width=t['high']+delta))
    transition_data=dict(source,fixtures=fixtures)
    edge_base=run('transition-zero',dict(transition_data,padding=[0,0]))
    edge_padded=run('transition-padded',transition_data)
    validate(edge_base,edge_padded,{f['id']:f for f in fixtures})
    (args.output/'transitions.json').write_text(json.dumps(transitions,indent=2)+'\n')
    (args.output/'failures.json').write_text(json.dumps(failures, indent=2)+'\n')
    summary = dict(outcome='blocked' if failures else 'geometry-scan-passed',
                   boundaryMinimal='pass', disabledEquivalence=dict(fixtures=751, blocks=len(control), exactJSON=True),
                   scanBlocks=len(padded), transitionBrackets=len(transitions), transitionProbeBlocks=len(edge_padded),
                   failures=len(failures), commands=commands,
                   inputsBefore=before, inputsAfter={p:digest(Path(p)) for p in before},
                   native='not run: remaining geometry/consistency gates not passed')
    assert summary['inputsBefore'] == summary['inputsAfter']
    summary['outputHashes'] = {p.name:digest(p) for p in args.output.iterdir() if p.is_file()}
    (args.output/'summary.json').write_text(json.dumps(summary, indent=2)+'\n')
    print(json.dumps({k:summary[k] for k in ('outcome','boundaryMinimal','disabledEquivalence','scanBlocks','failures')}))
    return bool(failures)


if __name__ == '__main__':
    raise SystemExit(main())
