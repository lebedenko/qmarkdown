#!/usr/bin/env python3
"""Aggregate retained samples without converting unavailable metrics to zero."""
import argparse
from collections import defaultdict, Counter
import json
import math
from pathlib import Path
from run import stats, digest

CPU_METRICS = ('constructionMs', 'initialLayoutMs', 'widthReflowMs', 'repeatLayoutMs',
               'metadataInvalidationMs', 'peakRssBytes')
NATIVE_METRICS = ('layoutMs', 'paintMs', 'acquireMs', 'submissionMs', 'swapBlockingMs',
                  'frameThreadWallMs', 'gpuElapsedMs', 'peakRssBytes', 'gpuCacheBytes')


def read_rows(path):
    if not path.exists():
        return []
    rows = []
    for line in path.read_text().splitlines():
        try:
            rows.append(json.loads(line))
        except json.JSONDecodeError:
            # A killed process can leave an incomplete final line; keep earlier samples.
            continue
    return rows


def cpu_summary(directory):
    groups = defaultdict(list)
    for item in read_rows(directory/'index.jsonl'):
        groups[item['case'], item['variant']].append(item)
    result = []
    for (case, variant), entries in groups.items():
        summary = dict(case=case, variant=variant, statusCounts=dict(Counter(e['status'] for e in entries)),
                       series=entries[0].get('series'), size=entries[0].get('size'),
                       exactUtf16Units=entries[0].get('exactUtf16Units'), modes={})
        for mode, expected in [('cold', 10), ('warm', 30)]:
            samples = [row for e in entries if e.get('mode') == mode
                       for row in read_rows(directory/e['raw']) if not row['warmup']]
            missing = [metric for metric in CPU_METRICS if metric != 'metadataInvalidationMs'
                       and stats([row.get(metric) for row in samples])['count'] != len(samples)]
            summary['modes'][mode] = dict(samples=len(samples), expected=expected, missingMetrics=missing,
                complete=not missing and len(samples) == expected and all(e['status'] == 'completed' for e in entries if e.get('mode') == mode),
                metrics={metric:stats([s.get(metric) for s in samples]) for metric in CPU_METRICS})
        result.append(summary)
    return result


def pacing(rows, scale):
    presented = [row for row in rows if row.get('presentation') == 'presented']
    deltas, excess = [], []
    for a, b in zip(presented, presented[1:]):
        delta = b['presentedNs']-a['presentedNs']
        if delta <= 0:
            continue
        deltas.append(delta)
        if b.get('refreshNs'):
            excess.append(delta/b['refreshNs'])
    counts = dict(Counter(row.get('presentation', 'missing') for row in rows))
    precision = scale.get('clockResolutionNs')
    refresh = [r['refreshNs'] for r in presented if r.get('refreshNs')]
    tolerance = 2*precision/min(refresh) if precision is not None and refresh else None
    ratios = stats(excess)
    gaps = []
    if len(deltas) < 2:
        gaps.append('Insufficient actual presentation intervals')
    if counts.get('missing', 0):
        gaps.append('Missing presentation feedback')
    if precision is None:
        gaps.append('Presentation clock resolution unavailable')
    if len(excess) != len(deltas):
        gaps.append('Refresh period unavailable for some intervals')
    return dict(counts=counts, intervalNs=stats(deltas), refreshMultiple=ratios,
        timestampUnitNs=scale.get('timestampUnitNs'), clockResolutionNs=precision,
        advertisedClock=scale.get('presentationClock'), refreshNs=sorted(set(refresh)),
        observedIntervalGcdNs=math.gcd(*deltas) if deltas else None,
        presentationFlagCounts=dict(Counter(r.get('presentationFlags') for r in presented)), gaps=gaps,
        intervalTargetMet=None if gaps or tolerance is None else
            ratios['p95'] <= 1+tolerance and ratios['maximum'] <= 2+tolerance)


def native_summary(directory):
    result = []
    for e in read_rows(directory/'index.jsonl'):
        raw = read_rows(directory/e['raw'])
        scale = raw[0]['data'] if raw and raw[0].get('type') == 'scale' else {}
        samples = [r for r in raw if 'frame' in r and not r['warmup']]
        run = next((r for r in raw if r.get('type') == 'run'), None)
        gaps = []
        if e['status'] != 'completed':
            gaps.append('Process did not complete: '+e['status'])
        if not run or run.get('captureRun') or run.get('protocolDiagnostic') or run.get('warmupSeconds') != 2 or run.get('measurementSeconds') != 5 or run.get('observedScenarioSeconds', 0) < 7:
            gaps.append('Full warmup/measurement window not established')
        if not samples:
            gaps.append('No measured frame records')
        for metric in NATIVE_METRICS:
            if metric != 'gpuElapsedMs' and stats([r.get(metric) for r in samples])['count'] != len(samples):
                gaps.append('Missing metric: '+metric)
        presentation = pacing(samples, scale)
        if gaps:
            presentation['gaps'].extend(gaps)
            presentation['intervalTargetMet'] = None
        result.append(dict(**{k:v for k,v in e.items() if k != 'command'}, scale=scale, measurementGaps=gaps,
            run=run,
            samples=len(samples), metrics={m:stats([r.get(m) for r in samples]) for m in NATIVE_METRICS},
            pacing=presentation, measuredLayoutCalls=sum(r['layoutCalls'] for r in samples),
            gpuTimingGap=('No frame/scale evidence' if not samples else
                'Ganesh does not advertise elapsed GPU timing on this backend' if not scale.get('gpuElapsedSupported') else
                str(sum(r.get('gpuElapsedMs') is None for r in samples))+' missing GPU completion/submission results'
                if any(r.get('gpuElapsedMs') is None for r in samples) else None)))
    return result


def growth(rows, mode="warm"):
    groups = defaultdict(list)
    for r in rows:
        if r.get('series'):
            groups[r['series'], r['variant']].append(r)
    result = []
    for (series, variant), group in groups.items():
        group.sort(key=lambda r:r['size'])
        for a, b in zip(group, group[1:]):
            ratios = {}
            for metric in CPU_METRICS:
                x = a['modes'][mode]['metrics'][metric]['median']
                y = b['modes'][mode]['metrics'][metric]['median']
                ratios[metric] = y/x if x and y is not None else None
            result.append(dict(series=series, variant=variant, mode=mode, fromSize=a['size'], toSize=b['size'],
                               bothComplete=a['modes'][mode]['complete'] and b['modes'][mode]['complete'], ratios=ratios))
    return result


def charts(rows, output):
    import matplotlib
    matplotlib.use('Agg')
    matplotlib.rcParams['svg.hashsalt'] = 'qmarkdown-073-performance'
    import matplotlib.pyplot as plt
    series = sorted({r['series'] for r in rows if r.get('series')})
    colors = dict(upstream='#2166ac', zero='#d99a00', padded='#b2182b')
    plots = [(m, 'warm') for m in ('constructionMs', 'initialLayoutMs', 'widthReflowMs', 'peakRssBytes')]
    plots += [(m, 'cold') for m in ('constructionMs', 'initialLayoutMs')]
    for metric, mode in plots:
        fig, axes = plt.subplots(7, 3, figsize=(13, 21), constrained_layout=True)
        for ax, name in zip(axes.flat, series):
            for variant in colors:
                selected = sorted((r for r in rows if r['series'] == name and r['variant'] == variant
                                   and (mode == 'cold' or r['modes'][mode]['complete'])
                                   and r['modes'][mode]['metrics'][metric]['median'] is not None), key=lambda r:r['size'])
                if selected:
                    x = [r['size'] for r in selected]
                    y = [r['modes'][mode]['metrics'][metric]['median'] for r in selected]
                    ax.plot(x, y, 'o-', color=colors[variant], label=variant, markersize=3)
                    for r, px, py in zip(selected, x, y):
                        if not r['modes'][mode]['complete']:
                            ax.plot(px, py, 'x', color=colors[variant], markersize=8)
                            ax.annotate('n='+str(r['modes'][mode]['samples']), (px, py), fontsize=6)
            ax.set_title(name, fontsize=9)
            ax.set_xscale('log', base=2)
            ax.set_yscale('log')
            ax.grid(True, alpha=.2)
            ax.set_ylabel('bytes' if metric.endswith('Bytes') else 'ms')
            ax.set_xlabel('span/paragraph count' if name.startswith(('spans-', 'document-')) else 'UTF-16 units')
        axes.flat[0].legend(fontsize=8)
        note = 'complete warm batches only' if mode == 'warm' else 'fresh processes; crosses and n label incomplete batches'
        fig.suptitle(metric+' — '+note+'; absent points are not zero')
        name = metric+('-cold' if mode == 'cold' else '')
        fig.savefig(output/(name+'.svg'), metadata={'Date': None})
        fig.savefig(output/(name+'.png'), dpi=110)
        plt.close(fig)



def native_table(rows, output):
    def value(x):
        return 'unavailable' if x is None else f'{x:.6f}'
    lines = ['# Native scenarios, each process repetition', '',
        'Timings are milliseconds. Pacing is evaluated using unrounded timestamp values; GPU gaps do not pass qualification.', '',
        '| Workload | Variant | API | Scenario | Repetition | Attempt | Cohort | Frames | Paint median | GPU median | Presentation p95 | Presentation max | Discarded | Missing | Interval target |',
        '| --- | --- | --- | --- | ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |']
    for r in rows:
        pacing = r['pacing']
        p95, maximum = pacing['intervalNs']['p95'], pacing['intervalNs']['maximum']
        target = 'gap' if pacing['intervalTargetMet'] is None else 'pass' if pacing['intervalTargetMet'] else 'fail'
        fields = [r['case'], r['variant'], r['api'], r['scenario'], str(r['repetition']), str(r.get('attempt', 0)), r.get('cohort', 'original'), str(r['samples']),
            value(r['metrics']['paintMs']['median']), value(r['metrics']['gpuElapsedMs']['median']),
            value(None if p95 is None else p95/1000000), value(None if maximum is None else maximum/1000000),
            str(pacing['counts'].get('discarded', 0)), str(pacing['counts'].get('missing', 0)), target]
        lines.append('| '+' | '.join(fields)+' |')
    (output/'native-scenarios.md').write_text('\n'.join(lines)+'\n')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('cpu', type=Path)
    p.add_argument('native', type=Path)
    p.add_argument('output', type=Path)
    p.add_argument('--charts', action='store_true')
    p.add_argument('--native-supplement', type=Path, action='append', default=[])
    p.add_argument('--allow-partial', action='store_true', help='allow a runner interrupted before its provenance record')
    args = p.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    for directory in (args.cpu, args.native, *args.native_supplement):
        provenance = directory/'provenance.json'
        if args.allow_partial and not provenance.exists():
            continue
        for name, expected in json.loads(provenance.read_text())['files'].items():
            assert digest(directory/name) == expected, 'evidence hash changed: '+str(directory/name)
    cpu = cpu_summary(args.cpu)
    native = native_summary(args.native)
    for directory in args.native_supplement:
        native.extend(native_summary(directory))
    for name, data in [('cpu-summary', cpu), ('native-summary', native), ('growth', growth(cpu)+growth(cpu, 'cold'))]:
        (args.output/(name+'.json')).write_text(json.dumps(data, indent=2)+'\n')
    native_table(native, args.output)
    if args.charts:
        charts(cpu, args.output)


if __name__ == '__main__':
    main()
