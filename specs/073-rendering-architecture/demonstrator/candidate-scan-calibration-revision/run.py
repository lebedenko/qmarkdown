#!/usr/bin/env python3
"""Fixed four-process-group baseline/self calibration. A durable attempt is never retried."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import platform
import random
import shutil
import sys
import time

HERE = Path(__file__).resolve().parent
FEATURE = HERE.parents[1]
ROOT = FEATURE.parents[1]
HISTORICAL = ROOT / 'build-ci/073-candidate-continuation-evidence'
BINARY = ROOT / 'build-ci/073-candidate-continuation/baseline-build/runtime-trace'
CASES = ('prose-480-4096', 'dense-240-4096')
TARGETS = dict(zip(CASES, ('reflow', 'metadata')))


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


IMPORT_PATH = list(sys.path)
continuation = module('calibration_continuation', HERE.parent / 'candidate-scan-continuation/run.py')
diagnosis = module('calibration_diagnosis', HERE.parent / 'candidate-scan-diagnosis/analyze.py')
sys.path[:] = IMPORT_PATH
recovery = continuation.recovery
supervisor = continuation.supervisor
digest = diagnosis.digest
require = diagnosis.require
write = recovery.durable_json


def schedule():
    rng = random.Random(73078)
    orders = {}
    for case in CASES:
        orders[case] = []
        for _ in range(10):
            block = ['ABBA', 'ABBA', 'BAAB', 'BAAB']
            rng.shuffle(block)
            orders[case].extend(block)
    result = []

    def add(mode, sample, case, label, order):
        pair = f'{mode}-{sample:02}-{case}-zero'
        result.append(dict(id=pair + '-' + label + (f'-{order}' if mode == 'timing' else ''), pair=pair, mode=mode,
                           sample=sample, case=case, variant='zero', label=label, order=order))

    for case in CASES:
        add('correctness', 0, case, 'baseline', 0)
    for sample in range(40):
        cases = list(CASES)
        rng.shuffle(cases)
        for case in cases:
            first = orders[case][sample]
            for order, arm in enumerate(first):
                add('timing', sample, case, arm, order)
    return result


def command(entry):
    # Original immutable manifest paths also make resumed commands byte-identical.
    return list(map(str, (BINARY, HISTORICAL / (entry['case'] + '.json'), 'zero', entry['mode'])))


def environment():
    # Retain inherited values without copying possible credentials into evidence.
    env = dict(os.environ)
    hashes = {k: hashlib.sha256(v.encode()).hexdigest() for k, v in env.items()}
    return dict(kernel=platform.platform(), python=sys.version, executable=sys.executable,
                affinity=sorted(os.sched_getaffinity(0)), load=os.getloadavg(),
                environmentValueSha256=hashes,
                visible={k: v for k, v in env.items() if k in
                         ('PATH', 'LANG', 'LC_ALL', 'LC_NUMERIC', 'LD_LIBRARY_PATH', 'TZ')})


def historical(output):
    identities = {}
    for kind in ('continuation', 'diagnosis', 'calibration'):
        folder = FEATURE / ('evidence/candidate-scan-' + kind)
        target = output / 'inputs' / kind
        members = diagnosis.verify_archives(folder, target)
        for path in folder.iterdir():
            if path.is_file():
                identities[str(path)] = digest(path)
        if kind == 'continuation':
            for name, expected in members.items():
                path = HISTORICAL / name
                diagnosis.check_hash(path, expected)
                identities[str(path)] = expected
            for filename in ('identities.json', 'supplemental-identities.json'):
                pinned = json.loads((target / filename).read_text())
                diagnosis.verify_identities(pinned)
                identities.update(pinned)
        else:
            if kind == 'calibration':
                pinned = json.loads((target / 'identities.json').read_text())
                diagnosis.verify_identities(pinned)
                identities.update(pinned)
            for path in (HERE.parent / ('candidate-scan-' + kind)).glob('*.py'):
                diagnosis.check_hash(path, members[('analysis/' if kind == 'diagnosis' else 'tool/') + path.name])
                identities[str(path)] = digest(path)
    require(str(BINARY) in identities, 'baseline binary not pinned')
    summary = json.loads((output / 'inputs/continuation/summary.json').read_text())
    limits = {}
    for case, operation in TARGETS.items():
        matches = [c for c in summary['cells'] if
                   (c['case'], c['variant'], c['operation']) == (case, 'zero', operation)]
        require(len(matches) == 1, 'missing/duplicate archived target')
        limits[case] = matches[0]['overheadLimitPerParagraphNs']
    # Only a sanity check uses rounded display values; decisions use loaded numbers.
    require([round(limits[c], 3) for c in CASES] == [4819.525, 101542.075], 'unexpected archived limits')
    return identities, limits


def prepare(output):
    require(not output.exists(), 'fresh output required')
    output.mkdir(parents=True)
    identities, limits = historical(output)
    for path in HERE.iterdir():
        if path.is_file():
            identities[str(path)] = digest(path)
    identities[sys.executable] = digest(Path(sys.executable))
    for case in CASES:
        shutil.copyfile(HISTORICAL / (case + '.json'), output / (case + '.json'))
    write(output / 'identities.json', identities)
    write(output / 'limits.json', limits)
    write(output / 'schedule.json', schedule())
    write(output / 'commands.json', [command(e) for e in schedule()])
    write(output / 'environment.json', environment())
    write(output / 'protocol.json', dict(scheduleSeed=73078, bootstrapSeed=73079,
          resamples=10000, processes=322, timeoutSeconds=60, rssLimitBytes=4*1024**3,
          retentionEstablished=False, environment='Inherited unchanged; values hashed to avoid disclosing credentials.'))
    for name in ('index.jsonl', 'attempts.jsonl'):
        (output / name).touch()
    recovery.checkpoint(output, 'prepared', [output / name for name in
        ('identities.json', 'limits.json', 'schedule.json', 'commands.json', 'environment.json', 'protocol.json',
         *(c + '.json' for c in CASES))] +
        [p for p in (output / 'inputs').rglob('*') if p.is_file()])


def validate_output(raw, entry, output):
    # The original validator uses label-dependent filenames. Map both logical arms
    # to the one current baseline correctness reference without altering its checks.
    mapped = dict(entry, label='baseline')
    rows = continuation.validate_output(raw, mapped, output)
    if entry['mode'] == 'correctness':
        reference = output / 'inputs/continuation' / raw.name
        expected = continuation.old.strict_rows(reference)
        require([(r['geometry'], r['shapeDeltas']) for r in rows] ==
                [(r['geometry'], r['shapeDeltas']) for r in expected], 'continuation geometry/shaping mismatch')
    return rows


def journal(path):
    data = path.read_bytes()
    require(not data or data.endswith(b'\n'), 'ambiguous interrupted journal')
    return [json.loads(line) for line in data.splitlines()]


def validate(output, complete=False):
    require(recovery.phase_complete(output, 'prepared'), 'missing preparation checkpoint')
    diagnosis.verify_identities(json.loads((output / 'identities.json').read_text()))
    expected = schedule()
    require(json.loads((output / 'schedule.json').read_text()) == expected, 'schedule mismatch')
    require(json.loads((output / 'commands.json').read_text()) == [command(e) for e in expected], 'command mismatch')
    for cp in output.glob('*-checkpoint.json'):
        recovery.phase_complete(output, cp.name.removesuffix('-checkpoint.json'))
    records = journal(output / 'index.jsonl')
    attempts = journal(output / 'attempts.jsonl')
    require(len(records) == len(attempts) <= 322, 'ambiguous attempted launch')
    require(not complete or len(records) == 322, 'incomplete calibration')
    for i, row in enumerate(records):
        entry = expected[i]
        require(all(row.get(k) == v for k, v in entry.items()), 'invalid completed prefix')
        require(attempts[i] == dict(id=entry['id'], command=command(entry)), 'attempt mismatch')
        require(row['command'] == command(entry) and row['status'] == 'completed' and row['returncode'] == 0,
                'terminal failed execution')
        require(0 <= row['wallSeconds'] <= 60 and 0 <= row['peakObservedRssBytes'] <= 4*1024**3,
                'process bounds violated')
        require(0 < row['parentStartMonotonicNs'] <= row['parentEndMonotonicNs'], 'invalid parent timestamps')
        if i:
            require(records[i-1]['parentEndMonotonicNs'] <= row['parentStartMonotonicNs'], 'overlapping processes')
        require(row['raw'] == entry['id'] + '-sample.jsonl' and
                row['stderr'] == entry['id'] + '-sample.stderr', 'raw identity mismatch')
        for name, key in ((row['raw'], 'rawSha256'), (row['stderr'], 'stderrSha256'),
                          (entry['case'] + '.json', 'fixtureSha256')):
            diagnosis.check_hash(output / name, row[key])
        validate_output(output / row['raw'], entry, output)
    require({p.name for p in output.glob('*-sample.jsonl')} == {r['raw'] for r in records}, 'unacknowledged raw output')
    return records


def resume(prior, output):
    require(not output.exists(), 'resume requires new directory')
    records = validate(prior)
    if len(records) < 322:
        previous = json.loads((prior / 'environment.json').read_text())
        current = environment()
        require(previous['environmentValueSha256'] == current['environmentValueSha256'] and
                previous['affinity'] == current['affinity'], 'resume environment mismatch')
    shutil.copytree(prior, output)
    return records


def execute(output, records):
    env = dict(os.environ)
    for entry in schedule()[len(records):]:
        raw = output / (entry['id'] + '-sample.jsonl')
        err = raw.with_suffix('.stderr')
        cmd = command(entry)
        supervisor.append(output / 'attempts.jsonl', dict(id=entry['id'], command=cmd))
        # An exception/kill after the durable intent leaves an ambiguous attempt,
        # which validation rejects even when Popen never created a raw file.
        started = time.monotonic_ns()
        result = supervisor.supervise(cmd, raw, err, timeout=60, rss_limit=4*1024**3, env=env)
        ended = time.monotonic_ns()
        result.update(parentStartMonotonicNs=started, parentEndMonotonicNs=ended)
        error = None
        if result['status'] == 'completed':
            try:
                validate_output(raw, entry, output)
            except (AssertionError, ValueError, KeyError, TypeError) as ex:
                result['status'] = 'invalid-output'
                error = str(ex)
        row = dict(**entry, **result, raw=raw.name, rawSha256=digest(raw), stderr=err.name,
                   stderrSha256=digest(err), fixtureSha256=digest(output / (entry['case'] + '.json')),
                   validationError=error)
        supervisor.append(output / 'index.jsonl', row)
        require(row['status'] == 'completed', str(row))
        records.append(row)
        print(entry['id'], 'completed', flush=True)
    validate(output, complete=True)
    recovery.checkpoint(output, 'finished', [output / 'index.jsonl', output / 'attempts.jsonl'])


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('output', type=Path)
    p.add_argument('--resume-from', type=Path)
    a = p.parse_args()
    with recovery.execution_lock(ROOT / 'build-ci/073-performance-recovery/execution.lock'):
        output = a.output.resolve()
        if a.resume_from:
            records = resume(a.resume_from.resolve(), output)
        else:
            prepare(output)
            records = []
        execute(output, records)


if __name__ == '__main__':
    main()
