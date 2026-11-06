#!/usr/bin/env python3
"""Fixed serial proof diagnostics; fresh workspaces, terminal failures, no retries."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[3]
PERF=HERE.parent/'performance-experiment'
sys.path.insert(0,str(PERF))
import recovery
from build import digest
from workloads import cases, manifest

def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

supervisor=module('supervisor',PERF/'run.py')
own=module('ownership_verify',HERE.parent/'ownership-experiment/verify.py')
old=module('runtime_driver',HERE.parent/'runtime-comparison/run.py')
instrument=module('ownership_instrument',HERE.parent/'ownership-experiment/instrument.py').instrument
from prototype import apply
IDS=old.IDS
PHASES=old.PHASES

def schedule():
    return [dict(id=f'{case}-{variant}-{mode}',case=case,variant=variant,mode=mode)
            for case in IDS for variant in ('zero','padded') for mode in ('oracle','shadow','prototype')]

def command(cmd,output,name):
    old.run_command(cmd,output,name)

def pinned():
    identities={}
    for directory in ('ownership-experiment','runtime-comparison'):
        archive=HERE.parents[1]/'evidence'/directory
        am=json.loads((archive/'archives.json').read_text())
        identities[str(archive/'archives.json')]=digest(archive/'archives.json')
        for name,r in am['archives'].items():
            assert digest(archive/name)==r['sha256'],name
            identities[str(archive/name)]=r['sha256']
            for relative,expected in r['files'].items():
                f=ROOT/'build-ci'/r.get('sourceDirectory','073-runtime-evidence')/relative
                assert digest(f)==expected,str(f)
                identities[str(f)]=expected
    prior=ROOT/'build-ci/073-ownership-evidence-v2/identities.json'
    for name,expected in json.loads(prior.read_text()).items():
        assert digest(Path(name))==expected,name
        identities[name]=expected
    return identities

def prepare(workspace,output):
    assert not workspace.exists() and not output.exists(),'fresh directories required'
    identities=pinned()
    workspace.mkdir(parents=True);output.mkdir(parents=True)
    (output/'index.jsonl').touch()
    recovery.durable_json(output/'schedule.json',schedule())
    recovery.durable_json(output/'environment.json',dict(kernel=platform.platform(),
        cpu=subprocess.check_output(['lscpu'],text=True),affinity=sorted(os.sched_getaffinity(0)),
        tools={n:subprocess.check_output([n,'--version'],text=True).splitlines()[0] for n in ('c++','cmake','ninja','python3')},
        dependencies=subprocess.check_output(['pkg-config','--modversion','freetype2','harfbuzz','icu-uc','icu-i18n','libpng','fontconfig','zlib'],text=True),
        scope='Diagnostic work only; no runtime-effect interpretation.'))
    source=ROOT/'build-ci/073-ownership-workspace-v2/headline'
    stage=workspace/'prototype'
    shutil.copytree(source,stage,symlinks=True)
    for f in stage.glob('modules/skparagraph/src/*'):
        if f.is_file():assert digest(f)==digest(source/f.relative_to(stage)),str(f)
    supervisor.append(output/'build-commands.jsonl',dict(action='copy verified ownership baseline',source=str(source),destination=str(stage)))
    instrument(stage);apply(stage)
    supervisor.append(output/'build-commands.jsonl',dict(action='apply ownership diagnostics then candidate prototype',driver=str(HERE)))
    # Retain a reviewable patch relative to the verified ownership baseline.
    patch=[]
    for f in stage.glob('modules/skparagraph/src/*'):
        if f.is_file() and (not (source/f.relative_to(stage)).exists() or digest(f)!=digest(source/f.relative_to(stage))):
            import difflib
            before=(source/f.relative_to(stage)).read_text() if (source/f.relative_to(stage)).exists() else ''
            patch.extend(difflib.unified_diff(before.splitlines(True),f.read_text().splitlines(True),fromfile='a/'+str(f.relative_to(stage)),tofile='b/'+str(f.relative_to(stage))))
    (output/'candidate.patch').write_text(''.join(patch))
    command(['ninja','-C',stage/'out/comparison','-j2','skparagraph'],output,'skia-build.log')
    build=workspace/'prototype-build'
    command(['cmake','-S',HERE,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DSKIA='+str(stage),'-DCOUNTERS=ON'],output,'configure.log')
    command(['cmake','--build',build,'-j2','--verbose'],output,'driver-build.log')
    for case in cases():
        if case['id'] in IDS:
            fixture=output/(case['id']+'.json');recovery.durable_json(fixture,manifest(case))
            assert digest(fixture)==digest(ROOT/'build-ci/073-runtime-evidence'/fixture.name)
            for font in manifest(case)['fonts'].values():identities[font]=digest(Path(font))
    for exe,args in [('numeric-tests',[]),('fast-math-tests',[]),('scan-tests',[output/'dense-480-1024.json']),('ownership-tests',[output/'dense-480-1024.json'])]:
        result=supervisor.supervise([build/exe,*args],output/(exe+'.log'),output/(exe+'.stderr'))
        supervisor.append(output/'build-commands.jsonl',result)
        assert result['status']=='completed',(exe,result)
    shutil.copytree(stage/'modules/skparagraph/src',output/'sources')
    shutil.copytree(HERE,output/'driver',ignore=shutil.ignore_patterns('__pycache__'))
    for folder in (HERE,PERF,HERE.parent/'ownership-experiment',HERE.parent/'boundary-experiment',HERE.parent/'runtime-comparison'):
        for f in folder.iterdir():
            if f.is_file():identities[str(f)]=digest(f)
    for f in [*stage.glob('modules/skparagraph/src/*'),*stage.glob('out/comparison/*.a'),
              stage/'out/comparison/obj/skparagraph.ninja',stage/'out/comparison/obj/skia.ninja',stage/'out/comparison/args.gn',
              *build.glob('*')]:
        if f.is_file():identities[str(f)]=digest(f)
    recovery.durable_json(output/'identities.json',identities)
    recovery.checkpoint(output,'build',[output/'identities.json',output/'schedule.json',output/'environment.json',output/'build-commands.jsonl',output/'candidate.patch'])

def validate(raw,entry,output):
    rows=old.strict_rows(raw)
    baseline=old.strict_rows(ROOT/'build-ci/073-runtime-evidence'/f"correctness-00-{entry['case']}-{entry['variant']}-baseline-sample.jsonl")
    for r,b in zip(rows,baseline):
        assert r['geometry']==b['geometry'],'geometry mismatch'
        assert r['shapeDeltas']==b['shapeDeltas'],'shaping mismatch'
        for w,g in zip(r['counters'],r['geometry']):
            s=w['scan']
            assert w['fitting']['calls']==s['evaluations']==w['candidates']
            assert s['scratchSize']<=s['scratchCapacity']
            assert sum(s['fallback'])==0,'unexpected arithmetic fallback'
            assert all(w[bucket]['ownerComparisons']==0 for bucket in ('fitting','formatting'))
            if r['phase'] in ('reflow','repeat'):assert w['ownerBuilds']==w['ownerExaminations']==0
            if entry['variant']=='zero' or entry['mode']=='oracle':
                assert s['preprocessing']==s['trimming']==s['scratchCapacity']==0
            if entry['mode']=='shadow':assert s['shadowChecks']==s['skipped']
            else:assert s['shadowChecks']==0
    oracle=output/f"{entry['case']}-{entry['variant']}-oracle-trace.jsonl"
    if entry['mode']!='oracle':
        for r,b in zip(rows,old.strict_rows(oracle)):
            assert (r['geometry'],r['shapeDeltas'])==(b['geometry'],b['shapeDeltas'])
            for w,o in zip(r['counters'],b['counters']):
                assert w['formatting']==o['formatting']
                assert w['scan']['evaluations']+(w['scan']['skipped'] if entry['mode']=='prototype' else 0)==o['scan']['evaluations']
                if entry['mode']=='shadow':assert w['fitting']==o['fitting']
    return rows

def resume(prior,output):
    assert not output.exists(),'new-directory resume required'
    pinned()
    for name,expected in json.loads((prior/'identities.json').read_text()).items():assert digest(Path(name))==expected,name
    for cp in prior.glob('*-checkpoint.json'):recovery.phase_complete(prior,cp.name.removesuffix('-checkpoint.json'))
    assert json.loads((prior/'schedule.json').read_text())==schedule()
    records=recovery.rows(prior/'index.jsonl');seen=set();known={e['id']:e for e in schedule()}
    for r in records:
        assert r['id'] in known and r['id'] not in seen
        seen.add(r['id'])
        for k,v in known[r['id']].items():assert r[k]==v
        assert digest(prior/r['raw'])==r['rawSha256']
        assert digest(prior/(r['case']+'.json'))==r['fixtureSha256']
        assert r['status']=='completed','terminal prior failure'
        validate(prior/r['raw'],r,prior)
    for raw in prior.glob('*-trace.jsonl'):
        assert raw.name.removesuffix('-trace.jsonl') in seen,'terminal unacknowledged output'
    shutil.copytree(prior,output)
    shutil.copy2(output/'index.jsonl',output/'inherited-index.jsonl')
    return records

def execute(workspace,output,records):
    done={r['id'] for r in records}
    for e in schedule():
        if e['id'] in done:continue
        fixture=output/(e['case']+'.json');raw=output/(e['id']+'-trace.jsonl')
        cmd=[workspace/'prototype-build/phase-trace',fixture,e['variant'],e['mode']]
        supervisor.append(output/'execution-commands.jsonl',list(map(str,cmd)))
        result=supervisor.supervise(cmd,raw,raw.with_suffix('.stderr'))
        error=None
        if result['status']=='completed':
            try:validate(raw,e,output)
            except (AssertionError,ValueError,KeyError,TypeError) as ex:result['status']='invalid-output';error=str(ex)
        row=dict(**e,**result,raw=raw.name,rawSha256=digest(raw),fixtureSha256=digest(fixture),validationError=error)
        supervisor.append(output/'index.jsonl',row)
        assert row['status']=='completed',row
        print(e['id'],'completed',flush=True)
    if not recovery.phase_complete(output,'gates'):
        (workspace/'optimized-build').symlink_to(workspace/'prototype-build',target_is_directory=True)
        old.gates(workspace,output) # prototype is default for historical harness.
    for name,expected in json.loads((output/'identities.json').read_text()).items():assert digest(Path(name))==expected,name
    recovery.checkpoint(output,'finished',[output/'index.jsonl',output/'execution-commands.jsonl'])

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('workspace',type=Path);p.add_argument('output',type=Path);p.add_argument('--resume-from',type=Path);a=p.parse_args()
    with recovery.execution_lock(ROOT/'build-ci/073-performance-recovery/execution.lock'):
        w=a.workspace.resolve();o=a.output.resolve()
        if a.resume_from:records=resume(a.resume_from.resolve(),o)
        else:prepare(w,o);records=[]
        execute(w,o,records)
if __name__=='__main__':main()
