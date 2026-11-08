#!/usr/bin/env python3
"""Deterministic descriptive pilot analysis; no calibration or significance claims."""
import json
from pathlib import Path
from statistics import mean, median
import sys
import run as p


def describe(values):
    if all(v is None for v in values): return None
    p.require(values and all(v is not None for v in values), 'mixed/missing values')
    m=median(values)
    return dict(count=len(values),median=m,mad=median([abs(v-m) for v in values]),minimum=min(values),maximum=max(values),
                positive=sum(v>0 for v in values),negative=sum(v<0 for v in values),zero=sum(v==0 for v in values))


def ratios(values):
    available=[v for v in values if v is not None]
    return dict(available=len(available),suppressed=len(values)-len(available),
                summary=describe(available) if available else None)


def qualified(values,resolution):
    return all(v is not None and v>=100*resolution for v in values)


def select(cells, complete):
    if not complete: return None
    for n in (96,128,192):
        c=cells[f'batch-{n}-enabled'];d=c['batchTiming']
        if d['count']==32 and d['median']>=10000000 and d['mad']/d['median']<=.10 and c['maximumParentDurationNs']<30000000000 and c['maximumSampledRssBytes']<1073741824:
            return n
    return None


def analyze(records,raw,complete):
    configurations=p.schedule()['configurations'];cells={};groups=[];processes=[]
    for config in configurations:
        cid=config['id'];rs=[r for r in records if r['configuration']==cid]
        timing=[r for r in rs if r['mode']=='timing'];original=cid=='original-reference'
        phases=('initial','reflow','repeat','metadata') if original else ('prepared-reflow',)
        cell=dict(configuration=cid,maximumParentDurationNs=max((r['parentEndMonotonicNs']-r['parentStartMonotonicNs'] for r in rs),default=None),
                  maximumSampledRssBytes=max((r['peakObservedRssBytes'] for r in rs),default=None),operations={})
        if not rs: cells[cid]=cell;continue
        for phase in phases:
            gs=[];values=[];normalized=[]
            for r in timing:
                row=next(x for x in raw[r['id']] if x['phase']==phase)
                n=row['applicableParagraphs'];ns=row['elapsedNs'];values.append(ns);normalized.append(ns/n if n else None)
                processes.append(dict(id=r['id'],configuration=cid,round=r['round'],arm=r['arm'],phase=phase,elapsedNs=ns,paragraphs=n,nsPerParagraph=ns/n if n else None,
                                      clockPeriodNs=row['clockPeriodNs'],minimumPositiveDeltaNs=row['minimumPositiveDeltaNs']))
            for round_index in range(8):
                group=[r for r in timing if r['round']==round_index]
                if not group: continue
                p.require(len(group)==4 and [r['positionInGroup'] for r in group]==list(range(4)), 'incomplete/misordered group')
                rows=[next(x for x in raw[r['id']] if x['phase']==phase) for r in group]
                n=rows[0]['applicableParagraphs'];v=[x['elapsedNs'] for x in rows];order=''.join(r['arm'] for r in group)
                A=mean([v[i] for i in range(4) if order[i]=='A']) if n else None
                B=mean([v[i] for i in range(4) if order[i]=='B']) if n else None
                resolution=max(max(x['clockPeriodNs'],x['minimumPositiveDeltaNs']) for x in rows)
                g=dict(configuration=cid,phase=phase,round=round_index,order=order,rawNs=v,paragraphs=n,A=A,B=B,
                       differenceNs=B-A if n else None,D=(B-A)/n if n else None,meanNs=mean(v) if n else None,
                       meanNsPerParagraph=mean(v)/n if n else None,relativeDifference=(B-A)/A if n and qualified(v,resolution) else None,timerResolutionNs=resolution)
                g['relativeDifferenceReason']=None if g['relativeDifference'] is not None else ('not applicable' if not n else 'timer resolution suppression')
                groups.append(g);gs.append(g)
            def summary(part): return {k:(ratios if k=='relativeDifference' else describe)([g[k] for g in part]) for k in ('A','B','differenceNs','D','meanNs','meanNsPerParagraph','relativeDifference')} if part else None
            cell['operations'][phase]=dict(rawNs=values,normalizedNs=normalized,timing=describe(values) if values else None,normalized=describe(normalized) if normalized else None,
                 groups=summary(gs),order={o:summary([g for g in gs if g['order']==o]) for o in ('ABBA','BAAB')},
                 halves={label:summary([g for g in gs if lo<=g['round']<hi]) for label,lo,hi in (('firstFour',0,4),('lastFour',4,8))})
        if not original: cell['batchTiming']=cell['operations']['prepared-reflow']['timing']
        cell['resourcesByMode']={mode:dict(maximumParentDurationNs=max((r['parentEndMonotonicNs']-r['parentStartMonotonicNs'] for r in rs if r['mode']==mode),default=None),maximumSampledRssBytes=max((r['peakObservedRssBytes'] for r in rs if r['mode']==mode),default=None)) for mode in ('correctness','timing')}
        cell['parentDurationNs']=describe([r['parentEndMonotonicNs']-r['parentStartMonotonicNs'] for r in rs])
        cell['sampledRssBytes']=describe([r['peakObservedRssBytes'] for r in rs])
        cell['parentSnapshotCollectionDurationNs']=describe([r[k]['durationNs'] for r in rs for k in ('snapshotBefore','snapshotAfter')]) if all('snapshotBefore' in r for r in rs) else None
        if config['telemetry']=='enabled':
            cell['telemetryDeltas']={metric:describe([raw[r['id']][0].get('boundaryDeltas',{}).get(metric) for r in rs if raw[r['id']][0].get('boundaryDeltas',{}).get(metric) is not None]) if any(raw[r['id']][0].get('boundaryDeltas',{}).get(metric) is not None for r in rs) else None for metric in ('processCpuNs','voluntary','involuntary')}
        cell['boundaryCollectionDurationNs']=describe([raw[r['id']][0][key]['durationNs'] for r in rs for key in ('boundaryBefore','boundaryAfter') if raw[r['id']][0].get(key) is not None]) if config['telemetry']=='enabled' else None
        cells[cid]=cell
    comparisons=[]
    for n in (96,128,192):
        matched=[]
        for round_index in range(8):
            pairs=[[g for g in groups if g['configuration']==f'batch-{n}-{mode}' and g['round']==round_index] for mode in ('disabled','enabled')]
            if not all(pairs): continue
            p.require(all(len(v)==1 for v in pairs),'duplicate matched round')
            d,e=[v[0] for v in pairs];difference=e['meanNs']-d['meanNs'];resolution=max(d['timerResolutionNs'],e['timerResolutionNs'])
            matched.append(dict(round=round_index,disabledMeanNs=d['meanNs'],enabledMeanNs=e['meanNs'],differenceNs=difference,differenceNsPerParagraph=difference/n,
                                relativeDifference=difference/d['meanNs'] if qualified(d['rawNs']+e['rawNs'],resolution) else None))
        comparisons.append(dict(batchSize=n,rounds=matched,summary={k:(ratios if k=='relativeDifference' else describe)([r[k] for r in matched]) for k in ('differenceNs','differenceNsPerParagraph','relativeDifference')} if matched else None))
    for comparison in comparisons:
        for row in comparison['rounds']: row['relativeDifferenceReason']=None if row['relativeDifference'] is not None else 'timer resolution suppression'
    selection=select(cells,complete)
    return dict(retentionEstablished=False,complete=complete,processCount=len(records),selection=selection,
                interpretation='provisional prepared-state throughput' if selection else 'no size selected',
                processes=processes,groups=groups,cells=cells,telemetryComparisons=comparisons)


def report(output):
    try: records=p.validate(output)
    except Exception:
        result=p.forensic(output)
        p.write(output/'analysis.json',result);p.write(output/'selection.json',dict(complete=False,selection=None,retentionEstablished=False))
        (output/'report.md').write_text('# Incomplete refinement pilot\n\nNo selection. Retention remains not established. See analysis.json for validation errors and preserved attempts.\n')
        return result
    complete=len(records)==231
    raw={r['id']:p.rows(output/(r['id']+'.jsonl')) for r in records}
    result=analyze(records,raw,complete)
    p.write(output/'analysis.json',result)
    p.write(output/'selection.json',{k:result[k] for k in ('retentionEstablished','complete','processCount','selection','interpretation')})
    text=['# Fixed batching pilot','',f"Completed {len(records)}/231 processes. Selection: {result['selection']} ({result['interpretation']}).",'',
          'Retention remains **not established**. These descriptive observations cannot qualify historical results or supply a later calibration dataset.', '',
          'Raw timing, normalized timing, all group/arm estimates, order/half summaries and matched-round telemetry comparisons are in analysis.json. Correctness and original-reference records remain distinct.', '',
          '| Configuration | Median batch ns | MAD ns | Maximum parent ns | Maximum sampled RSS bytes |','| --- | ---: | ---: | ---: | ---: |']
    for cid,c in result['cells'].items():
        if cid=='original-reference':
            for phase,operation in c['operations'].items():
                d=operation['timing'] or {};text.append(f"| {cid} / {phase} | {d.get('median')} | {d.get('mad')} | {c['maximumParentDurationNs']} | {c['maximumSampledRssBytes']} |")
            continue
        d=c.get('batchTiming') or {};text.append(f"| {cid} | {d.get('median')} | {d.get('mad')} | {c['maximumParentDurationNs']} | {c['maximumSampledRssBytes']} |")
    text+=['','Sampled RSS cannot establish unobserved peaks. Parent snapshots and operation-boundary observations do not establish frequency throughout the loop or rule out migration. Matched configurations are not simultaneous. Instrumentation durations are never subtracted.']
    (output/'report.md').write_text('\n'.join(text)+'\n')
    return result

if __name__=='__main__':report(Path(sys.argv[1]).resolve())
