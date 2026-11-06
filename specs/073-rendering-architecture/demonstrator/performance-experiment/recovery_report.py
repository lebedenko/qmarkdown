#!/usr/bin/env python3
"""Audit diagnostic recovery coverage and counters without rerunning measurements."""
import argparse
from collections import Counter
import json
from pathlib import Path
from recovery import rows, durable_json, phase_complete
from workloads import cases


def audit(directory):
    assert phase_complete(directory, 'finished'), 'diagnostic phases incomplete'
    index = rows(directory/'index.jsonl')
    equivalence = {(r['case'], r['variant']):r for r in rows(directory/'equivalence.jsonl')}
    upstream = {r['case']:r for r in rows(directory/'control-equivalence.jsonl')}
    outcomes, counter_checks = [], []
    for case in cases():
        for variant in ('zero', 'padded'):
            key = (case['id'], variant)
            records = [r for r in index if (r['case'], r['variant']) == key]
            if key in equivalence:
                assert equivalence[key]['equal']
                by_label = {r['label']:r for r in records if r['status'] == 'completed'}
                assert set(by_label) == {'headline', 'instrumented'}, key
                data = {label:json.loads((directory/r['raw']).read_text()) for label,r in by_label.items()}
                assert data['instrumented']['geometry'] == data['headline']['geometry'], key
                measured = data['instrumented']
                assert measured['shapingReused'] is True, key
                expected = sum(1+bool(b['spans']) for b in case['blocks'])
                assert measured['counters']['shapeGenerations'] == expected, key
                assert (measured['metadataInvalidationMs'] is not None) == any(b['spans'] for b in case['blocks']), key
                assert measured['counters'] == equivalence[key]['counters'], key
                if variant == 'zero':
                    assert measured['counters']['candidates'] == measured['counters']['fragmentCalculations'] == 0
                counter_checks.append(dict(case=case['id'], variant=variant, expectedShapeGenerations=expected, **measured['counters']))
                status = 'completed'
            else:
                terminal = [r['status'] for r in records if r['status'] != 'completed']
                assert terminal, ('unaccounted case', key)
                assert set(terminal) <= {'timeout', 'rss-limit', 'failed', 'skipped-larger-in-failed-series'}
                status = terminal[-1]
            outcomes.append(dict(case=case['id'], variant=variant, status=status))
        if case['size'] == 1024 or (case['series'].startswith('document-') and case['size'] == 10):
            row = upstream[case['id']]
            assert row['equal'] and row['shapingReused']
            assert 0 < row['counters']['shapeGenerations'] <= len(case['blocks'])
    assert len(outcomes) == 192 and len(upstream) == 18
    geometry = json.loads((directory/'variant-geometry.json').read_text())
    assert all(r['sourceMappingEqual'] for r in geometry)
    return dict(outcomes=outcomes, counts=dict(Counter(r['status'] for r in outcomes)),
                upstreamComparisons=len(upstream), variantMappingComparisons=len(geometry),
                counterChecks=counter_checks, overlapExcluded=True,
                diagnosticDurationsAreHeadlineTimings=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    durable_json(args.output, audit(args.directory))


if __name__ == '__main__':
    main()
