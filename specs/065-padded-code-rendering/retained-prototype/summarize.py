#!/usr/bin/env python3
"""Summarize complete diagnostic JSONL without relaxing pixel acceptance."""
import argparse
import json
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('results', type=Path)
args = parser.parse_args()
rows = [json.loads(line) for line in args.results.read_text().splitlines()]
cases = [row for row in rows if row['type'] == 'case']
assert len(cases) == 258 and rows[-1]['cases'] == 258
summary = {'identity': rows[0], 'cases': len(cases), 'comparisons': {},
           'borderInk': {}, 'overlapChanges': 0, 'outsideOverlapChanges': 0,
           'historical': cases[:2]}
for row in cases:
    assert row['occurrences'] == len(row['glyphs']) > 0
    assert all(n == row['occurrences'] for n in row['emissions'].values())
    if not row['id'].startswith('historical'):
        x, y = map(int, row['id'].split('-')[-2:])
        for actual, expected in zip(row['glyphs'][0]['phase'], (x/4, y/4)):
            assert abs(actual - expected) < 1e-10, (row['id'], actual, expected)
    summary['overlapChanges'] += row['directChangesInOverlap']
    summary['outsideOverlapChanges'] += row['directChangesOutsideOverlap']
    for key, count in row['borderInk'].items():
        summary['borderInk'][key] = summary['borderInk'].get(key, 0) + count
    for key, metric in row['metrics'].items():
        entry = summary['comparisons'].setdefault(key, {
            'failedCases': 0, 'changed': 0, 'maxChanged': 0, 'lost': 0,
            'extra': 0, 'maxBGRA': [0]*4})
        entry['failedCases'] += metric['changed'] > 0
        for field in ('changed', 'lost', 'extra'):
            entry[field] += metric[field]
        entry['maxChanged'] = max(entry['maxChanged'], metric['changed'])
        entry['maxBGRA'] = [max(a, b) for a, b in zip(entry['maxBGRA'], metric['maxBGRA'])]
assert sum(v['failedCases'] for v in summary['comparisons'].values()) == rows[-1]['failedComparisons']
print(json.dumps(summary, indent=2))
