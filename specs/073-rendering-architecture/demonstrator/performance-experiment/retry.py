#!/usr/bin/env python3
"""Repeat incomplete native processes once, preserving every original attempt."""
import argparse
import json
import os
import recovery
from pathlib import Path
from run import append, digest, supervise, validate_output


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('build', type=Path)
    p.add_argument('source', type=Path)
    p.add_argument('output', type=Path)
    p.add_argument('--cohort', default='retry', help='Explicit environment/attempt cohort label')
    args = p.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    original = json.loads((args.source/'provenance.json').read_text())
    build = json.loads((args.build/'build.json').read_text())
    assert digest(args.source/'index.jsonl') == original['files']['index.jsonl']
    env = os.environ.copy()
    env.pop('WAYLAND_DEBUG', None)
    env['SDL_VIDEODRIVER'] = 'wayland'
    for row in map(json.loads, (args.source/'index.jsonl').read_text().splitlines()):
        # Never repeat a completed process to seek a more favorable pacing result.
        if row['status'] == 'completed':
            continue
        relative = ('upstream' if row['variant'] == 'upstream' else 'patched')+'/perf-native'
        exe = args.build.resolve()/relative
        assert digest(exe) == build['executables'][relative]
        name = row['case']+'.json'
        assert digest(args.source/name) == original['files'][name]
        fixture = args.output/name
        fixture.write_bytes((args.source/name).read_bytes())
        raw = args.output/row['raw']
        command = [exe, fixture, row['variant'], row['api'], row['scenario'], raw]
        result = supervise(command, raw.with_suffix('.stdout'), raw.with_suffix('.stderr'), env=env)
        if result['status'] == 'completed':
            validate_output(raw, 'retry')
        append(args.output/'index.jsonl', dict(**{k:row[k] for k in ('case','variant','api','scenario','repetition','raw')},
            cohort=args.cohort, attempt=row.get('attempt', 0)+1, previousStatus=row['status'], previousEvidence=str(args.source), **result))
    # An empty retry set is still an explicit, reproducible result.
    (args.output/'index.jsonl').touch(exist_ok=True)
    record = dict(build=build, fonts={role:dict(path=info['path'], sha256=digest(Path(info['path']))) for role, info in original['fonts'].items()}, configuration=dict(mode='retry', source=str(args.source), cohort=args.cohort),
                  runnerSha256=digest(Path(__file__)),
                  files={f.name:digest(f) for f in args.output.iterdir() if f.is_file()})
    (args.output/'provenance.json').write_text(json.dumps(record, indent=2)+'\n')


if __name__ == '__main__':
    with recovery.execution_lock(Path(__file__).resolve().parents[4]/'build-ci/073-performance-recovery/execution.lock'):
        main()
