#!/usr/bin/env python3
"""Offline characterization of immutable revision evidence; no process execution."""
import argparse
from contextlib import ExitStack, contextmanager
import csv
import importlib.util
import json
import math
from pathlib import Path
import random
import shutil
from statistics import mean
import sys
from unittest.mock import patch

HERE = Path(__file__).resolve().parent
FEATURE = HERE.parents[1]


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


revision = module('characterization_revision', HERE.parent/'candidate-scan-calibration-revision/run.py')
# Historical report imports its sibling as `run`; keep the binding local to import.
with patch.dict(sys.modules, {'run': revision}):
    report = module('characterization_report', HERE.parent/'candidate-scan-calibration-revision/report.py')
calibration = module('characterization_calibration', HERE.parent/'candidate-scan-calibration/run.py')
diagnosis = revision.diagnosis
require, write, digest = diagnosis.require, diagnosis.write, diagnosis.digest


@contextmanager
def no_launches():
    """Fail loudly even if an imported validator accidentally starts a process."""
    with ExitStack() as stack:
        for target in ('subprocess.Popen', 'os.system', 'os.posix_spawn', 'os.posix_spawnp'):
            stack.enter_context(patch(target, side_effect=AssertionError('forbidden process launch')))
        for supervisor in (revision.supervisor, calibration.supervisor):
            stack.enter_context(patch.object(supervisor, 'supervise', side_effect=AssertionError('forbidden supervisor')))
        yield


def ranks(values):
    order = sorted(range(len(values)), key=values.__getitem__)
    result = [None]*len(values)
    start = 0
    while start < len(order):
        end = start + 1
        while end < len(order) and values[order[end]] == values[order[start]]:
            end += 1
        for index in order[start:end]:
            result[index] = (start + 1 + end)/2
        start = end
    return result


def spearman(x, y):
    require(len(x) == len(y) and len(x) > 0, 'invalid correlation vectors')
    for vector in (x, y):
        require(all(v is None for v in vector) or
                all(v is not None and math.isfinite(v) for v in vector), 'mixed null/nonfinite correlation')
    if all(v is None for v in x) or all(v is None for v in y):
        return None
    a, b = ranks(x), ranks(y)
    ac, bc = mean(a), mean(b)
    variance = sum((v-ac)**2 for v in a)*sum((v-bc)**2 for v in b)
    return sum((v-ac)*(w-bc) for v, w in zip(a, b))/math.sqrt(variance) if variance else None


def describe(values):
    if all(v is None for v in values):
        return None
    require(all(v is not None for v in values), 'mixed applicability')
    return diagnosis.describe(values)


def process_times(record, origin):
    start, end = record['parentStartMonotonicNs'], record['parentEndMonotonicNs']
    require(0 < start <= end, 'invalid parent timestamps')
    return dict(parentStartMonotonicNs=start, parentEndMonotonicNs=end,
                processDurationNs=end-start, processMidpointMonotonicNs=start+(end-start)/2,
                processMidpointElapsedNs=(start-origin)+(end-start)/2)


def group_times(records, origin):
    require(len(records) == 4 and [r['order'] for r in records] == list(range(4)), 'misordered group')
    require(len({r['id'] for r in records}) == 4, 'duplicate process')
    require(len({(r['case'], r['sample'], r['pair'], r['mode']) for r in records}) == 1, 'mixed group identities')
    order = ''.join(r['label'] for r in records)
    require(order in ('ABBA', 'BAAB'), 'invalid arm order')
    times = [process_times(r, origin) for r in records]
    gaps = [b['parentStartMonotonicNs']-a['parentEndMonotonicNs'] for a, b in zip(times, times[1:])]
    require(all(gap >= 0 for gap in gaps), 'overlapping processes')
    midpoints = [t['processMidpointElapsedNs'] for t in times]
    return dict(groupSpanNs=times[-1]['parentEndMonotonicNs']-times[0]['parentStartMonotonicNs'],
                gap1Ns=gaps[0], gap2Ns=gaps[1], gap3Ns=gaps[2],
                groupMidpointElapsedNs=mean(midpoints),
                midpointImbalanceNs=mean([m for m, arm in zip(midpoints, order) if arm == 'B'])-
                                    mean([m for m, arm in zip(midpoints, order) if arm == 'A']))


def matrix(observations, archived_cells):
    all_records = {r['id']: r for o in observations for r in o['processes']}
    records = sorted(all_records.values(), key=lambda r: r['parentStartMonotonicNs'])
    require(len(records) == 320, 'expected 320 timing processes')
    positions = {r['id']: i for i, r in enumerate(records)}
    origin = records[0]['parentStartMonotonicNs']
    processes, groups, cells = [], [], []
    for observation in observations:
        rs, raw = observation['processes'], observation['raw']
        common = {k: observation[k] for k in ('case', 'group', 'operation', 'order')}
        g = dict(**common, **group_times(rs, origin),
                 **report.group_value(raw, observation['order']))
        for key in ('A', 'B', 'differenceNs', 'overheadPerParagraphNs', 'improvement'):
            require(g[key] == observation[key], 'group estimate mismatch')
        groups.append(g)
        for record, row in zip(rs, raw):
            n, ns = row['applicableParagraphs'], row['elapsedNs']
            require((ns is None and n == 0) or (ns is not None and n > 0), 'invalid normalization')
            processes.append(dict(**common, id=record['id'], arm=record['label'],
                positionInGroup=record['order'], chronologicalPosition=positions[record['id']],
                **process_times(record, origin), elapsedNs=ns, applicableParagraphs=n,
                elapsedPerParagraphNs=ns/n if ns is not None else None,
                clockPeriodNs=row['clockPeriodNs'], minimumPositiveDeltaNs=row['minimumPositiveDeltaNs'],
                wallSeconds=record['wallSeconds'], peakObservedRssBytes=record['peakObservedRssBytes']))
    for archived in archived_cells:
        key = archived['case'], archived['operation']
        gs = [g for g in groups if (g['case'], g['operation']) == key]
        ps = [p for p in processes if (p['case'], p['operation']) == key]
        require(len(gs) == 40 and [g['group'] for g in gs] == list(range(40)), 'missing/duplicate/misordered observations')
        require([g['overheadPerParagraphNs'] for g in gs] == archived['groupDifferences'], 'archived differences mismatch')
        def subset(rows):
            return {field: describe([r[field] for r in rows]) for field in
                    ('overheadPerParagraphNs', 'A', 'B', 'groupSpanNs', 'gap1Ns', 'gap2Ns', 'gap3Ns',
                     'groupMidpointElapsedNs', 'midpointImbalanceNs')}
        cells.append(dict(case=key[0], operation=key[1], applicable=archived['applicable'],
            timerLimited=archived['timerLimited'], archivedSummary=archived,
            descriptive=subset(gs),
            correlations=dict(
                operationVsPosition=spearman([p['elapsedNs'] for p in ps], [p['chronologicalPosition'] for p in ps]),
                operationVsProcessMidpoint=spearman([p['elapsedNs'] for p in ps], [p['processMidpointElapsedNs'] for p in ps]),
                differenceVsElapsed=spearman([g['overheadPerParagraphNs'] for g in gs], [g['groupMidpointElapsedNs'] for g in gs]),
                differenceVsImbalance=spearman([g['overheadPerParagraphNs'] for g in gs], [g['midpointImbalanceNs'] for g in gs])),
            order={order: subset([g for g in gs if g['order'] == order]) for order in ('ABBA', 'BAAB')},
            halves={name: subset(gs[start:start+20]) for name, start in (('first20', 0), ('last20', 20))}))
    require(len(cells) == 8 and len({(c['case'], c['operation']) for c in cells}) == 8, 'invalid cell matrix')
    return dict(retentionEstablished=False, timestampScope='parent-observed processes, not operations',
                originParentMonotonicNs=origin, processes=processes, groups=groups, cells=cells)


def pilot_schedule():
    configurations = [dict(id='original-reference', batchSize=None, telemetry='original')]
    configurations += [dict(id=f'batch-{size}-{mode}', batchSize=size, telemetry=mode)
                       for size in (1, 16, 64, 256) for mode in ('disabled', 'enabled')]
    rng, orders = random.Random(73080), {}
    for config in configurations:
        sequence = []
        for _ in range(2):
            block = ['ABBA', 'ABBA', 'BAAB', 'BAAB']
            rng.shuffle(block)
            sequence.extend(block)
        orders[config['id']] = sequence
    commands = []
    def add(config, mode, round_index, position, arm):
        reference = config['id'] == 'original-reference'
        fixture = str(revision.HISTORICAL/'prose-480-4096.json')
        argv = ([str(revision.BINARY), fixture, 'zero', mode] if reference else
                ['<draft-prepared-reflow-executable>', fixture, 'zero', mode,
                 '--batch-size', str(config['batchSize']), '--operation-telemetry', config['telemetry']])
        commands.append(dict(sequence=len(commands), configuration=config['id'], mode=mode,
            round=round_index, positionInGroup=position, arm=arm, argv=argv,
            environment='identical inherited environment within configuration; freeze before execution'))
    for config in configurations:
        add(config, 'correctness', None, None, None)
    for round_index in range(8):
        shuffled = list(configurations)
        rng.shuffle(shuffled)
        for config in shuffled:
            for position, arm in enumerate(orders[config['id']][round_index]):
                add(config, 'timing', round_index, position, arm)
    return dict(status='draft; implementation and execution require separate approval', seed=73080,
                configurations=configurations, commands=commands)


def csv_write(path, rows):
    with path.open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        for row in rows:
            writer.writerow({k: json.dumps(v, sort_keys=True) if isinstance(v, (dict, list)) or v is None else v
                             for k, v in row.items()})


def render(data):
    lines = ['# Workstation revision characterization', '',
        'Retention remains **not established**. Offline analysis only; no new measurements.', '',
        'All eight archived revision estimates, summaries and report bytes reproduced exactly before diagnostics. '
        'The prose reflow target remains unresolved. All 320 timing processes and 40 groups per fixture are retained.', '',
        '| Fixture / operation | Timer limited | ρ timing / position | ρ difference / elapsed | ρ difference / imbalance | First 20 median ns/paragraph | Last 20 median ns/paragraph |',
        '| --- | --- | ---: | ---: | ---: | ---: | ---: |']
    def number(value):
        return 'null' if value is None else f'{value:.6g}'
    for cell in data['cells']:
        cor = cell['correlations']
        halves = [cell['halves'][h]['overheadPerParagraphNs'] for h in ('first20', 'last20')]
        fields = [cor['operationVsPosition'], cor['differenceVsElapsed'], cor['differenceVsImbalance']]
        fields += [h['median'] if h else None for h in halves]
        lines.append(f"| {cell['case']} / {cell['operation']} | {cell['timerLimited']} | " + ' | '.join(map(number, fields))+' |')
    for case in revision.CASES:
        gs = [g for g in data['groups'] if g['case'] == case and g['operation'] == 'reflow']
        span = describe([g['groupSpanNs']/1e6 for g in gs])
        imbalance = describe([g['midpointImbalanceNs']/1e6 for g in gs])
        lines += ['', f"{case}: group span median {span['median']:.3f} ms, range "
                  f"{span['minimum']:.3f}–{span['maximum']:.3f} ms; B−A process midpoint imbalance "
                  f"median {imbalance['median']:.3f} ms, range {imbalance['minimum']:.3f}–{imbalance['maximum']:.3f} ms."]
    lines += ['', 'Observed patterns: operation timings and group differences vary with chronological position as '
        'quantified above; midpoint imbalances are nonzero and unequal spacing occurs. ABBA/BAAB symmetry '
        'in ordinal position therefore does not establish equal cancellation of elapsed-time drift. '
        'The complete JSON/CSV matrix includes every process timing, process duration, group span/gap, '
        'midpoint imbalance, order and half summary. Prose metadata remains null; both repeat cells remain '
        'timer-limited. Their raw absolute timings and descriptive correlations are retained without restoring suppressed ratios.', '',
        'These are parent-observed process timestamps, not operation timestamps. Group elapsed time is the mean '
        'of four process midpoints relative to the first timing-process start; imbalance is mean B minus mean A '
        'midpoint. Operation elapsed time comes from the existing raw timer, with no inferred operation start time. '
        'Process duration includes supervision and setup. CSV null is the literal `null`; nested cell summaries use JSON fields.', '',
        'Spearman correlations are descriptive rank associations (average ranks for ties; null for constant or '
        'inapplicable input), not significance tests or causal estimates. No samples were excluded and no runs pooled. '
        'No new intervals or inferential claims are added. The reproduced historical bootstrap remains historical evidence.', '',
        'Hypotheses requiring future telemetry: CPU placement/migration, scheduling, frequency, thermal conditions '
        'or shared-cache effects might contribute to variation. This archive cannot distinguish them or explain '
        'historical variability. Future boundary telemetry will also have limited sampling scope.', '',
        'Recommendation: review the separate draft batching pilot protocol. It proposes 297 processes on this '
        'same workstation under ordinary shared use, but authorizes no harness or execution. A selected size '
        'would be provisional prepared-state throughput, not calibration or historical qualification.']
    return '\n'.join(lines)+'\n'


def analyze(output):
    require(not output.exists(), 'fresh output required')
    output.mkdir(parents=True)
    with no_launches():
        identities, _ = revision.historical(output)
        source = output/'inputs/revision'
        members = diagnosis.verify_archives(FEATURE/'evidence/candidate-scan-calibration-revision', source)
        for path in revision.HERE.iterdir():
            if path.is_file():
                diagnosis.check_hash(path, members['tool/'+path.name])
        for kind in ('continuation', 'diagnosis', 'calibration', 'calibration-revision'):
            for path in (FEATURE/'evidence'/('candidate-scan-'+kind)).iterdir():
                if path.is_file(): identities[str(path)] = digest(path)
        continuation = output/'inputs/continuation'
        continuation_records = revision.journal(continuation/'index.jsonl')
        diagnosis.verify_records(continuation, revision.continuation.schedule(), continuation_records)
        for row in continuation_records:
            revision.continuation.validate_output(continuation/row['raw'], row, continuation)
        calibration.validate(output/'inputs/calibration', complete=True)
        revision.validate(source, complete=True)
        identities.update(json.loads((source/'identities.json').read_text()))
        diagnosis.verify_identities(identities)
        for name in ('summary.json', 'report.md'):
            require((source/name).read_bytes() == (FEATURE/'evidence/candidate-scan-calibration-revision'/name).read_bytes(), 'published revision mismatch')
        recomputed = output/'recomputed'
        shutil.copytree(source, recomputed)
        summary = report.report(recomputed)
        for name in ('summary.json', 'observations.json', 'report.md'):
            require((recomputed/name).read_bytes() == (source/name).read_bytes(), 'nonidentical revision '+name)
        observations = json.loads((source/'observations.json').read_text())
        data = matrix(observations, summary['cells'])
        write(output/'matrix.json', data)
        for kind in ('processes', 'groups', 'cells'):
            csv_write(output/(kind+'.csv'), data[kind])
        write(output/'pilot-schedule.json', pilot_schedule())
        (output/'report.md').write_text(render(data))
        shutil.copyfile(FEATURE/'batching-pilot-proposal.md', output/'batching-pilot-proposal.md')
        write(output/'input-identities.json', identities)
        write(output/'integrity.json', dict(revisionMembers=members, revisionExact=True,
            continuationRecords=len(continuation_records), calibrationRecords=82, revisionRecords=322,
            fatalLaunchSentinels=True, retentionEstablished=False))
        diagnosis.verify_identities(identities)
    return data


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    analyze(args.output.resolve())
    print('Verified historical inputs and exact revision reproduction; offline matrix and draft schedule written.')
