#!/usr/bin/env python3
"""Fixed uninstrumented candidate cutoff study; any process failure is terminal."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
PERF = HERE.parent/'performance-experiment'
sys.path.insert(0, str(PERF))
import recovery
from build import digest
from workloads import cases, block, manifest


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


old = module('runtime_driver', HERE.parent/'runtime-comparison/run.py')
supervisor = old.supervisor
sys.path.insert(0, str(HERE.parent/'candidate-scan-implementation'))
implementation = module('retained_driver', HERE.parent/'candidate-scan-implementation/run.py')
PHASES = old.PHASES
SMALL = tuple(f'small-{kind}-480-{size}' for kind in ('prose','dense','mixed','long','objects') for size in (64,256))
IDS = (*old.IDS, *SMALL)
BASELINE = ROOT/'build-ci/073-runtime-evidence'
FORBIDDEN = ('scanMode','scanWork','phaseWork','phaseCandidates','shadowChecks','cutoff-proof','ownerExaminations','PHASE_COUNTERS')


def fixtures():
    result = [c for c in cases() if c['id'] in old.IDS]
    for kind in ('prose','dense','mixed','long','objects'):
        for size in (64,256):
            result.append(dict(id=f'small-{kind}-480-{size}',series=f'small-{kind}',size=size,
                               width=480,blocks=[block(kind,size) for _ in range(32)]))
    return result


def schedule():
    return [dict(id=f'{mode}-{sample:02}-{case}-{variant}-{label}',mode=mode,sample=sample,
                 case=case,variant=variant,label=label,pair=f'{mode}-{sample:02}-{case}-{variant}',order=order)
            for mode, passes in [('correctness',range(1)),('timing',range(20))]
            for sample in passes for case in IDS for variant in ('zero','padded')
            for order,label in enumerate(('baseline','retained') if sample%2==0 else ('retained','baseline'))]


def verify_archive(directory):
    """Read back immutable evidence, not unchanged proof-suite execution."""
    import hashlib
    import tarfile
    identities = {}
    index = HERE.parents[1]/'evidence'/directory/'archives.json'
    identities[str(index)] = digest(index)
    for name, record in json.loads(index.read_text())['archives'].items():
        archive = index.parent/name
        assert digest(archive)==record['sha256'], name
        with tarfile.open(archive,'r:gz') as tar:
            found = {}
            for member in tar:
                assert member.isfile()
                h = hashlib.sha256()
                with tar.extractfile(member) as stream:
                    for chunk in iter(lambda:stream.read(1024*1024),b''):h.update(chunk)
                found[member.name] = h.hexdigest()
            assert found==record['files'], name
        identities[str(archive)] = record['sha256']
    return identities


def command(cmd, output, name, journal='build-commands.jsonl'):
    old.run_command(cmd,output,name,journal)


def audit(workspace, output):
    source = ROOT/'build-ci/073-ownership-workspace-v2/headline/modules/skparagraph/src'
    left = workspace/'baseline/modules/skparagraph/src'
    right = workspace/'retained/modules/skparagraph/src'
    assert {f.name:digest(f) for f in source.iterdir() if f.is_file()} == {f.name:digest(f) for f in left.iterdir() if f.is_file()}
    patch = implementation.diff(left,right)
    assert patch==(HERE.parent/'candidate-scan-implementation/candidate.patch').read_text()
    flags = []
    for label in ('baseline','retained'):
        stage = workspace/label
        for f in (stage/'modules/skparagraph/src').iterdir():
            if f.is_file():assert not any(token in f.read_text() for token in FORBIDDEN), str(f)
        build = workspace/(label+'-build')
        text = (build/'build.ninja').read_text()
        assert not any(token in text for token in FORBIDDEN)
        flags.append([line.strip().replace(str(stage),'STAGE') for line in text.splitlines()
                      if line.strip().startswith(('FLAGS =','DEFINES =','INCLUDES ='))])
        symbols = subprocess.check_output(['nm','-C',build/'runtime-trace'],text=True)
        assert not any(token in symbols for token in FORBIDDEN)
    assert flags[0]==flags[1], 'asymmetric driver flags'
    for relative in ('args.gn','obj/modules/skparagraph/skparagraph.ninja'):
        assert (workspace/'baseline/out/comparison'/relative).read_bytes()==(workspace/'retained/out/comparison'/relative).read_bytes()
    for f in (workspace/'baseline/out/comparison').glob('*.a'):
        if f.name!='libskparagraph.a':assert digest(f)==digest(workspace/'retained/out/comparison'/f.name)
    recovery.durable_json(output/'preflight.json',dict(sourceDifference=patch,flags=flags[0],
        forbiddenTokens=list(FORBIDDEN),binaryAudit='nm -C both runtime-trace binaries; no forbidden symbols',
        affinity=sorted(os.sched_getaffinity(0))))


def prepare(workspace, output):
    assert not workspace.exists() and not output.exists(), 'fresh directories required'
    identities = implementation.pinned()
    for folder in ('candidate-scan-experiment','candidate-scan-implementation'):
        identities.update(verify_archive(folder))
    # Anchor the clean source and proof identities as well as archived member hashes.
    prior = ROOT/'build-ci/073-candidate-implementation-evidence-v4/identities.json'
    for name, expected in json.loads(prior.read_text()).items():
        assert digest(Path(name))==expected,name
        identities[name]=expected
    workspace.mkdir(parents=True);output.mkdir(parents=True)
    (output/'index.jsonl').touch()
    recovery.durable_json(output/'schedule.json',schedule())
    recovery.durable_json(output/'environment.json',dict(kernel=platform.platform(),
        cpu=subprocess.check_output(['lscpu'],text=True),affinity=sorted(os.sched_getaffinity(0)),load=os.getloadavg(),
        tools={n:subprocess.check_output([n,'--version'],text=True).splitlines()[0] for n in ('c++','cmake','ninja','python3')},
        dependencies=subprocess.check_output(['pkg-config','--modversion','freetype2','harfbuzz','icu-uc','icu-i18n','libpng','fontconfig','zlib'],text=True),
        cacheClaim='Identical inherited affinity; uncontrolled caches; no independently cold-cache claim.'))
    source = ROOT/'build-ci/073-ownership-workspace-v2/headline'
    for label in ('baseline','retained'):
        stage = workspace/label
        shutil.copytree(source,stage,symlinks=True)
        supervisor.append(output/'build-commands.jsonl',dict(copy=str(source),destination=str(stage)))
        if label=='retained':
            implementation.apply(stage)
        # Clean copied object outputs so both arms compile fresh with the pinned recipe.
        command(['ninja','-C',stage/'out/comparison','-t','clean'],output,label+'-clean.log')
        command(['ninja','-C',stage/'out/comparison','-j2','skparagraph'],output,label+'-skia.log')
        build = workspace/(label+'-build')
        command(['cmake','-S',HERE,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DSKIA='+str(stage)],output,label+'-configure.log')
        command(['cmake','--build',build,'-j2','--verbose'],output,label+'-driver.log')
        shutil.copytree(stage/'modules/skparagraph/src',output/'sources'/label)
        for f in [*(stage/'modules/skparagraph/src').glob('*'),*(stage/'out/comparison').rglob('*.ninja'),
                  *(stage/'out/comparison').glob('*.a'),stage/'out/comparison/args.gn',*build.glob('*')]:
            if f.is_file():identities[str(f)]=digest(f)
    audit(workspace,output)
    for case in fixtures():
        fixture = output/(case['id']+'.json')
        recovery.durable_json(fixture,manifest(case))
        if case['id'] in old.IDS:assert digest(fixture)==digest(BASELINE/fixture.name)
        for font in manifest(case)['fonts'].values():identities[font]=digest(Path(font))
    for folder in (HERE,PERF,HERE.parent/'runtime-comparison',HERE.parent/'candidate-scan-implementation',HERE.parent/'boundary-experiment',HERE.parent/'ownership-experiment'):
        for f in folder.iterdir():
            if f.is_file():identities[str(f)]=digest(f)
    shutil.copytree(HERE,output/'driver',ignore=shutil.ignore_patterns('__pycache__'))
    shutil.copy2(HERE.parent/'candidate-scan-implementation/candidate.patch',output/'candidate.patch')
    recovery.durable_json(output/'identities.json',identities)
    recovery.checkpoint(output,'build',[output/'identities.json',output/'schedule.json',output/'environment.json',output/'build-commands.jsonl',output/'preflight.json',*output.glob('*.json')])


def validate_lifecycle(row, blocks, correctness):
    phase=row['phase']
    deltas=row['shapeDeltas']
    assert len(deltas)==len(blocks), f'{phase}: shaping paragraph count mismatch'
    assert ('cacheEvents' in row)==correctness, f'{phase}: cache diagnostics mode mismatch'
    if correctness:assert len(row['cacheEvents'])==len(blocks)
    for i,(delta,b) in enumerate(zip(deltas,blocks)):
        metadata=bool(b['spans'])
        allowed=(0,1) if phase=='initial' and not metadata else (int(phase=='initial' or (phase=='metadata' and metadata)),)
        assert type(delta) is int and delta in allowed, f'paragraph {i} phase {phase}: expected shaping delta {allowed}, observed {delta}'
        if correctness:
            expected=[]
            if phase=='initial' and not metadata:
                expected=['foundParagraph'] if delta==0 else ['missingParagraph','addedParagraph']
            assert row['cacheEvents'][i]==expected, f'paragraph {i} phase {phase}: expected cache events {expected}, observed {row["cacheEvents"][i]}, shaping delta {delta}'


def compare_shapes(rows, reference, context):
    for row,other in zip(rows,reference):
        assert len(row['shapeDeltas'])==len(other['shapeDeltas']), f'{context}: shaping paragraph count mismatch'
        for i,(delta,expected) in enumerate(zip(row['shapeDeltas'],other['shapeDeltas'])):
            assert delta==expected, f'{context}: paragraph {i} phase {row["phase"]}: expected shaping delta {expected}, observed {delta}'


def validate_output(raw, entry, output):
    rows = old.strict_rows(raw)
    fixture = json.loads((output/(entry['case']+'.json')).read_text())
    blocks = fixture['fixtures'][0]['blocks']
    for row in rows:
        phase = row['phase']
        validate_lifecycle(row,blocks,entry['mode']=='correctness')
        count = sum(bool(b['spans']) for b in blocks) if phase=='metadata' else len(blocks)
        assert row['applicableParagraphs']==count
        assert (row['elapsedNs'] is None)==(count==0)
        assert row['elapsedNs'] is None or (type(row['elapsedNs']) is int and row['elapsedNs']>=0)
        assert row['clockPeriodNs']>0 and row['minimumPositiveDeltaNs']>0 and row['highWaterRssBytes']>0
        assert row['fpEnvironment']['rounding']==0 and row['fpEnvironment']['mxcsr'] & 0xe040==0
        assert row['cpuAffinity']==json.loads((output/'environment.json').read_text())['affinity']
        assert ('geometry' in row)==(entry['mode']=='correctness')
    if entry['mode']=='correctness':
        if entry['case'] in old.IDS:
            archived = old.strict_rows(BASELINE/f"correctness-00-{entry['case']}-{entry['variant']}-baseline-sample.jsonl")
            assert [(r['geometry'],r['shapeDeltas']) for r in rows]==[(r['geometry'],r['shapeDeltas']) for r in archived], 'archived geometry mismatch'
        partner = output/f"correctness-00-{entry['case']}-{entry['variant']}-baseline-sample.jsonl"
        if entry['label']=='retained':
            paired=old.strict_rows(partner)
            compare_shapes(rows,paired,'paired arms')
            assert [r['geometry'] for r in rows]==[r['geometry'] for r in paired], 'paired geometry mismatch'
    else:
        reference=old.strict_rows(output/f"correctness-00-{entry['case']}-{entry['variant']}-{entry['label']}-sample.jsonl")
        compare_shapes(rows,reference,'timing/correctness')
    return rows


def resume(prior, output):
    assert not output.exists(),'new-directory resume required'
    for name,expected in json.loads((prior/'identities.json').read_text()).items():assert digest(Path(name))==expected,name
    for cp in prior.glob('*-checkpoint.json'):recovery.phase_complete(prior,cp.name.removesuffix('-checkpoint.json'))
    assert json.loads((prior/'schedule.json').read_text())==schedule()
    records=recovery.rows(prior/'index.jsonl')
    assert len(records)<=len(schedule())
    for row,entry in zip(records,schedule()):
        for k,v in entry.items():assert row[k]==v
        assert row['status']=='completed','terminal prior failure'
        assert digest(prior/row['raw'])==row['rawSha256']
        assert digest(prior/(row['case']+'.json'))==row['fixtureSha256']
        validate_output(prior/row['raw'],row,prior)
    acknowledged={r['raw'] for r in records}
    assert all(f.name in acknowledged for f in prior.glob('*-sample.jsonl')), 'terminal unacknowledged output'
    shutil.copytree(prior,output)
    shutil.copy2(output/'index.jsonl',output/'inherited-index.jsonl')
    return records


def gates(workspace, output):
    if recovery.phase_complete(output,'gates'):return
    # Reuse the historical scripts and tolerances with the retained geometry binary.
    (workspace/'optimized-build').symlink_to(workspace/'retained-build',target_is_directory=True)
    old.gates(workspace,output)


def execute(workspace, output, records):
    for entry in schedule()[len(records):]:
        if entry['mode']=='timing' and not recovery.phase_complete(output,'correctness'):
            assert len(records)==88 and all(r['status']=='completed' for r in records)
            gates(workspace,output)
            recovery.checkpoint(output,'correctness',[output/r['raw'] for r in records])
        fixture=output/(entry['case']+'.json');raw=output/(entry['id']+'-sample.jsonl')
        cmd=[workspace/(entry['label']+'-build/runtime-trace'),fixture,entry['variant'],entry['mode']]
        supervisor.append(output/'execution-commands.jsonl',list(map(str,cmd)))
        result=supervisor.supervise(cmd,raw,raw.with_suffix('.stderr'))
        error=None
        if result['status']=='completed':
            try:validate_output(raw,entry,output)
            except (AssertionError,ValueError,KeyError,TypeError) as ex:result['status']='invalid-output';error=str(ex)
        row=dict(**entry,**result,raw=raw.name,rawSha256=digest(raw),fixtureSha256=digest(fixture),validationError=error)
        supervisor.append(output/'index.jsonl',row)
        assert row['status']=='completed',row
        records.append(row)
        print(entry['id'],'completed',flush=True)
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
