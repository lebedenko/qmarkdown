#!/usr/bin/env python3
"""Whole-group calibration, descriptive ratios, and terminal forensic reporting."""
import json
from pathlib import Path
import random
from statistics import median
import sys
import run as p


def describe(values):
    if not values or all(v is None for v in values): return None
    p.require(all(v is not None for v in values), 'mixed applicability')
    m=median(values)
    return dict(count=len(values), median=m, mad=median([abs(v-m) for v in values]),
                minimum=min(values), maximum=max(values), positive=sum(v>0 for v in values),
                negative=sum(v<0 for v in values), zero=sum(v==0 for v in values))


def indices():
    rng=random.Random(73085)
    return [[rng.randrange(40) for _ in range(40)] for _ in range(10000)]


def interval(values, samples):
    estimates=sorted(median([values[i] for i in sample]) for sample in samples)
    return [estimates[int(.025*(10000-1))], estimates[int(.975*(10000-1))]]


def group(rows, order):
    p.require(len(rows)==4 and order in ('ABBA','BAAB'), 'invalid four-process group')
    n=rows[0]['applicableParagraphs']; p.require(all(r['applicableParagraphs']==n for r in rows), 'unequal paragraph counts')
    v=[r['elapsedNs'] for r in rows]
    p.require(all((x is None)==(n==0) for x in v), 'invalid applicability')
    threshold=100*max(max(r['clockPeriodNs'],r['minimumPositiveDeltaNs']) for r in rows)
    A=sum(x for x,a in zip(v,order) if a=='A')/2 if n else None
    B=sum(x for x,a in zip(v,order) if a=='B')/2 if n else None
    ratio=(B-A)/A if n and min(v)>=threshold else None
    return dict(order=order, rawNs=v, normalizedNs=[x/n if n else None for x in v], paragraphs=n,
                A=A,B=B,differenceNs=B-A if n else None,D=(B-A)/n if n else None,
                relativeDifference=ratio, relativeDifferenceReason=None if ratio is not None else
                ('not applicable' if not n else 'timer resolution suppression'), ratioThresholdNs=threshold)


def envelope(cell, limit):
    if not cell['applicable'] or cell['timerLimited']: return False
    lo,hi=cell['interval95NsPerParagraph']
    return -limit<=lo<=hi<=limit


def decision(cells):
    controls=[c for c in cells if c.get('selfTarget')]
    p.require(len(controls)==6, 'six required self targets')
    if not all(c['envelopeFit'] for c in controls):
        return dict(outcome='unresolved contemporaneous controls',modes=None)
    modes={}
    for mode in ('prepared','original'):
        def target(comparison):
            return next(c for c in cells if c['measurement']==mode and c['comparison']==comparison and c.get('target'))
        residual=target('isolated-minus-baseline');effect=target('isolated-minus-candidate')
        limited=any(target(name)['timerLimited'] for name in ('candidate-minus-baseline','isolated-minus-baseline','isolated-minus-candidate'))
        negative=effect['interval95NsPerParagraph'][1]<0
        modes[mode]=dict(timerLimited=limited,strictlyNegativeIsolationInterval=negative,
                         residualEnvelopeFit=residual['envelopeFit'],diagnosticSupport=not limited and negative and residual['envelopeFit'])
    support=[x['diagnosticSupport'] for x in modes.values()]
    return dict(outcome='diagnostic support in both modes' if all(support) else 'mixed diagnostic support' if any(support) else 'unsupported',modes=modes)


def summarize(groups, samples):
    p.require(len(groups)==40, 'forty groups required')
    def summary(gs):
        ratios=[g['relativeDifference'] for g in gs if g['relativeDifference'] is not None]
        return dict(**{key:describe([g[key] for g in gs]) for key in ('A','B','differenceNs','D')},
                    ratios=dict(available=len(ratios),suppressed=len(gs)-len(ratios),summary=describe(ratios)))
    applicable=groups[0]['D'] is not None
    p.require(all((g['D'] is not None)==applicable for g in groups), 'mixed cell applicability')
    result=dict(applicable=applicable, groups=groups, descriptive=summary(groups),
                rawTimingNs=describe([v for g in groups for v in g['rawNs']]),
                normalizedTimingNs=describe([v for g in groups for v in g['normalizedNs']]),
                order={o:summary([g for g in groups if g['order']==o]) for o in ('ABBA','BAAB')},
                halves={name:summary(groups[start:start+20]) for name,start in (('first20',0),('last20',20))})
    threshold=max(g['ratioThresholdNs'] for g in groups)
    result.update(timerLimited=applicable and min(result['descriptive'][k]['median'] for k in ('A','B'))<threshold,
                  timerThresholdNs=threshold, interval95NsPerParagraph=interval([g['D'] for g in groups],samples) if applicable else None)
    return result


def analyze(records, raw, limits):
    p.require(len(records)==2001 and [r['id'] for r in records]==[e['id'] for e in p.schedule()['commands']], 'complete ordered observations required')
    cells=[]; observations=[]
    for config in p.schedule()['configurations']:
        cid=config['id'];rs=[r for r in records if r['configuration']==cid];samples=indices()
        phases=('prepared-reflow',) if config['batchSize'] else ('initial','reflow','repeat','metadata')
        for phase in phases:
            groups=[]
            for rnd in range(40):
                entries=[r for r in rs if r['mode']=='timing' and r['round']==rnd]
                p.require(len(entries)==4 and [r['positionInGroup'] for r in entries]==list(range(4)), 'missing/duplicate/misordered group')
                rows=[next(x for x in raw[r['id']] if x['phase']==phase) for r in entries]
                g=dict(round=rnd,**group(rows,''.join(r['arm'] for r in entries)));groups.append(g)
                observations.append(dict(configuration=cid,phase=phase,round=rnd,processes=[r['id'] for r in entries],raw=rows,**{k:v for k,v in g.items() if k!='round'}))
            p.require(all(sum(g['order']==o for g in half)==10 for half in (groups[:20],groups[20:]) for o in ('ABBA','BAAB')), 'unbalanced halves')
            target=phase==('prepared-reflow' if config['batchSize'] else 'metadata')
            cell=dict(configuration=cid,measurement=config['measurement'],comparison=config['comparison'],operation=phase,
                      target=target,selfTarget=target and config['comparison'].endswith('-self'),**summarize(groups,samples))
            if target:
                limit=limits[config['case']]
                cell.update(diagnosticToleranceNsPerParagraph=limit,envelopeFit=envelope(cell,limit))
            cells.append(cell)
    resources={}
    for mode in ('functional','correctness','timing'):
        rs=[r for r in records if r['mode']==mode]
        resources[mode]=dict(processes=len(rs),maximumSampledRssBytes=max(r['peakObservedRssBytes'] for r in rs),
            maximumWallSeconds=max(r['wallSeconds'] for r in rs),
            maximumHighWaterRssBytes=max((x['highWaterRssBytes'] for r in rs for x in raw[r['id']] if 'highWaterRssBytes' in x),default=None),
            highWaterCoverage='original-contract outputs only; functional/prepared use parent sampled RSS')
    return dict(complete=True,processCount=2001,retentionEstablished=False,**decision(cells),cells=cells,
        observations=observations,resourcesByStage=resources,
        processes=[dict(id=r['id'],mode=r['mode'],configuration=r['configuration'],implementation=r['implementation'],raw=raw[r['id']],
            normalizedNs=[x['elapsedNs']/x['applicableParagraphs'] if x['applicableParagraphs'] else None for x in raw[r['id']]]) for r in records],
        bootstrap=dict(seed=73085,resamples=10000,percentileIndices=[249,9749],sharedIndicesAcrossOperations=True))


def report(output):
    try:
        records=p.validate(output,True)
        raw={r['id']:([] if r['mode']=='functional' else p.rows(output/(r['id']+'.jsonl'))) for r in records}
        result=analyze(records,raw,json.loads((output/'limits.json').read_text()))
    except Exception as ex:
        result=p.forensic(output);result['analysisError']=type(ex).__name__+': '+str(ex)
    result['recommendation']=('Only a separately approved broader qualification decision may follow.' if result['outcome']=='diagnostic support in both modes'
        else 'Stop this iteration. No further measurements authorized.')
    p.write(output/'analysis.json',result)
    p.write(output/'decision.json',{k:result[k] for k in ('complete','outcome','retentionEstablished','recommendation')})
    lines=['# Prepared-state path-isolation experiment','',f"Outcome: **{result['outcome']}**. Retention remains **not established**.",'',result['recommendation'],'',
        '| Configuration | Operation | Median D ns/paragraph | 95% interval | Diagnostic tolerance ±ns/paragraph | Envelope fit | Timer limited |',
        '| --- | --- | ---: | --- | ---: | --- | --- |']
    for c in result.get('cells',[]):
        d=c['descriptive']['D']
        lines.append(f"| {c['configuration']} | {c['operation']} | {d['median'] if d else None} | {c['interval95NsPerParagraph']} | {c.get('diagnosticToleranceNsPerParagraph')} | {c.get('envelopeFit')} | {c['timerLimited']} |")
    lines+=['','D = ((B1+B2) − (A1+A2))/(2×N); N is 96 for prepared prose and recorded applicability for original dense. Forty whole groups; 10,000 bootstrap draws, seed 73085 reset per configuration, percentile indices 249/9749. Intervals are diagnostic and have no multiple-comparison adjustment.','',
        'All six contemporaneous controls must resolve before isolation interpretation. Per-mode support additionally requires a strictly negative I−C upper bound, inclusive I−B envelope fit and resolved comparison timers. Zero-inclusive intervals do not establish no effect.','',
        'Historical observations are neither pooled nor subtracted. Prose uses a diagnostic tolerance from a different measurement mode; no equivalence or retention decision follows. C−B and dense initial/reflow/repeat are descriptive. Code layout and dispatch also changed; no stack-only causal attribution is possible.','',
        'analysis.json retains raw and normalized timings, arm means, differences, medians, unscaled MAD, ranges, signs, order/half summaries, ratio suppression reasons, nulls and separate stage resource maxima. Sampled RSS does not bound unobserved peaks; snapshots do not establish continuous frequency or exclude migration.']
    if result.get('modes'):lines+=['',json.dumps(result['modes'],sort_keys=True)]
    if not result['complete']:lines+=['',result.get('analysisError','Incomplete; see preserved evidence.')]
    (output/'report.md').write_text('\n'.join(lines)+'\n')
    return result

if __name__=='__main__':report(Path(sys.argv[1]).resolve())
