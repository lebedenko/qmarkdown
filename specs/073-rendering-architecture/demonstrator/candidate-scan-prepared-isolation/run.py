#!/usr/bin/env python3
"""One fixed 2,001-process path-isolation experiment; no retries or implicit extra launches."""
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
import random

HERE = Path(__file__).resolve().parent
FEATURE = HERE.parents[1]
ROOT = FEATURE.parents[1]
spec = importlib.util.spec_from_file_location('batching_characterization', HERE.parent/'candidate-scan-characterization/analyze.py')
a = importlib.util.module_from_spec(spec)
spec.loader.exec_module(a)
r = a.revision
require, digest, write = a.require, a.digest, r.write
WORK = ROOT/'build-ci/candidate-scan-prepared-isolation'
BINARY = ROOT/'build-ci/073-candidate-batching-refinement-build/prepared-reflow'
BUILD = BINARY.parent
SKIA = ROOT/'build-ci/073-candidate-continuation/baseline'
LOCK = ROOT/'build-ci/073-performance-recovery/execution.lock'
ORIGINAL = dict(B=r.BINARY, C=ROOT/'build-ci/073-candidate-continuation/retained-build/runtime-trace',
                I=WORK/'I-original-build/runtime-trace')
PREPARED = dict(B=BINARY, C=WORK/'C-prepared-build/prepared-reflow', I=WORK/'I-prepared-build/prepared-reflow')
PAIRS = [('baseline-self','B','B'), ('candidate-self','C','C'), ('isolated-self','I','I'),
         ('candidate-minus-baseline','B','C'), ('isolated-minus-baseline','B','I'), ('isolated-minus-candidate','C','I')]


def schedule():
    configs = [dict(id=mode+'/'+name, measurement=mode, comparison=name, A=A, B=B,
                   batchSize=96 if mode=='prepared' else None,
                   telemetry='enabled' if mode=='prepared' else 'original',
                   case='prose-480-4096' if mode=='prepared' else 'dense-240-4096')
               for mode in ('prepared','original') for name,A,B in PAIRS]
    rng, orders = random.Random(73084), {}
    for c in configs:
        sequence=[]
        for _ in range(10):
            block=['ABBA','ABBA','BAAB','BAAB'];rng.shuffle(block);sequence.extend(block)
        orders[c['id']]=sequence
    commands=[]
    def add(mode, case=None, variant=None, implementation=None, measurement=None, config=None, rnd=None, position=None, arm=None, functional=None):
        if functional:
            argv=[str(WORK/(implementation+'-diagnostic-build')/functional)]
            if functional=='scan-tests':argv += [str(r.HISTORICAL/'dense-480-1024.json')]
        else:
            argv=[str((PREPARED if measurement=='prepared' else ORIGINAL)[implementation]),str(r.HISTORICAL/(case+'.json')),variant,mode]
            if measurement=='prepared':argv += ['--batch-size','96','--operation-telemetry','enabled']
        seq=len(commands)
        commands.append(dict(sequence=seq,id=f'{seq:04}-{mode}',mode=mode,case=case,variant=variant,
            implementation=implementation,measurement=measurement,configuration=config,round=rnd,
            positionInGroup=position,arm=arm,functional=functional,argv=argv,
            environment='identical inherited environment; frozen before execution'))
    for functional in ('scan-tests','numeric-tests','fast-math-tests'):
        for label in ('C','I'):add('functional',implementation=label,functional=functional)
    for case in r.continuation.old.IDS:
        for variant in ('zero','padded'):
            for label in ('B','C','I'):add('correctness',case,variant,label,'original')
    for label in ('B','C','I'):add('correctness','prose-480-4096','zero',label,'prepared')
    assert len(commands)==81
    for rnd in range(40):
        shuffled=list(configs);rng.shuffle(shuffled)
        for c in shuffled:
            for position,arm in enumerate(orders[c['id']][rnd]):
                add('timing',c['case'],'zero',c[arm],c['measurement'],c['id'],rnd,position,arm)
    return dict(status='approved fixed isolation experiment',seed=73084,configurations=configs,commands=commands)


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
    for kind in ('calibration-revision', 'characterization', 'batching-pilot'):
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


def prepare(output):
    require(not output.exists(), 'fresh output required')
    output.mkdir(parents=True)
    with a.no_launches():
        require(not (WORK/'failure.json').exists(), 'terminal preflight failure')
        audit=json.loads((WORK/'compiled-audit.json').read_text())
        require(audit['passed'] is True, 'compiled isolation not established')
        identities=json.loads((WORK/'input-identities.json').read_text())
        a.diagnosis.verify_identities(identities)
        identities.update(json.loads((WORK/'new-identities.json').read_text()))
        a.diagnosis.verify_identities(identities)
        for path in HERE.iterdir():
            if path.is_file():identities[str(path)]=digest(path)
        for e in schedule()['commands']:
            require(str(Path(e['argv'][0])) in identities,'unfrozen binary')
        shutil.copytree(WORK/'preflight',output/'preflight')
    write(output/'identities.json',identities)
    summary=json.loads((output/'preflight/inputs/continuation/summary.json').read_text())
    limits={}
    for case,phase in r.TARGETS.items():
        cells=[c for c in summary['cells'] if (c['case'],c['variant'],c['operation'])==(case,'zero',phase)]
        require(len(cells)==1,'missing target');limits[case]=cells[0]['overheadLimitPerParagraphNs']
    require(list(limits.values())==[4819.525000000001,101542.07500000001],'exact tolerance mismatch')
    write(output/'limits.json',limits);write(output/'schedule.json',schedule())
    write(output/'commands.json',[e['argv'] for e in schedule()['commands']])
    write(output/'environment.json',environment())
    write(output/'protocol.json',dict(processes=2001,functional=6,correctness=75,timing=1920,seed=73084,
        bootstrapSeed=73085,resamples=10000,batchSize=96,timeoutSeconds=60,rssLimitBytes=4*1024**3,
        retentionEstablished=False,protocolSha256=digest(FEATURE/'prepared-path-isolation-proposal.md')))
    shutil.copyfile(FEATURE/'prepared-path-isolation-proposal.md',output/'protocol.md')
    for name in ('index.jsonl','attempts.jsonl'):(output/name).touch()
    write(output/'frozen.json',{str(p.relative_to(output)):digest(p) for p in output.rglob('*')
                              if p.is_file() and p.name not in ('index.jsonl','attempts.jsonl')})


def rows(raw):
    data = raw.read_bytes()
    require(not data or data.endswith(b'\n'), 'partial JSONL file')
    return [json.loads(line) for line in data.splitlines()]


def validate_raw(raw, entry, output):
    if entry['mode']=='functional':
        text=raw.read_text()
        if entry['functional']=='scan-tests':require('focused cases=134 passed' in text and 'late fallback passed' in text,'scanner completion')
        if entry['functional']=='numeric-tests':require('passed' in text,'numerical completion')
        if entry['functional']=='fast-math-tests':require(not text.strip(),'unexpected fast-math output')
        return []
    data = rows(raw)
    if entry['measurement']=='original':
        expected = r.continuation.old.strict_rows(raw)
        require([x['phase'] for x in expected] == ['initial','reflow','repeat','metadata'], 'original phases')
        blocks = json.loads(Path(entry['argv'][1]).read_text())['fixtures'][0]['blocks']
        for row in expected:
            r.continuation.validate_lifecycle(row, blocks, entry['mode']=='correctness')
            count = sum(bool(b['spans']) for b in blocks) if row['phase']=='metadata' else len(blocks)
            require(row['applicableParagraphs']==count, 'reference applicability')
            require((row['elapsedNs'] is None)==(count==0), 'reference null')
            require(row['elapsedNs'] is None or (type(row['elapsedNs']) is int and row['elapsedNs']>=0), 'reference timer')
            require(row['clockPeriodNs']>0 and row['minimumPositiveDeltaNs']>0 and row['highWaterRssBytes']>0, 'reference timer/resource metadata')
            require(row['fpEnvironment']['rounding']==0 and row['fpEnvironment']['mxcsr'] & 0xe040==0, 'reference floating point environment')
            require(row['cpuAffinity']==json.loads((output/'environment.json').read_text())['affinity'], 'reference affinity')
            require(('geometry' in row)==(entry['mode']=='correctness'), 'reference geometry mode')
        historical = output/'preflight/inputs/continuation'/('correctness-00-'+entry['case']+'-'+entry['variant']+'-baseline-sample.jsonl')
        old = r.continuation.old.strict_rows(historical)
        r.continuation.compare_shapes(expected, old, 'reference generation')
        if entry['mode']=='correctness':
            require([(v['geometry'],v['shapeDeltas']) for v in data] == [(v['geometry'],v['shapeDeltas']) for v in old], 'original geometry/generation changed')
        return data
    require(len(data)==1, 'batch row count')
    row=data[0]; config=dict(batchSize=96,telemetry='enabled')
    n=config['batchSize']; enabled=config['telemetry']=='enabled'
    require(row['phase']=='prepared-reflow' and row['mode']==entry['mode'] and row['telemetry']==config['telemetry'], 'batch identity')
    require(row['applicableParagraphs']==n and isinstance(row['elapsedNs'],int) and row['elapsedNs']>0, 'batch timing')
    require(row['elapsedPerParagraphNs']==row['elapsedNs']/n, 'batch normalization')
    require(row['distinctInstances'] is True and row['generationUnchanged'] is True and row['sharedFontCollection'] is True and row['finalWidth']==497, 'batch lifecycle')
    require(row['cacheCounts']==[0,1,1], 'batch cache counts')
    require(row['initialGenerations']==[1]+[0]*95, 'initial shaping generations')
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
        for metric in ('processCpuNs','voluntary','involuntary'):
            before=row['boundaryBefore'][metric]['value'];after=row['boundaryAfter'][metric]['value']
            expected=None if before is None or after is None else after-before
            require(metric in row['boundaryDeltas'] and row['boundaryDeltas'][metric]==expected, 'invalid boundary delta')
            require(expected is None or expected>=0, 'negative boundary delta')
    if entry['mode']=='correctness':
        ref=next(e for e in schedule()['commands'] if e['mode']=='correctness' and e['measurement']=='original' and e['case']=='prose-480-4096' and e['variant']=='zero' and e['implementation']==entry['implementation'])
        reference=rows(output/(ref['id']+'.jsonl'))[1]['geometry'][0]
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
    require(len(records)==len(attempts)<=2001, 'ambiguous attempted launch')
    require(not complete or len(records)==2001, 'incomplete isolation experiment')
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
    for suffix in ('.jsonl', '.stderr'):
        require({p.name for p in output.glob('[0-9]*'+suffix)}=={e['id']+suffix for e in entries[:len(records)]}, 'unacknowledged output')
    return records


def resume(prior, output):
    require(not output.exists(), 'fresh resume directory required')
    records=validate(prior)
    if len(records)<2001: require(environment()==json.loads((prior/'environment.json').read_text()), 'resume environment mismatch')
    shutil.copytree(prior,output)
    return records


def execute(output,records):
    require(environment()==json.loads((output/'environment.json').read_text()), 'execution environment mismatch')
    validate(output)
    for e in schedule()['commands'][len(records):]:
        if e['sequence']==81:
            require(len(records)==81, '81 functional/correctness completions required')
            validate(output)
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
        require(row['status']=='completed' and row['returncode']==0 and row['command']==e['argv'] and
                0<row['peakObservedRssBytes']<=4*1024**3 and 0<=row['wallSeconds']<=60, str(row))
        records.append(row); print(e['id'],'completed',flush=True)
    validate(output,True)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('action',choices=('run','resume','validate'))
    p.add_argument('output',type=Path);p.add_argument('--from-directory',type=Path);args=p.parse_args()
    with r.recovery.execution_lock(LOCK):
        out=args.output.resolve();source=args.from_directory.resolve() if args.from_directory else None
        if args.action=='validate': validate(out,True)
        elif args.action=='run': prepare(out);execute(out,[])
        else:
            records=resume(source,out)
            if len(records)<2001:execute(out,records)

if __name__=='__main__':main()


def forensic(output):
    """Read-only terminal evidence; never used by execution or recovery."""
    errors=[]
    try: validate(output)
    except Exception as ex: errors.append(type(ex).__name__+': '+str(ex))
    journals={}
    for name in ('attempts.jsonl','index.jsonl'):
        parsed=[]
        for line in (output/name).read_text().splitlines() if (output/name).exists() else []:
            try: parsed.append(json.loads(line))
            except ValueError as ex: errors.append(name+': '+str(ex))
        journals[name]=parsed
    return dict(complete=False,outcome='incomplete',retentionEstablished=False,
                validationErrors=errors,journals=journals,
                preservedFiles={str(x.relative_to(output)):digest(x) for x in sorted(output.rglob('*')) if x.is_file() and x.name not in ('analysis.json','decision.json','report.md')})
