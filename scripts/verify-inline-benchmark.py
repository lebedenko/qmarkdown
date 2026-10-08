#!/usr/bin/env python3
"""Validate preparation evidence and equivalence, without timing thresholds."""
import json
import math
from pathlib import Path
import statistics
import sys


def require(condition, message):
    if not condition:
        raise ValueError(message)


def validate(report):
    require(type(report['schema']) is int and report['schema'] == 1
            and report['advisory'] is True, 'Invalid schema')
    require(report['warmups'] == 2 and report['runs'] == 10, 'Invalid run counts')
    for field in ('qt', 'os', 'architecture', 'compiler', 'buildType'):
        require(isinstance(report[field], str) and report[field], 'Missing environment: ' + field)
    expected = {(algorithm, workload, count) for algorithm in ('previous', 'sweep')
                for workload in ('format-only', 'overlapping-links')
                for count in (1024, 2048, 4096, 8192)}
    seen = set()
    outputs = {}
    for entry in report['measurements']:
        key = (entry['algorithm'], entry['workload'], entry['formatSpans'])
        require(key in expected and key not in seen, 'Unexpected/duplicate workload')
        seen.add(key)
        count = entry['formatSpans']
        require(type(count) is int and type(entry['linkSpans']) is int
                and entry['linkSpans'] == (count if key[1] == 'overlapping-links' else 0), 'Invalid spans')
        require(type(entry['textLengthUtf16']) is int and entry['textLengthUtf16'] == count * 4, 'Invalid length')
        require(entry['equivalent'] is True and type(entry['outputIntervals']) is int
                and entry['outputIntervals'] == count, 'Invalid output/equivalence')
        values = entry['durationsMs']
        require(isinstance(values, list) and len(values) == 10
                and all(type(v) in (int, float) and math.isfinite(v) and v >= 0 for v in values), 'Invalid durations')
        require(entry['medianMs'] == statistics.median(values)
                and entry['maximumMs'] == max(values), 'Invalid summaries')
        outputs[key] = entry['outputIntervals']
    require(seen == expected, 'Missing workloads')
    for workload in ('format-only', 'overlapping-links'):
        for count in (1024, 2048, 4096, 8192):
            require(outputs['previous', workload, count] == outputs['sweep', workload, count], 'Unequal output counts')


if __name__ == '__main__':
    try:
        validate(json.loads(Path(sys.argv[1]).read_text()))
    except (ValueError, KeyError, TypeError, IndexError) as error:
        sys.exit(str(error))
    print('PASS: inline preparation benchmark JSON')
