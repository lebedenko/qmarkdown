#!/usr/bin/env python3
"""Whole-four-process-group diagnostic; retention is never established."""
import argparse
import json
from pathlib import Path
import random
from statistics import median
import run

PHASES = ('initial', 'reflow', 'repeat', 'metadata')


def envelope(cell, limit):
    if not cell['applicable'] or cell['timerLimited']:
        return False
    lo, hi = cell['intervals95']['overheadPerParagraphNs']
    return -limit <= lo <= hi <= limit


def group_value(rows, order):
    run.require(order in ('ABBA', 'BAAB') and len(rows) == 4, 'invalid group')
    for row in rows:
        run.diagnosis.pair_value(rows[0], row)
    n = rows[0]['applicableParagraphs']
    if rows[0]['elapsedNs'] is None:
        return dict(A=None, B=None, differenceNs=None, overheadPerParagraphNs=None, improvement=None)
    totals = {arm: sum(r['elapsedNs'] for r, label in zip(rows, order) if label == arm) for arm in 'AB'}
    delta = (totals['B'] - totals['A']) / 2
    return dict(A=totals['A']/2, B=totals['B']/2, differenceNs=delta,
                overheadPerParagraphNs=(totals['B']-totals['A'])/(2*n),
                improvement=(totals['A']-totals['B'])/totals['A'] if min(totals.values()) > 0 else None)


def summarize(groups, indices, orders):
    run.require(len(groups) == len(orders) == 40, 'forty groups required')
    run.require(orders.count('ABBA') == orders.count('BAAB') == 20, 'unbalanced groups')
    values = [group_value(rows, order) for rows, order in zip(groups, orders)]
    applicable = values[0]['A'] is not None
    run.require(all((v['A'] is not None) == applicable for v in values), 'mixed applicability')
    if not applicable:
        return dict(applicable=False, completedGroups=40, timerLimited=False, intervals95=None,
                    descriptive=None, groupDifferences=[None]*40)
    def metrics(vs):
        return dict(AMedianNs=median([v['A'] for v in vs]), BMedianNs=median([v['B'] for v in vs]),
                    medianDifferenceNs=median([v['differenceNs'] for v in vs]),
                    overheadPerParagraphNs=median([v['overheadPerParagraphNs'] for v in vs]),
                    medianImprovement=median([v['improvement'] for v in vs])
                    if all(v['improvement'] is not None for v in vs) else None)
    center = metrics(values)
    period = max(r['clockPeriodNs'] for rows in groups for r in rows)
    resolution = max(r['minimumPositiveDeltaNs'] for rows in groups for r in rows)
    limited = min(center['AMedianNs'], center['BMedianNs']) < 100*max(period, resolution)
    boot = [metrics([values[i] for i in sample]) for sample in indices]
    intervals = {}
    for key in center:
        data = sorted(v[key] for v in boot if v[key] is not None)
        intervals[key] = [data[int(.025*(len(data)-1))], data[int(.975*(len(data)-1))]] if data else None
    if limited:
        center['medianImprovement'] = intervals['medianImprovement'] = None
    def subset(vs):
        return dict(overheadPerParagraphNs=run.diagnosis.describe([v['overheadPerParagraphNs'] for v in vs]),
                    armTimingNs={arm: run.diagnosis.describe([v[arm] for v in vs]) for arm in 'AB'})
    return dict(applicable=True, completedGroups=40, **center, intervals95=intervals,
                timerLimited=limited, clockPeriodNs=period, empiricalResolutionNs=resolution,
                ratioThresholdNs=100*max(period, resolution),
                descriptive=run.diagnosis.describe([v['overheadPerParagraphNs'] for v in values]),
                groupDifferences=[v['overheadPerParagraphNs'] for v in values],
                armGroupTimingsNs={arm: [v[arm] for v in values] for arm in 'AB'},
                order={o: subset([v for v, order in zip(values, orders) if order == o]) for o in ('ABBA', 'BAAB')},
                halves={name: subset(values[start:start+20]) for name, start in (('first20', 0), ('last20', 20))})


def report(output):
    records = run.validate(output, complete=True)
    limits = json.loads((output/'limits.json').read_text())
    cells, observations = [], []
    for case in run.CASES:
        rng = random.Random(73079)
        indices = [[rng.randrange(40) for _ in range(40)] for _ in range(10000)]
        groups, orders = [], []
        for sample in range(40):
            entries = [r for r in records if (r['mode'], r['case'], r['sample']) == ('timing', case, sample)]
            run.require(len(entries) == 4 and [e['order'] for e in entries] == list(range(4)), 'missing/duplicate/misordered group')
            order = ''.join(e['label'] for e in entries)
            rows = [run.continuation.old.strict_rows(output/e['raw']) for e in entries]
            groups.append(rows)
            orders.append(order)
            for i, operation in enumerate(PHASES):
                values = [r[i] for r in rows]
                observations.append(dict(case=case, group=sample, operation=operation, order=order,
                    raw=values, processes=entries, **group_value(values, order)))
        for i, operation in enumerate(PHASES):
            cell = dict(case=case, operation=operation,
                        **summarize([[r[i] for r in rows] for rows in groups], indices, orders))
            if operation == run.TARGETS[case]:
                cell.update(referenceLimitPerParagraphNs=limits[case], resolved=envelope(cell, limits[case]))
            cells.append(cell)
    resolved = all(c['resolved'] for c in cells if 'resolved' in c)
    recommendation = ('Recommend a separately approved path-isolation experiment using this grouping with baseline, '
        'unchanged candidate, isolated candidate and contemporaneous controls.' if resolved else
        'Stop: reconsider the measurement procedure before another separately approved experiment. Do not extend this run.')
    result = dict(retentionEstablished=False, calibration='resolved-control' if resolved else 'unresolved',
                  recommendation=recommendation, seed=73079, resamples=10000, cells=cells,
                  processCount=len(records), wallSeconds=sum(r['wallSeconds'] for r in records),
                  maximumSampledRssBytes=max(r['peakObservedRssBytes'] for r in records),
                  maximumHighWaterRssBytes=max(r['highWaterRssBytes'] for o in observations for r in o['raw']))
    run.write(output/'observations.json', observations)
    run.write(output/'summary.json', result)
    lines = ['# Candidate-scan calibration revision', '',
        f"Calibration: **{result['calibration']}**. Retention remains **not established**.", '', recommendation, '',
        '| Fixture | Operation | Median B − A ns/paragraph | 95% interval | Target envelope ±ns/paragraph | Timer limited |',
        '| --- | --- | ---: | --- | ---: | --- |']
    for c in cells:
        interval = c['intervals95']['overheadPerParagraphNs'] if c['applicable'] else None
        lines.append(f"| {c['case']} | {c['operation']} | {c.get('overheadPerParagraphNs')} | {interval} | {c.get('referenceLimitPerParagraphNs', '—')} | {c['timerLimited']} |")
    lines += ['', 'Forty four-process groups per fixture; D = ((B1 + B2) − (A1 + A2)) / (2 × applicableParagraphs). '
        'Central estimate: median of forty group differences. 10,000 whole-group resamples, seed 73079 reset per fixture, '
        'shared indices across operations. Percentile index int(p × (N − 1)); exact archived inclusive envelopes.', '',
        'Each arm group timing is its two-process mean. Median relative improvement is (A − B) / A across groups; '
        'ratios are suppressed when either median arm timing is below 100× maximum recorded clock period/empirical resolution. '
        'All eight cells, raw observations, group differences, unscaled MAD, range, signs, per-arm timings, '
        'ABBA/BAAB and first-20/last-20 summaries are retained in observations.json and summary.json. Prose metadata stays null.', '',
        f"322 processes completed; summed supervised duration {result['wallSeconds']:.3f} seconds; maximum sampled RSS {result['maximumSampledRssBytes']} bytes; high-water RSS {result['maximumHighWaterRssBytes']} bytes.", '',
        'This estimator differs from the earlier median paired difference. Runs are not pooled; historical gates are unchanged; '
        'no calibration estimate is subtracted and historical variability is not claimed explained. Balanced groups cancel an ideal '
        'linear effect at equally spaced execution positions, not necessarily real elapsed-time drift.', '',
        'Forty groups are a fixed budget, not a power guarantee or proof of statistical independence. CPU placement, frequency, '
        'thermals and scheduling remain unmeasured. No warm-ups, cache flushing, retries, optional samples or environment tuning.']
    (output/'report.md').write_text('\n'.join(lines)+'\n')
    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('output', type=Path)
    report(parser.parse_args().output.resolve())
