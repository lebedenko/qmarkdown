#!/usr/bin/env python3
"""Uninstrumented serial paired study; terminal outcomes are never retried."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tarfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
PERF = HERE.parent/'performance-experiment'
sys.path.insert(0, str(PERF))
import recovery
from build import digest
from workloads import cases, manifest
spec = importlib.util.spec_from_file_location('perf_runner', PERF/'run.py')
supervisor = importlib.util.module_from_spec(spec)
spec.loader.exec_module(supervisor)
spec = importlib.util.spec_from_file_location('ownership_verify', HERE.parent/'ownership-experiment/verify.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
IDS = ('prose-480-4096', 'dense-480-1024', 'dense-480-4096', 'dense-240-4096',
       'dense-960-4096', 'mixed-480-4096', 'objects-480-4096', 'long-480-16384',
       'spans-480-1', 'spans-480-8', 'spans-480-32', 'document-480-10')
PHASES = ['initial', 'reflow', 'repeat', 'metadata']


def schedule():
    return [dict(id=f'{mode}-{sample:02}-{case}-{variant}-{label}', mode=mode,
                 sample=sample, case=case, variant=variant, label=label,
                 pair=f'{mode}-{sample:02}-{case}-{variant}', order=order)
            for mode, passes in [('correctness', range(1)), ('timing', range(10))]
            for sample in passes for case in IDS for variant in ('zero', 'padded')
            for order, label in enumerate(('baseline', 'optimized') if sample % 2 == 0
                                          else ('optimized', 'baseline'))]


def strict_rows(path):
    data = path.read_bytes()
    assert data.endswith(b'\n'), 'partial output'
    rows = [json.loads(line) for line in data.splitlines()]
    assert [r['phase'] for r in rows] == PHASES, 'incomplete operations'
    return rows


def validate_output(path, entry, fixture, baseline):
    rows = strict_rows(path)
    expected = recovery.rows(baseline/f"{entry['case']}-{entry['variant']}-headline-trace.jsonl")
    assert len(expected) == 4
    for row, old in zip(rows, expected):
        assert row['shapeDeltas'] == old['shapeDeltas'], 'shaping invariant failed'
        assert row['clockPeriodNs'] > 0 and row['minimumPositiveDeltaNs'] > 0
        assert row['highWaterRssBytes'] > 0
        assert isinstance(row['applicableParagraphs'], int) and row['applicableParagraphs'] >= 0
        if row['phase'] != 'metadata':
            assert row['applicableParagraphs'] == len(old['shapeDeltas'])
        else:
            assert row['applicableParagraphs'] == sum(old['shapeDeltas'])
        assert (row['elapsedNs'] is None) == (row['applicableParagraphs'] == 0)
        assert row['elapsedNs'] is None or (isinstance(row['elapsedNs'], int) and row['elapsedNs'] >= 0)
        if entry['mode'] == 'correctness':
            assert row['geometry'] == old['geometry'], 'geometry invariant failed'
        else:
            assert 'geometry' not in row
    assert digest(fixture) == digest(baseline/fixture.name), 'changed fixture'
    return rows


def run_command(command, output, name, journal='build-commands.jsonl'):
    supervisor.append(output/journal, list(map(str, command)))
    with (output/name).open('w') as log:
        subprocess.run(list(map(str, command)), check=True, stdout=log,
                       stderr=subprocess.STDOUT, pass_fds=recovery.LOCK_FDS)


def prepare(workspace, output, baseline):
    assert not workspace.exists() and not output.exists(), 'fresh directories required'
    identities = verify.validate_baseline(baseline)
    archive = HERE.parents[1]/'evidence/ownership-experiment'
    am = json.loads((archive/'archives.json').read_text())
    identities[str(archive/'archives.json')] = digest(archive/'archives.json')
    for name, record in am['archives'].items():
        assert digest(archive/name) == record['sha256'], name
        identities[str(archive/name)] = record['sha256']
        directory = ROOT/'build-ci'/record['sourceDirectory']
        for relative, expected in record['files'].items():
            assert digest(directory/relative) == expected, relative
            identities[str(directory/relative)] = expected
    # Archived identities additionally anchor the retained libraries and ownership source.
    prior = json.loads((ROOT/'build-ci/073-ownership-evidence-v2/identities.json').read_text())
    for name, expected in prior.items():
        assert digest(Path(name)) == expected, name
        identities[name] = expected
    source = ROOT/'build-ci/073-performance-recovery/sources/control'
    patched = ROOT/'build-ci/073-performance-recovery/sources/patched'
    build_record = ROOT/'build-ci/073-performance-v2/build.json'
    record = json.loads(build_record.read_text())
    identities[str(build_record)] = digest(build_record)
    for name, expected in record['boundaryBuild']['patchedSources'].items():
        assert digest(patched/name) == expected, name
        identities[str(patched/name)] = expected
    restored = ROOT/'build-ci/073-performance-recovery/recovery-manifest.json'
    identities[str(restored)] = digest(restored)
    restored_record = json.loads(restored.read_text())
    assert restored_record['base'] == record['boundaryBuild']['base']
    subprocess.run(['git','diff','--exit-code','HEAD','--'],cwd=source,check=True)
    assert subprocess.check_output(['git','rev-parse','HEAD'],cwd=source,text=True).strip() == restored_record['base']
    for name, expected in restored_record['controlIdentities'].items():
        assert digest(source/name) == expected, name
        identities[str(source/name)] = expected
    workspace.mkdir(parents=True); output.mkdir(parents=True)
    (output/'index.jsonl').touch()
    recovery.durable_json(output/'schedule.json', schedule())
    recovery.durable_json(output/'environment.json', dict(kernel=platform.platform(),
        cpu=subprocess.check_output(['lscpu'], text=True), affinity=sorted(os.sched_getaffinity(0)),
        load=os.getloadavg(), tools={n:subprocess.check_output([n,'--version'],text=True).splitlines()[0]
                                    for n in ('c++','cmake','ninja','python3')},
        dependencies=subprocess.check_output(['pkg-config','--modversion','freetype2','harfbuzz','icu-uc','icu-i18n','libpng','fontconfig','zlib'],text=True),
        cacheClaim='Fresh processes only; filesystem caches are uncontrolled; no independent warming.'))
    for label in ('baseline','optimized'):
        stage = workspace/label
        shutil.copytree(source, stage, symlinks=True)
        for name in record['boundaryBuild']['patchedSources']:
            shutil.copyfile(patched/name, stage/name)
        if label == 'optimized':
            command = ['patch','--batch','--fuzz=0','-p1','-i',str(HERE.parent/'ownership-experiment/ownership.patch')]
            supervisor.append(output/'build-commands.jsonl', dict(command=command,cwd=str(stage)))
            with (output/'patch.log').open('w') as log:
                subprocess.run(command,cwd=stage,check=True,stdout=log,stderr=subprocess.STDOUT,pass_fds=recovery.LOCK_FDS)
            archived = ROOT/'build-ci/073-ownership-workspace-v2/headline'
            for f in stage.glob('modules/skparagraph/src/*'):
                if f.is_file():assert digest(f) == digest(archived/f.relative_to(stage)), str(f)
        run_command(['ninja','-C',stage/'out/comparison','-j2','skparagraph'], output, label+'-skia.log')
        build = workspace/(label+'-build')
        run_command(['cmake','-S',HERE,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DSKIA='+str(stage)],output,label+'-configure.log')
        run_command(['cmake','--build',build,'-j2','--verbose'],output,label+'-build.log')
        (output/'sources'/label).mkdir(parents=True)
        for f in stage.glob('modules/skparagraph/src/*'):
            if f.is_file():shutil.copy2(f,output/'sources'/label/f.name)
        for f in [*stage.glob('modules/skparagraph/src/*'),*stage.glob('out/comparison/*.a'),
                  stage/'out/comparison/obj/skia.ninja',stage/'out/comparison/args.gn',
                  build/'runtime-trace',build/'perf-geometry',build/'build.ninja',build/'CMakeCache.txt']:
            if f.is_file():identities[str(f)] = digest(f)
    for folder in (HERE, PERF, HERE.parent/'boundary-experiment', HERE.parent/'ownership-experiment'):
        for f in folder.iterdir():
            if f.is_file():identities[str(f)] = digest(f)
    for case in cases():
        if case['id'] in IDS:
            fixture = output/(case['id']+'.json')
            recovery.durable_json(fixture, manifest(case))
            assert digest(fixture) == digest(baseline/fixture.name)
            for font in manifest(case)['fonts'].values():identities[font] = digest(Path(font))
    shutil.copytree(HERE,output/'driver',ignore=shutil.ignore_patterns('__pycache__'))
    shutil.copy2(HERE.parent/'ownership-experiment/ownership.patch',output/'ownership.patch')
    shutil.copy2(HERE.parent/'boundary-experiment/fragment-edges.patch',output/'boundary.patch')
    recovery.durable_json(output/'identities.json', identities)
    recovery.checkpoint(output,'build',[output/'identities.json',output/'schedule.json',output/'environment.json',output/'build-commands.jsonl',*output.glob('*.json')])


def invariant_failure(row, output):
    error = row.get('validationError') or ''
    stderr = (output/row['raw']).with_suffix('.stderr')
    diagnostic = stderr.read_text(errors='replace') if stderr.exists() else ''
    return any(word in error for word in ('geometry', 'shaping')) or 'shaping invariant failed' in diagnostic


def resume(prior, output):
    assert not output.exists(), 'new-directory resume required'
    for name, expected in json.loads((prior/'identities.json').read_text()).items():
        assert digest(Path(name)) == expected, name
    for cp in prior.glob('*-checkpoint.json'):
        recovery.phase_complete(prior,cp.name.removesuffix('-checkpoint.json'))
    assert json.loads((prior/'schedule.json').read_text()) == schedule(), 'changed schedule'
    records = recovery.rows(prior/'index.jsonl')
    known = {e['id']:e for e in schedule()}
    seen = set()
    for row in records:
        assert not invariant_failure(row, prior), 'study stopped: prior geometry/shaping failure'
        assert row['id'] not in seen and row['id'] in known
        seen.add(row['id'])
        for key, value in known[row['id']].items():assert row[key] == value
        assert digest(prior/row['raw']) == row['rawSha256']
        assert digest(prior/(row['case']+'.json')) == row['fixtureSha256']
        if row['status'] == 'completed':
            validate_output(prior/row['raw'],row,prior/(row['case']+'.json'),ROOT/'build-ci/073-phase-evidence-v3')
    shutil.copytree(prior,output)
    shutil.copy2(output/'index.jsonl',output/'inherited-index.jsonl')
    for raw in output.glob('*-sample.jsonl'):
        identity = raw.name.removesuffix('-sample.jsonl')
        if identity not in seen:
            assert identity in known
            entry = known[identity]
            records.append(dict(**entry,status='interrupted',raw=raw.name,rawSha256=digest(raw),
                                fixtureSha256=digest(output/(entry['case']+'.json'))))
    (output/'index.jsonl').write_text(''.join(json.dumps(row)+'\n' for row in records))
    return records


def gates(workspace, output):
    if recovery.phase_complete(output,'gates'):return
    for label,exe in [('control',ROOT/'build-ci/073-performance-recovery/diagnostics/control-build/perf-geometry'),
                      ('patched',workspace/'optimized-build/perf-geometry')]:
        directory = output/'gates'/label
        directory.mkdir(parents=True,exist_ok=True)
        if not (directory/'fragment-preflight').exists():(directory/'fragment-preflight').symlink_to(exe)
    for script, args in [('verify.py',[output/'gates',output/'full-geometry-gates']),
                         ('audit.py',[output/'full-geometry-gates',output/'extent-audit.json'])]:
        run_command([sys.executable,HERE.parent/'boundary-experiment'/script,*args],output,script+'.log',journal='gate-commands.jsonl')
    recovery.checkpoint(output,'gates',[output/'full-geometry-gates/summary.json',output/'extent-audit.json'])


def execute(workspace, output, baseline, records):
    stopped = {(r['case'],r['variant']) for r in records if r['status'] != 'completed'}
    done = {r['id']:r for r in records}
    for entry in schedule():
        if entry['mode'] == 'timing' and not recovery.phase_complete(output,'correctness'):
            correctness = [r for r in records if r['mode'] == 'correctness']
            assert len(correctness) == 48 and all(r['status']=='completed' for r in correctness), 'correctness incomplete; timing prohibited'
            gates(workspace,output)
            recovery.checkpoint(output,'correctness',[output/r['raw'] for r in correctness])
        if entry['id'] in done or (entry['case'],entry['variant']) in stopped:continue
        fixture = output/(entry['case']+'.json')
        raw = output/(entry['id']+'-sample.jsonl')
        result = supervisor.supervise([workspace/(entry['label']+'-build/runtime-trace'),fixture,entry['variant'],entry['mode']],raw,raw.with_suffix('.stderr'))
        failure = None
        if result['status'] == 'completed':
            try:validate_output(raw,entry,fixture,baseline)
            except (AssertionError,ValueError,KeyError,TypeError) as error:
                result['status'] = 'invalid-output';failure = str(error)
        row = dict(**entry,**result,raw=raw.name,rawSha256=digest(raw),fixtureSha256=digest(fixture),validationError=failure)
        supervisor.append(output/'index.jsonl',row)
        records.append(row);done[entry['id']] = row
        if row['status'] != 'completed':stopped.add((entry['case'],entry['variant']))
        if invariant_failure(row, output):
            raise RuntimeError('study stopped: geometry/shaping failure '+entry['id'])
        print(entry['id'],row['status'],flush=True)
    for name, expected in json.loads((output/'identities.json').read_text()).items():assert digest(Path(name)) == expected,name
    recovery.checkpoint(output,'finished',[output/'index.jsonl'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('workspace',type=Path);parser.add_argument('output',type=Path)
    parser.add_argument('--resume-from',type=Path)
    args = parser.parse_args()
    workspace=args.workspace.resolve();output=args.output.resolve()
    baseline=ROOT/'build-ci/073-phase-evidence-v3'
    with recovery.execution_lock(ROOT/'build-ci/073-performance-recovery/execution.lock'):
        if args.resume_from:records=resume(args.resume_from.resolve(),output)
        else:prepare(workspace,output,baseline);records=[]
        execute(workspace,output,baseline,records)

if __name__ == '__main__':main()
