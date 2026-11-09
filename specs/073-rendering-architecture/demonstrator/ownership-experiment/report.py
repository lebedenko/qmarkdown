#!/usr/bin/env python3
"""Verify retained work counts and report ownership searches, not speedup."""
import json
from pathlib import Path
import sys
from verify import compare, validate_baseline, digest


def rows(path):
    return [json.loads(line) for line in path.read_text().splitlines() if line.strip()]


def report(output, baseline):
    validate_baseline(baseline)
    for name, expected in json.loads((output/'identities.json').read_text()).items():
        assert digest(name) == expected, name
    records = rows(output/'index.jsonl')
    assert len(records) == 48 and all(r['status'] == 'completed' for r in records)
    result = []
    for record in records:
        assert digest(output/record['raw']) == record['rawSha256']
        assert digest(output/(record['case']+'.json')) == record['fixtureSha256']
        actual = rows(output/record['raw']); prior = rows(baseline/record['raw'])
        compare(actual, prior, record['label'])
        if record['label'] != 'instrumented':
            continue
        peer = rows(output/record['raw'].replace('-instrumented-', '-headline-'))
        assert [(r['geometry'], r['shapeDeltas']) for r in actual] == [(r['geometry'], r['shapeDeltas']) for r in peer]
        capacity = None
        for row, old in zip(actual, prior):
            work = row['counters']
            cap = [w['ownerCapacity'] for w in work]
            if row['phase'] in ('repeat', 'reflow'):
                assert cap == capacity, 'reflow storage grew'
            capacity = cap
            old_search = sum(w[b]['ownerComparisons'] for w in old['counters'] for b in ('fitting','formatting'))
            fallback = sum(w[b]['ownerComparisons'] for w in work for b in ('fitting','formatting'))
            build = sum(w['ownerExaminations'] for w in work)
            # No improvement requirement for cases without ownership work.
            result.append(dict(case=record['case'],variant=record['variant'],phase=row['phase'],
                legacyComparisons=old_search,buildExaminations=build,fallbackComparisons=fallback,
                tableLookups=sum(w['ownerLookups'] for w in work),builds=sum(w['ownerBuilds'] for w in work),
                capacityEntries=sum(cap),payloadBytes=sum(w['ownerPayloadBytes'] for w in work),
                candidates=sum(w['candidates'] for w in work),
                runVisits=sum(w[b]['runVisits'] for w in work for b in ('fitting','formatting'))))
    return dict(processes=len(records),phaseRecords=4*len(records),
                maxRssBytes=max(r['peakObservedRssBytes'] for r in records),work=result)


if __name__ == '__main__':
    output=Path(sys.argv[1]); baseline=Path(sys.argv[2])
    print(json.dumps(report(output, baseline),indent=2))
