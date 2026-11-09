#!/usr/bin/env python3
"""Audit retained CPU geometry for complete vertical and fragment reservations."""
import argparse
import gzip
import hashlib
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('geometry',type=Path)
    parser.add_argument('output',type=Path)
    args = parser.parse_args()
    failures, inputs = [], {}
    cells = fragments = near_edges = 0
    for name in ('padded-scan','regression-padded','transition-padded'):
        manifest = args.geometry/(name+'.json')
        trace = args.geometry/(name+'.jsonl.gz')
        for path in (manifest,trace):
            inputs[str(path)] = hashlib.sha256(path.read_bytes()).hexdigest()
        data = json.loads(manifest.read_text())
        fixtures = {f['id']:f for f in data['fixtures']}
        for row in map(json.loads,gzip.open(trace)):
            block = fixtures[row['fixture']]['blocks'][row['block']]
            for index,line in enumerate(row['lines']):
                slices = [s for s in row['fragments'] if s['line']==index]
                for i,slice_ in enumerate(slices):
                    identity = slice_['identity']
                    span = next((s for s in row['edges'] if s['identity']==identity),None)
                    expected_before = data['padding'][0] if span and (i==0 or slices[i-1]['identity']!=identity) else 0
                    expected_after = data['padding'][0] if span and (i+1==len(slices) or slices[i+1]['identity']!=identity) else 0
                    if (slice_['before'],slice_['after']) != (expected_before,expected_after):
                        failures.append(dict(fixture=row['fixture'],line=index,reason='fragment edge cost differs'))
                    fragments += bool(span and expected_before)
            for cell in row['ranges']:
                for box in cell['boxes']:
                    near_edges += box['nearEdgeSourceHits'] != box['sourceHits']
                if not any(s['kind']=='code' and s['start']<=cell['sourceStart']<s['start']+s['length'] for s in block['spans']):
                    continue
                for box in cell['boxes']:
                    left,top,right,bottom = box['rect']
                    line = next((l for l in row['lines'] if l['offset'][1]-.05 <= (top+bottom)/2 <= l['offset'][1]+l['height']+.05),None)
                    if not line or row['sourceToBytes'][cell['sourceStart']]>=line['trimmedRange'][1]:
                        continue  # Selectable upstream ghost whitespace is not painted.
                    cells += 1
                    pad = data['padding'][1]
                    if top < line['offset'][1]+pad-.05 or bottom > line['offset'][1]+line['height']-pad+.05:
                        failures.append(dict(fixture=row['fixture'],cell=cell,reason='vertical padding missing'))
    result = dict(passed=not failures,failures=failures,verticalCells=cells,visualFragments=fragments,
                  nearEdgeSnapDifferences=near_edges,containmentTolerance=.05,inputs=inputs)
    args.output.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({k:v for k,v in result.items() if k not in ('inputs','failures')}))
    return bool(failures)


if __name__ == '__main__':
    raise SystemExit(main())
