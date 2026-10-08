#!/usr/bin/env python3
"""Validate advisory benchmark evidence, without imposing timing thresholds."""
import json
import math
from pathlib import Path
import statistics
import sys


def require(condition, message):
    if not condition:
        raise ValueError(message)


def validate(report):
    require(type(report['schema']) is int and report['schema'] == 1 and report['advisory'] is True, 'Invalid benchmark schema')
    require(type(report['smoke']) is bool and report['runs'] == 10 and report['warmups'] == 2, 'Invalid run counts')
    require(report['viewportWidth'] == 480 and report['fontPixelSize'] == 16, 'Invalid layout parameters')
    for field in ('font', 'resolvedFont', 'codeFont', 'resolvedCodeFont', 'qt', 'cmark', 'platform', 'os', 'architecture', 'compiler', 'buildType'):
        require(isinstance(report[field], str) and report[field], 'Missing environment: ' + field)
    require(report['font'] == report['resolvedFont'] == 'Noto Sans'
            and report['codeFont'] == report['resolvedCodeFont'] == 'Noto Sans Mono', 'Unexpected font substitution')
    sizes = [1024] if report['smoke'] else [1024, 10240, 102400, 1048576]
    expected = {(operation + '-mixed', size, 0) for size in sizes for operation in ('parse', 'replace', 'layout')}
    expected.update((operation + '-dense', 1024 if report['smoke'] else 10240, 0) for operation in ('parse', 'replace', 'layout'))
    images = [1] if report['smoke'] else [1, 8, 32]
    seen = set()
    image_seen = set()
    for entry in report['measurements']:
        key = (entry['operation'], entry['sourceBytes'], entry['images'])
        require(key not in seen, 'Duplicate measurement')
        seen.add(key)
        require(type(entry['images']) is int and entry['images'] >= 0, 'Invalid image count')
        require(type(entry['sourceBytes']) is int and entry['sourceBytes'] > 0, 'Invalid workload size')
        values = entry['durationsMs']
        require(isinstance(values, list) and len(values) == 10
                and all(type(v) in (int, float) and math.isfinite(v) and v >= 0 for v in values), 'Invalid durations')
        require(entry['medianMs'] == statistics.median(values) and entry['maximumMs'] == max(values), 'Invalid duration summary')
        for field in ('renderedItemCounts', 'modelResetCounts'):
            require(len(entry[field]) == 10 and all(type(v) is int and v >= 0 for v in entry[field]), 'Invalid counts')
        if entry['operation'] == 'sequential-local-images':
            require(entry['images'] in images and entry['images'] not in image_seen, 'Invalid image workload')
            image_seen.add(entry['images'])
            require(entry['modelResetCounts'] == [entry['images'] + 1] * 10, 'Missing image resets')
        else:
            require(key in expected, 'Unexpected measurement')
        if entry['operation'].startswith('layout') or entry['images']:
            require(all(v > 0 for v in entry['renderedItemCounts']), 'Missing rendered items')
        elif entry['operation'].startswith('parse'):
            require(entry['modelResetCounts'] == [0] * 10, 'Parse unexpectedly resets model')
        if entry['operation'].startswith(('replace', 'layout')):
            require(entry['modelResetCounts'] == [1] * 10, 'Missing model replacement')
    require(expected <= seen and image_seen == set(images) and len(seen) == len(expected) + len(images), 'Missing workloads')


if __name__ == '__main__':
    try:
        validate(json.loads(Path(sys.argv[1]).read_text()))
    except (ValueError, KeyError, TypeError, IndexError) as error:
        sys.exit(str(error))
    print('PASS: advisory benchmark JSON')
