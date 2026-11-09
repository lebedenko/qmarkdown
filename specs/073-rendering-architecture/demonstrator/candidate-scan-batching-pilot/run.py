#!/usr/bin/env python3
"""One fixed 297-process prepared-state pilot; no retries or implicit extra launches."""
import argparse
from contextlib import contextmanager
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import time

HERE = Path(__file__).resolve().parent
FEATURE = HERE.parents[1]
ROOT = FEATURE.parents[1]
spec = importlib.util.spec_from_file_location('batching_characterization', HERE.parent/'candidate-scan-characterization/analyze.py')
a = importlib.util.module_from_spec(spec)
spec.loader.exec_module(a)
r = a.revision
require, digest, write = a.require, a.digest, r.write
BINARY = ROOT/'build-ci/073-candidate-batching-pilot-build/prepared-reflow'
BUILD = BINARY.parent
SKIA = ROOT/'build-ci/073-candidate-continuation/baseline'
LOCK = ROOT/'build-ci/073-performance-recovery/execution.lock'


def schedule():
    result = a.pilot_schedule()
    result['status'] = 'approved fixed pilot'
    for e in result['commands']:
        if e['argv'][0] == '<draft-prepared-reflow-executable>':
            e['argv'][0] = str(BINARY)
        e['id'] = f"{e['sequence']:03}-{e['configuration']}-{e['mode']}"
    return result


def snapshot():
    start = time.monotonic_ns()
    def read(path):
        try:
            return dict(value=path.read_text().strip(), reason=None)
        except OSError as ex:
            return dict(value=None, reason=f'{type(ex).__name__}: {ex}')
    def files(pattern):
        found = sorted(Path('/sys').glob(pattern))
        return dict(readings={str(p): read(p) for p in found},
                    reason=None if found else 'no exposed sysfs readings')
    try:
        load = dict(value=list(os.getloadavg()), reason=None)
    except OSError as ex:
        load = dict(value=None, reason=str(ex))
    data = dict(startMonotonicNs=start, scope='parent before/after process; exposed sysfs only', load=load,
                frequency=files('devices/system/cpu/cpu[0-9]*/cpufreq/scaling_cur_freq'),
                thermal=files('class/thermal/thermal_zone*/temp'),
                hwmon=files('class/hwmon/hwmon*/temp*_input'),
                boost=files('devices/system/cpu/cpufreq/boost'),
                policy=files('devices/system/cpu/cpufreq/policy*/*governor'))
    data['durationNs'] = time.monotonic_ns()-start
    return data


def environment():
    env=dict(os.environ)
    data=dict(kernel=list(os.uname()),python=sys.version,executable=sys.executable,
              affinity=sorted(os.sched_getaffinity(0)),
              environmentValueSha256={k:hashlib.sha256(v.encode()).hexdigest() for k,v in env.items()},
              visible={k:v for k,v in env.items() if k in ('PATH','LANG','LC_ALL','LC_NUMERIC','LD_LIBRARY_PATH','TZ')})
    data.pop('load', None)
    data.update(machine=platform.machine(), cpuInfoSha256=digest(Path('/proc/cpuinfo')),
                ramBytes=os.sysconf('SC_PAGE_SIZE')*os.sysconf('SC_PHYS_PAGES'),
                topology={str(p): p.read_text().strip() for p in sorted(Path('/sys/devices/system/cpu').glob('cpu[0-9]*/topology/*_siblings_list'))})
    # /proc/cpuinfo contains live MHz; retain stable topology/model instead.
    data.pop('cpuInfoSha256')
    data['cpuModel'] = sorted(set(line for line in Path('/proc/cpuinfo').read_text().splitlines()
                                     if line.startswith(('model name', 'vendor_id', 'cpu family', 'model\t', 'stepping'))))
    return data


def historical(output):
    identities, _ = r.historical(output)
    for kind in ('calibration-revision', 'characterization'):
        folder = FEATURE/('evidence/candidate-scan-'+kind)
        members = a.diagnosis.verify_archives(folder, output/'inputs'/kind)
        for p in folder.iterdir():
            if p.is_file(): identities[str(p)] = digest(p)
        for name, expected in members.items():
            if name.startswith(('tool/', 'analysis/')):
                p = HERE.parent/('candidate-scan-'+kind)/Path(name).name
                a.diagnosis.check_hash(p, expected)
                identities[str(p)] = expected
    for name in ('input-identities.json',):
        pinned = json.loads((output/'inputs/characterization'/name).read_text())
        a.diagnosis.verify_identities(pinned)
        identities.update(pinned)
    require(str(r.BINARY) in identities, 'missing original binary identity')
    # Every linked baseline archive and its defining Ninja recipe must be pinned.
    for p in [SKIA/'out/comparison/obj/skia.ninja', *sorted((SKIA/'out/comparison').glob('*.a'))]:
        require(str(p) in identities, 'unidentified build input: '+str(p))
        a.diagnosis.check_hash(p, identities[str(p)])
    require(list((SKIA/'out/comparison').glob('*.a')), 'missing baseline libraries')
    return identities


def build(output):
    require(not output.exists() and not BUILD.exists(), 'fresh preflight and build directories required')
    output.mkdir(parents=True)
    with a.no_launches():
        identities = historical(output)
    for p in HERE.iterdir():
        if p.is_file(): identities[str(p)] = digest(p)
    write(output/'historical-identities.json', identities)
    commands = [['cmake', '-S', str(HERE), '-B', str(BUILD), '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=Release', '-DSKIA='+str(SKIA)],
                ['cmake', '--build', str(BUILD), '--target', 'prepared-reflow', '--parallel', '2']]
    write(output/'build-commands.json', commands)
    for i, cmd in enumerate(commands):
        with (output/f'build-{i}.log').open('wb') as log:
            result = subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT)
        require(result.returncode == 0, f'build failed; see {output}/build-{i}.log')
    a.diagnosis.verify_identities(identities)
    identities[str(BINARY)] = digest(BINARY)
    for p in (Path('/usr/bin/c++').resolve(), Path(sys.executable).resolve()): identities[str(p)] = digest(p)
    for p in BUILD.rglob('*'):
        if p.is_file() and p.suffix in ('.ninja', '.txt'): identities[str(p)] = digest(p)
    write(output/'identities.json', identities)


def prepare(output, preflight):
    require(not output.exists(), 'fresh output required')
    identities = json.loads((preflight/'identities.json').read_text())
    a.diagnosis.verify_identities(identities)
    output.mkdir(parents=True)
    for path in HERE.iterdir():
        if path.is_file(): identities[str(path)]=digest(path)
    shutil.copytree(preflight, output/'preflight')
    write(output/'identities.json', identities)
    write(output/'schedule.json', schedule())
    write(output/'environment.json', environment())
    write(output/'protocol.json', dict(processes=297, correctness=9, timing=288, seed=73080,
          timeoutSeconds=60, rssLimitBytes=4*1024**3, retentionEstablished=False,
          selection='smallest enabled size: median>=10ms, MAD/median<=0.10, wall<30s, sampled RSS<1GiB'))
    for name in ('index.jsonl', 'attempts.jsonl'): (output/name).touch()
    frozen = {str(p.relative_to(output)): digest(p) for p in output.rglob('*') if p.is_file() and p.name not in ('index.jsonl', 'attempts.jsonl')}
    write(output/'frozen.json', frozen)


def rows(raw):
    return [json.loads(line) for line in raw.read_text().splitlines()]


def validate_raw(raw, entry, output):
    data = rows(raw)
    if entry['configuration'] == 'original-reference':
        mapped = dict(mode=entry['mode'], label='baseline', case='prose-480-4096', variant='zero')
        # Unchanged strict validator reads fixture from output and baseline correctness there.
        expected = a.revision.continuation.old.strict_rows(raw)
        require([x['phase'] for x in expected] == ['initial','reflow','repeat','metadata'], 'original phases')
        fixture = json.loads(Path(entry['argv'][1]).read_text())
        for row in expected:
            r.continuation.validate_lifecycle(row,fixture['fixtures'][0]['blocks'],entry['mode']=='correctness')
            require(row['clockPeriodNs']>0 and row['minimumPositiveDeltaNs']>0 and row['highWaterRssBytes']>0, 'reference timer/resource metadata')
            require(row['fpEnvironment']['rounding']==0 and row['fpEnvironment']['mxcsr'] & 0xe040==0, 'reference floating point environment')
            require(row['cpuAffinity']==json.loads((output/'environment.json').read_text())['affinity'], 'reference affinity')
        require(len(fixture['fixtures'][0]['blocks']) == 1, 'reference fixture')
        for row in expected:
            require(row['shapeDeltas'] == ([0] if row['phase'] != 'initial' else row['shapeDeltas']), 'reference generation')
            require(row['applicableParagraphs'] == (0 if row['phase']=='metadata' else 1), 'reference applicability')
            require((row['elapsedNs'] is None) == (row['phase']=='metadata'), 'reference null')
            if row['elapsedNs'] is not None: require(row['elapsedNs'] > 0, 'reference timer')
        if entry['mode']=='correctness':
            historical = output/'preflight/inputs/continuation'/('correctness-00-prose-480-4096-zero-baseline-sample.jsonl')
            if not historical.exists():
                matches = list((output/'preflight/inputs/continuation').glob('correctness*prose-480-4096*baseline*.jsonl'))
                require(len(matches)==1, 'historical correctness missing'); historical=matches[0]
            old = rows(historical)
            require([(v['geometry'],v['shapeDeltas']) for v in data] == [(v['geometry'],v['shapeDeltas']) for v in old], 'original geometry/generation changed')
        return data
    require(len(data)==1, 'batch row count')
    row=data[0]; config=next(c for c in schedule()['configurations'] if c['id']==entry['configuration'])
    n=config['batchSize']; enabled=config['telemetry']=='enabled'
    require(row['phase']=='prepared-reflow' and row['mode']==entry['mode'] and row['telemetry']==config['telemetry'], 'batch identity')
    require(row['applicableParagraphs']==n and isinstance(row['elapsedNs'],int) and row['elapsedNs']>0, 'batch timing')
    require(row['elapsedPerParagraphNs']==row['elapsedNs']/n, 'batch normalization')
    require(row['distinctInstances'] is True and row['generationUnchanged'] is True and row['sharedFontCollection'] is True and row['finalWidth']==497, 'batch lifecycle')
    require(row['cacheCounts']==[0,1,1], 'batch cache counts')
    require(row['initialWidth']==480 and row['initialGenerations']==row['finalGenerations'] and len(row['initialGenerations'])==n, 'generation evidence')
    require(row['initialStates']==row['finalStates']==[6]*n, 'state evidence')
    require(row['timerEndMonotonicNs']-row['timerStartMonotonicNs']==row['elapsedNs'], 'timer interval')
    for key in ('boundaryBefore','boundaryAfter','boundaryDeltas'):
        require((row[key] is not None)==enabled, 'telemetry switch')
    if enabled:
        require(row['boundaryBefore']['startMonotonicNs']+row['boundaryBefore']['durationNs']<=row['timerStartMonotonicNs'] and row['boundaryAfter']['startMonotonicNs']>=row['timerEndMonotonicNs'], 'telemetry overlaps timer')
        for key in ('boundaryBefore','boundaryAfter'):
            require(row[key]['durationNs']>=0 and row[key]['startMonotonicNs']>0, 'boundary timestamps')
            for metric in ('cpu','processCpuNs','voluntary','involuntary'):
                value=row[key][metric];require((value['value'] is None)==(value['reason'] is not None), 'optional telemetry reason')
    if entry['mode']=='correctness':
        reference=rows(output/(schedule()['commands'][0]['id']+'.jsonl'))[1]['geometry'][0]
        require(len(row['geometry'])==n and all(g==reference for g in row['geometry']), 'reference geometry mismatch')
        require(row['mutationIsolation'] is True, 'mutation isolation')
        require(row['cacheEvents']==[['missingParagraph','addedParagraph']]+[['foundParagraph']]*(n-1), 'cache events')
    for row in data:
        require(row['clockPeriodNs']>0 and row['minimumPositiveDeltaNs']>0, 'timer metadata')
    return data


def validate(output, complete=False):
    frozen=json.loads((output/'frozen.json').read_text())
    for name, expected in frozen.items(): a.diagnosis.check_hash(output/name,expected)
    a.diagnosis.verify_identities(json.loads((output/'identities.json').read_text()))
    require(json.loads((output/'schedule.json').read_text())==schedule(), 'altered schedule')
    records=rows(output/'index.jsonl');attempts=rows(output/'attempts.jsonl');entries=schedule()['commands']
    require(len(records)==len(attempts)<=297, 'ambiguous attempted launch')
    require(not complete or len(records)==297, 'incomplete pilot')
    for i,row in enumerate(records):
        e=entries[i]
        require(all(row.get(k)==v for k,v in e.items()), 'misordered/duplicate record')
        require(attempts[i]==dict(id=e['id'],command=e['argv']), 'attempt identity')
        require(row['status']=='completed' and row['returncode']==0 and row['command']==e['argv'], 'terminal execution failure')
        require(0<row['peakObservedRssBytes']<=4*1024**3 and 0<=row['wallSeconds']<=60, 'resource bounds')
        require(0<row['parentStartMonotonicNs']<=row['parentEndMonotonicNs'], 'parent lifecycle')
        if i: require(records[i-1]['parentEndMonotonicNs']<=row['parentStartMonotonicNs'], 'overlapping processes')
        for suffix,key in (('.jsonl','rawSha256'),('.stderr','stderrSha256')):
            a.diagnosis.check_hash(output/(e['id']+suffix),row[key])
        for key in ('snapshotBefore','snapshotAfter'):
            require(row[key]['durationNs']>=0 and row[key]['startMonotonicNs']>0, 'snapshot lifecycle')
        validate_raw(output/(e['id']+'.jsonl'),e,output)
    require({p.name for p in output.glob('[0-9]*.jsonl')}=={e['id']+'.jsonl' for e in entries[:len(records)]}, 'unacknowledged output')
    return records


def resume(prior, output):
    require(not output.exists(), 'fresh resume directory required')
    records=validate(prior)
    if len(records)<297: require(environment()==json.loads((prior/'environment.json').read_text()), 'resume environment mismatch')
    shutil.copytree(prior,output)
    return records


def execute(output,records):
    require(environment()==json.loads((output/'environment.json').read_text()), 'execution environment mismatch')
    for e in schedule()['commands'][len(records):]:
        before=snapshot()
        r.supervisor.append(output/'attempts.jsonl',dict(id=e['id'],command=e['argv']))
        started=time.monotonic_ns()
        raw=output/(e['id']+'.jsonl');err=output/(e['id']+'.stderr')
        result=r.supervisor.supervise(e['argv'],raw,err,timeout=60,rss_limit=4*1024**3,env=dict(os.environ))
        ended=time.monotonic_ns();after=snapshot()
        error=None
        if result['status']=='completed':
            try: validate_raw(raw,e,output)
            except (AssertionError,ValueError,KeyError,TypeError) as ex: result['status']='invalid-output';error=str(ex)
        row=dict(**e,**result,parentStartMonotonicNs=started,parentEndMonotonicNs=ended,
                 snapshotBefore=before,snapshotAfter=after,rawSha256=digest(raw),stderrSha256=digest(err),validationError=error)
        r.supervisor.append(output/'index.jsonl',row)
        require(row['status']=='completed' and row['peakObservedRssBytes']>0, str(row))
        records.append(row); print(e['id'],'completed',flush=True)
    validate(output,True)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('action',choices=('build','run','resume','validate'))
    p.add_argument('output',type=Path);p.add_argument('--from-directory',type=Path);args=p.parse_args()
    with r.recovery.execution_lock(LOCK):
        out=args.output.resolve();source=args.from_directory.resolve() if args.from_directory else None
        if args.action=='build': build(out)
        elif args.action=='validate': validate(out,True)
        elif args.action=='run': prepare(out,source);execute(out,[])
        else:
            records=resume(source,out)
            if len(records)<297:execute(out,records)

if __name__=='__main__':main()
