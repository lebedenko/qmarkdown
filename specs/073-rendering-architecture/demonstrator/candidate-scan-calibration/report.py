#!/usr/bin/env python3
"""Whole-pair baseline/self analysis; never establishes candidate retention."""
import argparse
import json
from pathlib import Path
import random
from statistics import median
import run

original = run.module('calibration_original_report', run.HERE.parent / 'candidate-scan-continuation/report.py')


def envelope(cell, limit):
    lo, hi = cell['intervals95']['overheadPerParagraphNs']
    return not cell['timerLimited'] and -limit <= lo <= hi <= limit


def summarize(pairs, indices, orders):
    run.require(len(pairs) == 20 and len(orders) == 20, 'twenty pairs required')
    for a, b in pairs:
        run.diagnosis.pair_value(a, b)
    cell = original.summarize(pairs, indices)
    # Preserve the original bootstrap and ratio conventions, name the logical arms.
    for key in list(cell):
        renamed = key.replace('baseline', 'A').replace('retained', 'B')
        if renamed != key:
            cell[renamed] = cell.pop(key)
    if not cell['applicable']:
        cell['descriptive'] = None
        return cell
    cell['intervals95'] = {k.replace('baseline', 'A').replace('retained', 'B'): v
                           for k, v in cell['intervals95'].items()}
    values = [run.diagnosis.pair_value(a, b)[1] for a, b in pairs]
    cell['descriptive'] = run.diagnosis.describe(values)
    cell['order'] = {arm: dict(overheadPerParagraphNs=median([v for v, o in zip(values, orders) if o == arm]),
        armMedianNs={label: median([p[i]['elapsedNs'] for p, o in zip(pairs, orders) if o == arm])
                     for i, label in enumerate(('A', 'B'))}) for arm in ('A', 'B')}
    cell['halves'] = {name: dict(overheadPerParagraphNs=median(values[start:start+10]),
        armMedianNs={label: median([p[i]['elapsedNs'] for p in pairs[start:start+10]])
                     for i, label in enumerate(('A', 'B'))}) for name, start in (('firstTen', 0), ('lastTen', 10))}
    return cell


def report(output):
    records = run.validate(output, complete=True)
    limits = json.loads((output / 'limits.json').read_text())
    cells = []
    observations = []
    for case in run.CASES:
        rng = random.Random(73077)
        indices = [[rng.randrange(20) for _ in range(20)] for _ in range(10000)]
        pairs = []
        orders = []
        for sample in range(20):
            entries = [r for r in records if (r['mode'], r['case'], r['sample']) == ('timing', case, sample)]
            run.require(len(entries) == 2 and {e['label'] for e in entries} == {'A', 'B'}, 'missing/duplicate pair')
            arms = {e['label']: run.continuation.old.strict_rows(output / e['raw']) for e in entries}
            pairs.append((arms['A'], arms['B']))
            orders.append(entries[0]['label'])
            for i, operation in enumerate(original.PHASES):
                a, b = arms['A'][i], arms['B'][i]
                delta, normalized = run.diagnosis.pair_value(a, b)
                observations.append(dict(case=case, sample=sample, operation=operation, first=orders[-1],
                    A=a, B=b, differenceNs=delta, overheadPerParagraphNs=normalized,
                    processes={e['label']: e for e in entries}))
        for i, operation in enumerate(original.PHASES):
            cell = dict(case=case, operation=operation,
                        **summarize([(a[i], b[i]) for a, b in pairs], indices, orders))
            if operation == run.TARGETS[case]:
                cell['referenceLimitPerParagraphNs'] = limits[case]
                cell['resolved'] = envelope(cell, limits[case])
            cells.append(cell)
    resolved = all(c['resolved'] for c in cells if 'resolved' in c)
    recommendation = ('Propose a separately approved path-isolation experiment with baseline, unchanged candidate, '
                      'isolated candidate and contemporaneous calibration controls.' if resolved else
                      'Propose a separately approved measurement-design revision before path isolation or qualification.')
    result = dict(retentionEstablished=False, calibration='resolved-control' if resolved else 'unresolved',
                  recommendation=recommendation, seed=73077, resamples=10000, cells=cells,
                  processCount=len(records), wallSeconds=sum(r['wallSeconds'] for r in records),
                  maximumSampledRssBytes=max(r['peakObservedRssBytes'] for r in records),
                  maximumHighWaterRssBytes=max(x[arm]['highWaterRssBytes'] for x in observations for arm in ('A', 'B')))
    run.write(output / 'observations.json', observations)
    run.write(output / 'summary.json', result)
    lines = ['# Candidate-scan calibration', '', f"Calibration: **{result['calibration']}**. Retention remains **not established**.", '',
             recommendation, '',
             '| Fixture | Operation | Median B − A ns/paragraph | 95% interval | Target envelope ±ns/paragraph | Timer limited |',
             '| --- | --- | ---: | --- | ---: | --- |']
    for c in cells:
        interval = c['intervals95']['overheadPerParagraphNs'] if c['applicable'] else None
        lines.append(f"| {c['case']} | {c['operation']} | {c.get('overheadPerParagraphNs')} | {interval} | {c.get('referenceLimitPerParagraphNs', '—')} | {c['timerLimited']} |")
    lines += ['', 'Twenty adjacent pairs per fixture; 10,000 whole-pair resamples, seed 73077 reset per fixture, shared indices across operations. '
              'Percentile indices are int(p × (N − 1)). Envelopes use exact archived numeric limits with inclusive boundaries. '
              'This diagnostic does not establish equivalence, change historical gates or justify subtracting control estimates from historical timings.', '',
              'Each process retains initial, width+17 reflow, unchanged repeat and metadata. Prose metadata is null. '
              'observations.json retains all raw operation values, duration and RSS; summary.json includes arm medians, unscaled MAD, range, signs, order and early/late summaries. '
              'Ratios are suppressed when either arm median is below 100× the larger clock period/empirical resolution.', '',
              f"82 processes completed; summed supervised duration {result['wallSeconds']:.3f} seconds; maximum sampled RSS {result['maximumSampledRssBytes']} bytes; high-water RSS {result['maximumHighWaterRssBytes']} bytes.", '',
              'Twenty pairs are a fixed diagnostic budget, not a power guarantee. Order and chronology are exploratory. '
              'Actual CPU placement, frequency, thermals and scheduling are unmeasured; caches are uncontrolled. No warm-ups, retries, extra samples, affinity changes or builds.']
    (output / 'report.md').write_text('\n'.join(lines) + '\n')
    return result


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('output', type=Path)
    report(p.parse_args().output.resolve())
