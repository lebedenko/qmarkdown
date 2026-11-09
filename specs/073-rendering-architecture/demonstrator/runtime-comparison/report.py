#!/usr/bin/env python3
"""Descriptive paired process bootstrap; no performance acceptance threshold."""
import argparse
import json
from pathlib import Path
import random
from statistics import median

SEED = 73073
RESAMPLES = 10000


def summarize(pairs, resamples=RESAMPLES, seed=SEED):
    """Each tuple contains baseline/optimized operation records from one process pair."""
    valid = [(a,b) for a,b in pairs if a['elapsedNs'] is not None and b['elapsedNs'] is not None]
    if not valid:
        return dict(completedPairs=len(pairs), applicablePairs=0, baselineMedianNs=None,
                    optimizedMedianNs=None, medianDifferenceNs=None, medianRatio=None,
                    timerLimited=False, intervals95=None)
    threshold = 100 * max(max(a['clockPeriodNs'],b['clockPeriodNs'],
                              a['minimumPositiveDeltaNs'],b['minimumPositiveDeltaNs']) for a,b in valid)
    def metrics(sample):
        aa = [a['elapsedNs'] for a,b in sample];bb = [b['elapsedNs'] for a,b in sample]
        ratio = median([a/b for a,b in zip(aa,bb)]) if all(a>0 and b>0 for a,b in zip(aa,bb)) else None
        return [median(aa),median(bb),median([b-a for a,b in zip(aa,bb)]),ratio]
    values = metrics(valid)
    limited = min(values[:2]) < threshold
    rng = random.Random(seed)
    samples = [metrics([rng.choice(valid) for _ in valid]) for _ in range(resamples)]
    keys = ['baselineMedianNs','optimizedMedianNs','medianDifferenceNs','medianRatio']
    intervals = {}
    for i,key in enumerate(keys):
        data = sorted(s[i] for s in samples if s[i] is not None)
        intervals[key] = [data[int(.025*(len(data)-1))], data[int(.975*(len(data)-1))]] if data else None
    if limited:values[3] = None;intervals['medianRatio'] = None
    return dict(completedPairs=len(pairs), applicablePairs=len(valid), **dict(zip(keys,values)),
                timerLimited=limited, ratioThresholdNs=threshold, intervals95=intervals)


def report(root):
    records = [json.loads(line) for line in (root/'index.jsonl').read_text().splitlines()]
    entries = [r for r in records if r['mode']=='timing']
    result = []
    schedule = json.loads((root/'schedule.json').read_text())
    cells = list(dict.fromkeys((e['case'],e['variant']) for e in schedule))
    for case,variant in cells:
        pairs = []
        incomplete = []
        for sample in range(10):
            rows = [r for r in entries if (r['case'],r['variant'],r['sample'])==(case,variant,sample)]
            if len(rows) != 2 or any(r['status']!='completed' for r in rows):
                incomplete.append(sample);continue
            by_label = {r['label']:[json.loads(line) for line in (root/r['raw']).read_text().splitlines()] for r in rows}
            pairs.append((by_label['baseline'],by_label['optimized']))
        for i,phase in enumerate(('initial','reflow','repeat','metadata')):
            result.append(dict(case=case,variant=variant,operation=phase,incompleteSamples=incomplete,
                               **summarize([(a[i],b[i]) for a,b in pairs])))
    data = dict(seed=SEED,resamples=RESAMPLES,interval='descriptive paired percentile 95%',
                difference='optimized minus baseline (signed absolute nanoseconds)',
                ratio='baseline / optimized',cells=result,
                processCount=len(entries),completedProcesses=sum(r['status']=='completed' for r in entries),
                maximumSampledRssBytes=max((r.get('peakObservedRssBytes',0) for r in records),default=0),
                maximumHighWaterRssBytes=max((x['highWaterRssBytes'] for r in records if r['status']=='completed'
                                            for x in [json.loads(line) for line in (root/r['raw']).read_text().splitlines()]),default=0))
    (root/'summary.json').write_text(json.dumps(data,indent=2)+'\n')
    lines = ['# Runtime comparison','',
             'Ten fresh-process pairs per cell. No filesystem or independently warmed cache claim.',
             'Difference is optimized minus baseline; ratios are baseline / optimized.',
             'Intervals are descriptive 95% paired-bootstrap intervals (10,000 resamples; seed 73073).',
             'No timer-overhead subtraction. RSS covers the whole process and is not table allocation.',
             '', '| Fixture | Padding | Operation | Pairs | Baseline ns [95%] | Optimized ns [95%] | Difference ns [95%] | Ratio [95%] |',
             '| --- | --- | --- | ---: | ---: | ---: | ---: | ---: |']
    for r in result:
        def fmt(key):
            value=r[key]
            if value is None:return 'timer-limited' if key=='medianRatio' and r['timerLimited'] else 'null'
            interval=r['intervals95'][key]
            if key=='medianRatio':return f'{value:.3f} [{interval[0]:.3f}, {interval[1]:.3f}]'
            return f'{value:,.0f} [{interval[0]:,.0f}, {interval[1]:,.0f}]'
        lines.append(f"| {r['case']} | {r['variant']} | {r['operation']} | {r['completedPairs']} | {fmt('baselineMedianNs')} | {fmt('optimizedMedianNs')} | {fmt('medianDifferenceNs')} | {fmt('medianRatio')} |")
    (root/'report.md').write_text('\n'.join(lines)+'\n')
    return data

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('evidence',type=Path)
    report(parser.parse_args().evidence)
