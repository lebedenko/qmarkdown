#!/usr/bin/env python3
"""Summarize retained raw runs without converting missing evidence into a pass."""
import argparse
import hashlib
import json
from pathlib import Path
import statistics

p = argparse.ArgumentParser()
p.add_argument('results', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
summary = {'timing': 'capture trace, not display frame rate', 'targets': []}
for kind in ['native', 'software']:
    for qt in ['6.12.0', '6.8.0']:
        directory = a.results / f'{kind}-{qt}'
        target = {'kind': kind, 'qt': qt, 'rawDirectory': str(directory), 'runs': [], 'performance': []}
        runs_file = directory / 'runs.json'
        if not runs_file.exists():
            target['status'] = 'unverified'
            summary['targets'].append(target)
            continue
        runs = json.loads(runs_file.read_text())
        for run in runs:
            log = directory / run['name'] / 'results.jsonl'
            item = dict(run)
            if log.exists():
                rows = [json.loads(line) for line in log.read_text().splitlines() if line.startswith('{')]
                captures = [r for r in rows if r.get('type') == 'capture']
                item.update(traceSha256=hashlib.sha256(log.read_bytes()).hexdigest(),
                            captureCount=len(captures),
                            mutationCount=sum(r.get('type') == 'mutation' for r in rows),
                            motionSummaryCount=sum(r.get('type') == 'motionSummary' for r in rows),
                            allControlsEqual=all(r['controlEqual'] for r in captures),
                            zeroPaints=all(r['paints'] == 0 for r in captures),
                            zeroLayoutRebuilds=all(r['layoutRebuilds'] == 0 for r in captures))
                contrasts = {}
                for fixture in ['plain', 'bold', 'italic', 'link', 'code', 'combined']:
                    values = [r['edgeContrast'] for r in captures if r['fixture'] == fixture and r['id'] in ['origin', '0-0', '0-1', '0-2', '0-3', '1-0', '1-1', '1-2', '1-3', '2-0', '2-1', '2-2', '2-3', '3-0', '3-1', '3-2', '3-3', 'return']]
                    if values:
                        contrasts[fixture] = {'minimum': min(values), 'maximum': max(values)}
                item['fontFiles'] = [json.loads(value) for value in sorted({json.dumps(font, sort_keys=True) for row in rows for font in row.get('fonts', [])})]
                if contrasts:
                    item['stationaryContrast'] = contrasts
            if run['name'].endswith('-geometry'):
                prefix = run['name'][:-len('-geometry')]
                snapshots = {}
                for variant in ['baseline', 'candidate']:
                    log = directory / f'{prefix}-{variant}-static' / 'results.jsonl'
                    fixture_rows = [json.loads(line) for line in log.read_text().splitlines() if line.startswith('{')]
                    snapshots[variant] = {}
                    for row in fixture_rows:
                        if row.get('type') != 'fixture':
                            continue
                        value = {k:row[k] for k in ['text', 'width', 'height', 'cells', 'interactions', 'fonts', 'surfaceCount', 'allocationBytes']}
                        value['fonts'] = sorted({json.dumps(font,sort_keys=True) for font in value['fonts']})
                        snapshots[variant][row['fixture']] = value
                common = set(snapshots['baseline']) & set(snapshots['candidate'])
                item['comparedFixtureCount'] = len(common)
                item['mismatchedFixtureNames'] = sorted(name for name in common if snapshots['baseline'][name] != snapshots['candidate'][name])
                item['completeSnapshotComparison'] = len(snapshots['baseline']) == len(snapshots['candidate']) == 12
            target['runs'].append(item)
        perf_file = directory / 'performance.json'
        if perf_file.exists():
            for group in json.loads(perf_file.read_text()):
                record = {k:v for k,v in group.items() if k != 'records'}
                record['medians'] = {}
                for variant in ['baseline', 'candidate']:
                    rows = [r for r in group['records'] if r['variant'] == variant]
                    record['medians'][variant] = {metric: statistics.median(r[metric] for r in rows) / 1e6 for metric in ['preparationNs', 'medianNs', 'p95Ns']}
                    record['medians'][variant]['imageAllocationBytes'] = sorted({r['allocationBytes'] for r in rows})
                    record['medians'][variant]['surfaces'] = sorted({r['surfaces'] for r in rows})
                target['performance'].append(record)
        expected = 20 if kind == 'native' else 30
        complete = len(runs) == expected and len(target['performance']) == 2
        gates = complete and all(r['valid'] for r in runs)
        for group in target['performance']:
            gates &= all(group[k] for k in ['allocationEqual', 'surfacesEqual', 'zeroPaints', 'fontFilesEqual'])
            gates &= not any(group[m]['repeatableRegression'] for m in ['preparationNs', 'medianNs', 'p95Ns'])
        target['status'] = 'pass' if gates else 'failed' if complete else 'incomplete'
        summary['targets'].append(target)
a.output.write_text(json.dumps(summary, indent=2)+'\n')
print('\n'.join(f"{t['kind']} Qt {t['qt']}: {t['status']}" for t in summary['targets']))
