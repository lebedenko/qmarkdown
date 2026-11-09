#!/usr/bin/env python3
"""Bounded serial phase study. Resume only into a new directory; no automatic retries."""
import argparse
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import sys

HERE = Path(__file__).resolve().parent
PERF = HERE.parent/'performance-experiment'
sys.path.insert(0, str(PERF))
from build import digest
import recovery
from workloads import cases, manifest
spec = importlib.util.spec_from_file_location('performance_run', PERF/'run.py')
supervisor = importlib.util.module_from_spec(spec)
spec.loader.exec_module(supervisor)
from instrument import instrument

IDS = {'prose-480-4096', 'dense-480-1024', 'dense-480-4096', 'dense-240-4096',
       'dense-960-4096', 'mixed-480-4096', 'objects-480-4096', 'long-480-16384',
       'spans-480-1', 'spans-480-8', 'spans-480-32', 'document-480-10'}


def resume(prior, output):
    assert not output.exists(), 'new-directory resume required'
    identities = json.loads((prior/'identities.json').read_text())
    for name, expected in identities.items():
        assert digest(Path(name)) == expected, name
    for cp in prior.glob('*-checkpoint.json'):
        recovery.phase_complete(prior, cp.name.removesuffix('-checkpoint.json'))
    records = recovery.rows(prior/'index.jsonl')
    for row in records:
        assert digest(prior/row['raw']) == row['rawSha256']
        assert digest(prior/(row['case']+'.json')) == row['fixtureSha256']
        if row['status']=='completed':
            assert [x['phase'] for x in recovery.rows(prior/row['raw'])]==['initial','reflow','repeat','metadata']
    shutil.copytree(prior, output)
    acknowledged = {r['raw'] for r in records}
    for raw in output.glob('*-trace.jsonl'):
        if raw.name not in acknowledged:
            # An interrupted process is terminal even if the supervisor died before acknowledgement.
            case, variant, label = raw.name.removesuffix('-trace.jsonl').rsplit('-',2)
            records.append(dict(case=case,variant=variant,label=label,raw=raw.name,
                rawSha256=digest(raw),fixtureSha256=digest(output/(case+'.json')),status='interrupted'))
    shutil.copy2(output/'index.jsonl',output/'inherited-index.jsonl')
    (output/'index.jsonl').write_text(''.join(json.dumps(r)+'\n' for r in records))
    return records


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('workspace',type=Path)
    p.add_argument('output',type=Path)
    p.add_argument('--resume-from',type=Path)
    args=p.parse_args()
    root=HERE.parents[3]
    with recovery.execution_lock(root/'build-ci/073-performance-recovery/execution.lock'):
        workspace=args.workspace.resolve(); output=args.output.resolve()
        if args.resume_from:
            records=resume(args.resume_from,output)
        else:
            assert not workspace.exists() and not output.exists()
            workspace.mkdir(parents=True);output.mkdir(parents=True)
            records=[]
            (output/'index.jsonl').touch()
            patched=root/'build-ci/073-performance-recovery/sources/patched'
            source=root/'build-ci/073-performance-recovery/sources/control'
            baseline=root/'build-ci/073-performance-v2/build.json'
            record=json.loads(baseline.read_text())
            for name, expected in record['boundaryBuild']['patchedSources'].items():
                assert digest(patched/name)==expected, name
            restored=root/'build-ci/073-performance-recovery/recovery-manifest.json'
            identities={str(baseline):digest(baseline),str(restored):digest(restored)}
            for f in source.rglob('*'):
                if f.is_file() and (f.suffix=='.a' or f.name in ('args.gn','comparison-build.json')):
                    identities[str(f)]=digest(f)
            for label in ('headline','instrumented'):
                stage=workspace/label
                shutil.copytree(source,stage,symlinks=True)
                for name in record['boundaryBuild']['patchedSources']:
                    shutil.copyfile(patched/name,stage/name)
                if label=='instrumented':
                    instrument(stage)
                with (output/(label+'-skia-build.log')).open('w') as log:
                    subprocess.run(['ninja','-C',str(stage/'out/comparison'),'-j2','skparagraph'],check=True,stdout=log,stderr=subprocess.STDOUT,pass_fds=recovery.LOCK_FDS)
                build=workspace/(label+'-build')
                with (output/(label+'-build.log')).open('w') as log:
                    for command in (['cmake','-S',str(HERE),'-B',str(build),'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DSKIA='+str(stage),'-DCOUNTERS='+('ON' if label=='instrumented' else 'OFF')],['cmake','--build',str(build),'-j2']):
                        subprocess.run(command,check=True,stdout=log,stderr=subprocess.STDOUT,pass_fds=recovery.LOCK_FDS)
                for f in [*stage.glob('modules/skparagraph/src/*'),*stage.glob('out/comparison/*.a'),build/'phase-trace',build/'perf-geometry']:
                    if f.is_file():identities[str(f)]=digest(f)
            for folder in (HERE, PERF, HERE.parent/'boundary-experiment'):
                for f in folder.iterdir():
                    if f.is_file():identities[str(f)]=digest(f)
            recovery.durable_json(output/'identities.json',identities)
            recovery.checkpoint(output,'build',[output/'identities.json'])
        for case in cases():
            if case['id'] not in IDS:continue
            fixture=output/(case['id']+'.json')
            if not fixture.exists():recovery.durable_json(fixture,manifest(case))
            for variant in ('zero','padded'):
                pair=[]
                for label in ('instrumented','headline'):
                    old=next((r for r in records if (r['case'],r['variant'],r['label'])==(case['id'],variant,label)),None)
                    raw=output/f"{case['id']}-{variant}-{label}-trace.jsonl"
                    if old is None:
                        result=supervisor.supervise([workspace/(label+'-build/phase-trace'),fixture,variant],raw,raw.with_suffix('.stderr'))
                        old=dict(case=case['id'],variant=variant,label=label,raw=raw.name,rawSha256=digest(raw),fixtureSha256=digest(fixture),**result)
                        supervisor.append(output/'index.jsonl',old)
                    if old['status']!='completed':break
                    rows=recovery.rows(raw)
                    assert [r['phase'] for r in rows]==['initial','reflow','repeat','metadata']
                    pair.append(rows)
                if len(pair)==2:
                    assert [(r['geometry'],r['shapeDeltas']) for r in pair[0]]==[(r['geometry'],r['shapeDeltas']) for r in pair[1]]
                    recovery.checkpoint(output,case['id']+'-'+variant,[output/f"{case['id']}-{variant}-{label}-trace.jsonl" for label in ('instrumented','headline')])
        if not recovery.phase_complete(output,'gates'):
            gates=output/'gates'
            for label,exe in [('control',root/'build-ci/073-performance-recovery/diagnostics/control-build/perf-geometry'),('patched',workspace/'instrumented-build/perf-geometry')]:
                (gates/label).mkdir(parents=True)
                (gates/label/'fragment-preflight').symlink_to(exe)
            with (output/'gates.log').open('w') as log:
                for cmd in (['verify.py',str(gates),str(output/'full-geometry-gates')],['audit.py',str(output/'full-geometry-gates'),str(output/'extent-audit.json')]):
                    subprocess.run([sys.executable,str(HERE.parent/'boundary-experiment'/cmd[0]),*cmd[1:]],check=True,stdout=log,stderr=subprocess.STDOUT,pass_fds=recovery.LOCK_FDS)
            recovery.checkpoint(output,'gates',[output/'full-geometry-gates/summary.json',output/'extent-audit.json'])
        for name, expected in json.loads((output/'identities.json').read_text()).items():
            assert digest(Path(name))==expected, name
        recovery.checkpoint(output,'finished',[output/'index.jsonl'])

if __name__=='__main__':main()
