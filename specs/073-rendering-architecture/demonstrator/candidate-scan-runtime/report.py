#!/usr/bin/env python3
"""Per-cell whole-process-pair bootstrap and predeclared retention gates."""
import argparse
import json
import hashlib
from pathlib import Path
import random
from statistics import median

SEED=73075
RESAMPLES=10000
PHASES=('initial','reflow','repeat','metadata')


def metrics(pairs):
    a=[x['elapsedNs'] for x,y in pairs];b=[y['elapsedNs'] for x,y in pairs]
    n=[x['applicableParagraphs'] for x,y in pairs]
    return dict(baselineMedianNs=median(a),retainedMedianNs=median(b),
        medianDifferenceNs=median([y-x for x,y in zip(a,b)]),
        medianImprovement=median([(x-y)/x for x,y in zip(a,b)]) if all(x>0 and y>0 for x,y in zip(a,b)) else None,
        baselinePerParagraphNs=median([x/z for x,z in zip(a,n)]),
        retainedPerParagraphNs=median([y/z for y,z in zip(b,n)]),
        overheadPerParagraphNs=median([(y-x)/z for x,y,z in zip(a,b,n)]))


def summarize(pairs, indices):
    if not pairs or pairs[0][0]['elapsedNs'] is None:
        assert all(a['elapsedNs'] is None and b['elapsedNs'] is None for a,b in pairs)
        return dict(applicable=False,completedPairs=len(pairs),timerLimited=False,intervals95=None)
    assert all(a['applicableParagraphs']==b['applicableParagraphs']>0 and a['elapsedNs'] is not None and b['elapsedNs'] is not None for a,b in pairs)
    values=metrics(pairs)
    period=max(max(a['clockPeriodNs'],b['clockPeriodNs']) for a,b in pairs)
    resolution=max(max(a['minimumPositiveDeltaNs'],b['minimumPositiveDeltaNs']) for a,b in pairs)
    limited=min(values['baselineMedianNs'],values['retainedMedianNs'])<100*max(period,resolution)
    boot=[metrics([pairs[i] for i in sample]) for sample in indices]
    intervals={}
    for key in values:
        data=sorted(r[key] for r in boot if r[key] is not None)
        intervals[key]=[data[int(.025*(len(data)-1))],data[int(.975*(len(data)-1))]] if data else None
    if limited:values['medianImprovement']=None;intervals['medianImprovement']=None
    return dict(applicable=True,completedPairs=len(pairs),**values,intervals95=intervals,
                clockPeriodNs=period,empiricalResolutionNs=resolution,ratioThresholdNs=100*max(period,resolution),timerLimited=limited)


def gate(cell):
    if not cell['applicable']:return 'not-applicable'
    if cell['completedPairs']!=20:return 'incomplete'
    primary=cell['variant']=='padded' and cell['case'] in ('long-480-16384','spans-480-32') and cell['operation'] in ('initial','reflow','metadata')
    if primary:
        if cell['timerLimited']:return 'timer-limited-primary'
        return 'pass' if cell['medianImprovement'] is not None and cell['medianImprovement']>=.10 and cell['intervals95']['medianImprovement'][0]>0 else 'primary-fail'
    limit=max(.05*cell['baselinePerParagraphNs'],2000)
    cell['overheadLimitPerParagraphNs']=limit
    return 'pass' if cell['intervals95']['overheadPerParagraphNs'][1]<=limit else 'regression-fail'


def report(root, resamples=RESAMPLES):
    schedule=json.loads((root/'schedule.json').read_text())
    records=[json.loads(line) for line in (root/'index.jsonl').read_text().splitlines()]
    assert len({r['id'] for r in records})==len(records)
    known={e['id']:e for e in schedule}
    for r in records:
        for k,v in known[r['id']].items():assert r[k]==v
    cells=[]
    for case,variant in dict.fromkeys((e['case'],e['variant']) for e in schedule):
        pairs=[];missing=[];rss=[]
        for sample in range(20):
            rows=[r for r in records if (r['mode'],r['sample'],r['case'],r['variant'])==('timing',sample,case,variant)]
            if len(rows)!=2 or any(r['status']!='completed' for r in rows):missing.append(sample);continue
            by={r['label']:[json.loads(line) for line in (root/r['raw']).read_text().splitlines()] for r in rows}
            assert set(by)=={'baseline','retained'}
            pairs.append((by['baseline'],by['retained']))
            rss.extend(dict(label=r['label'],sample=sample,sampledBytes=r['peakObservedRssBytes'],highWaterBytes=max(x['highWaterRssBytes'] for x in by[r['label']])) for r in rows)
        # Identical indices across operations preserve each process pair's relationships.
        rng=random.Random(SEED)
        indices=[[rng.randrange(len(pairs)) for _ in pairs] for _ in range(resamples)] if pairs else []
        for i,phase in enumerate(PHASES):
            cell=dict(case=case,variant=variant,operation=phase,incompleteSamples=missing,
                      processRss=rss,**summarize([(a[i],b[i]) for a,b in pairs],indices))
            cell['gate']=gate(cell) if pairs else 'incomplete'
            cells.append(cell)
    complete=len(records)==1848 and all(r['status']=='completed' for r in records) and [r['id'] for r in records]==[e['id'] for e in schedule]
    correctness=True
    for phase in ('correctness','gates'):
        checkpoint=root/(phase+'-checkpoint.json')
        if not checkpoint.exists():correctness=False;continue
        for name,expected in json.loads(checkpoint.read_text()).items():
            assert hashlib.sha256((root/name).read_bytes()).hexdigest()==expected, name
    if complete:
        assert sum(r['mode']=='correctness' for r in records)==88

    accepted=complete and correctness and all(c['gate'] in ('pass','not-applicable') for c in cells)
    result=dict(seed=SEED,resamples=resamples,completeSampling=complete,correctness=correctness,
        retentionEstablished=accepted,recommendation='Retention established for this study only.' if accepted else 'Retention is not established.',
        difference='retained minus baseline ns',improvement='(baseline - retained) / baseline',
        maximumTimingSampledRssBytes=max((r['sampledBytes'] for c in cells for r in c['processRss']),default=0),
        maximumTimingHighWaterRssBytes=max((r['highWaterBytes'] for c in cells for r in c['processRss']),default=0),
        scratchBound='Verified layout-local Candidate vector reserved to paragraph cluster count; no zero-path scratch. RSS is whole-process, not scratch attribution.',cells=cells)
    (root/'summary.json').write_text(json.dumps(result,indent=2)+'\n')
    lines=['# Candidate-scan runtime study','',result['recommendation'],'',
        'Twenty adjacent fresh-process pairs per cell; descriptive 95% percentile whole-pair intervals, 10,000 resamples, seed 73075. No pooled speedup. Difference is retained minus baseline. Ratios are suppressed below 100 times recorded timer resolution. Absolute regression gates remain active. RSS is whole-process; preprocessing and allocation are included without separate attribution.','',
        '| Fixture | Padding | Operation | Pairs | Baseline ns [95%] | Retained ns [95%] | Difference ns [95%] | Improvement [95%] | Overhead ns/paragraph [95%] | Gate |',
        '| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |']
    for c in cells:
        def fmt(key,ratio=False):
            if not c['applicable']:return 'null'
            if c[key] is None:return 'timer-limited'
            lo,hi=c['intervals95'][key];value=c[key]
            return f'{100*value:.2f}% [{100*lo:.2f}, {100*hi:.2f}]' if ratio else f'{value:,.1f} [{lo:,.1f}, {hi:,.1f}]'
        lines.append(f"| {c['case']} | {c['variant']} | {c['operation']} | {c['completedPairs']} | {fmt('baselineMedianNs')} | {fmt('retainedMedianNs')} | {fmt('medianDifferenceNs')} | {fmt('medianImprovement',True)} | {fmt('overheadPerParagraphNs')} | {c['gate']} |")
    lines+=['','| Small fixture | Padding | Operation | Baseline ns/paragraph [95%] | Retained ns/paragraph [95%] |',
            '| --- | --- | --- | ---: | ---: |']
    for c in cells:
        if not c['case'].startswith('small-'):continue
        def per(key):
            if not c['applicable']:return 'null'
            lo,hi=c['intervals95'][key]
            return f"{c[key]:,.1f} [{lo:,.1f}, {hi:,.1f}]"
        lines.append(f"| {c['case']} | {c['variant']} | {c['operation']} | {per('baselinePerParagraphNs')} | {per('retainedPerParagraphNs')} |")
    lines+=['','Full per-cell timer periods, empirical resolution, baseline/retained per-paragraph intervals and individual process RSS are retained in summary.json. Small fixtures have 32 paragraphs; metadata uses only applicable paragraphs.','',result['scratchBound']]
    (root/'report.md').write_text('\n'.join(lines)+'\n')
    return result

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('evidence',type=Path);report(p.parse_args().evidence)
