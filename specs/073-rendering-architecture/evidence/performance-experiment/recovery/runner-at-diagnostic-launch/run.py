#!/usr/bin/env python3
"""Bounded serial measurement runner. Never overwrites an evidence directory."""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import signal
import subprocess
import time
from workloads import cases, manifest, native_cases
import recovery

HERE = Path(__file__).resolve().parent
VARIANTS = ('upstream', 'zero', 'padded')
SCENARIOS = ('stationary', 'slow', 'flick-reversal', 'settling', 'resize', 'selection')


def digest(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def stats(values):
    values = sorted(x for x in values if x is not None and math.isfinite(x))
    if not values:
        return dict(count=0, median=None, p95=None, maximum=None)
    n = len(values)
    return dict(count=n, median=(values[(n-1)//2]+values[n//2])/2,
                p95=values[math.ceil(.95*n)-1], maximum=values[-1])


def supervise(command, out, err, timeout=60, rss_limit=4*1024**3, env=None):
    started = time.monotonic()
    peak, status = 0, 'completed'
    with out.open('wb') as stdout, err.open('wb') as stderr:
        proc = subprocess.Popen(list(map(str, command)), stdout=stdout, stderr=stderr,
                                env=env, start_new_session=True, pass_fds=recovery.LOCK_FDS)
        while proc.poll() is None:
            try:
                values = Path(f'/proc/{proc.pid}/status').read_text().splitlines()
                rss = next(int(line.split()[1])*1024 for line in values if line.startswith('VmRSS:'))
                peak = max(peak, rss)
            except (FileNotFoundError, ProcessLookupError, StopIteration):
                pass
            if peak > rss_limit:
                status = 'rss-limit'
            elif time.monotonic()-started > timeout:
                status = 'timeout'
            if status != 'completed':
                os.killpg(proc.pid, signal.SIGKILL)
                break
            time.sleep(.02)
        code = proc.wait()
        stdout.flush()
        stderr.flush()
        os.fsync(stdout.fileno())
        os.fsync(stderr.fileno())
    if status == 'completed' and code:
        status = 'failed'
    return dict(status=status, returncode=code, peakObservedRssBytes=peak,
                wallSeconds=time.monotonic()-started, command=list(map(str, command)))


def append(path, row):
    with path.open('a') as output:
        output.write(json.dumps(row)+'\n')
        output.flush()
        os.fsync(output.fileno())


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('build', type=Path)
    p.add_argument('output', type=Path)
    p.add_argument('--mode', choices=['cpu', 'native', 'scale', 'captures'], default='cpu')
    p.add_argument('--case', help='optional exact case id for reproduction')
    args = p.parse_args()
    args.build = args.build.resolve()
    args.output.mkdir(parents=True, exist_ok=False)
    build = json.loads((args.build/'build.json').read_text())
    for name, expected in build['executables'].items():
        assert digest(args.build/name) == expected, name
    env = os.environ.copy()
    env.pop('WAYLAND_DEBUG', None)
    env['SDL_VIDEODRIVER'] = 'wayland'
    selected = list(cases() if args.mode == 'cpu' else native_cases())
    if args.case:
        selected = [case for case in selected if case['id'] == args.case]
        assert selected, 'unknown case'
    skipped = set()
    for case in selected:
        data = manifest(case)
        fixture = args.output/(case['id']+'.json')
        fixture.write_text(json.dumps(data, ensure_ascii=False)+'\n')
        for variant in VARIANTS:
            exe = args.build/('upstream' if variant == 'upstream' else 'patched')/('perf-cpu' if args.mode == 'cpu' else 'perf-native')
            if args.mode == 'cpu':
                key = (variant, case['series'])
                if key in skipped:
                    append(args.output/'index.jsonl', dict(case=case['id'], variant=variant, status='skipped-larger-in-failed-series'))
                    continue
                began = time.monotonic()
                for sample, mode in [(i, 'cold') for i in range(10)]+[(0, 'warm')]:
                    name = f"{case['id']}-{variant}-{mode}-{sample}"
                    result = supervise([exe, fixture, variant, mode], args.output/(name+'.jsonl'),
                                       args.output/(name+'.stderr'), timeout=max(.001, 60-(time.monotonic()-began)))
                    append(args.output/'index.jsonl', dict(case=case['id'], series=case['series'], size=case['size'],
                        exactUtf16Units=data['exactUtf16Units'], width=case['width'], variant=variant,
                        mode=mode, sample=sample, raw=name+'.jsonl', **result))
                    if result['status'] != 'completed':
                        skipped.add(key)
                        break
            else:
                scenarios = SCENARIOS if args.mode == 'native' else ('stationary',)
                repetitions = range(3) if args.mode == 'native' else range(1)
                if args.mode == 'scale' and (case['id'] != 'typical' or variant != 'padded'):
                    continue
                for api in ('opengl', 'vulkan'):
                    for scenario in scenarios:
                        for repetition in repetitions:
                            name = f"{case['id']}-{variant}-{api}-{scenario}-{repetition}"
                            output = args.output/(name+'.jsonl')
                            command = [exe, fixture, variant, api, scenario, output]
                            local_env = env.copy()
                            if args.mode == 'scale':
                                command += ['diagnostic']
                                local_env['WAYLAND_DEBUG'] = 'client'
                            elif args.mode == 'captures':
                                command += ['capture']
                            result = supervise(command, args.output/(name+'.stdout'), args.output/(name+'.stderr'), env=local_env)
                            append(args.output/'index.jsonl', dict(case=case['id'], variant=variant, api=api,
                                scenario=scenario, repetition=repetition, raw=name+'.jsonl', **result))
    fonts = manifest(selected[0])['fonts']
    provenance = dict(build=build, fonts={role:dict(path=path, sha256=digest(Path(path))) for role, path in fonts.items()},
                      configuration=vars(args), files={str(f.relative_to(args.output)):digest(f) for f in args.output.iterdir() if f.is_file()})
    provenance['configuration'] = {k:str(v) if isinstance(v, Path) else v for k,v in vars(args).items()}
    (args.output/'provenance.json').write_text(json.dumps(provenance, indent=2)+'\n')


if __name__ == '__main__':
    with recovery.execution_lock(Path(__file__).resolve().parents[4]/'build-ci/073-performance-recovery/execution.lock'):
        main()
