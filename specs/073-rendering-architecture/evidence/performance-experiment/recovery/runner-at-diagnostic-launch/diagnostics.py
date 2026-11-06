#!/usr/bin/env python3
"""Copy patched build, add private counters, compare geometry outside timing."""
import argparse
import gzip
import json
from pathlib import Path
import shutil
import subprocess
from build import digest
from run import supervise, append
from workloads import cases, manifest
import recovery

HERE = Path(__file__).resolve().parent


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('patched', type=Path)
    p.add_argument('build', type=Path)
    p.add_argument('stage', type=Path)
    p.add_argument('output', type=Path)
    p.add_argument('--resume-from', type=Path, help='Import verified prior evidence into a new output directory')
    p.add_argument('--identity-map', type=Path, help='JSON mapping lost source paths to restored hash-matched paths')
    p.add_argument('--control', type=Path, help='Restored upstream source root')
    p.add_argument('--recovery-manifest', type=Path, help='Verified rebuilt prerequisite identities')
    args = p.parse_args()
    if not args.resume_from:
        assert not args.stage.exists() and not args.output.exists()
    if args.resume_from:
        relocations = json.loads(args.identity_map.read_text()) if args.identity_map else {}
        recovery.import_prior(args.resume_from, args.output, relocations)
    build_record = json.loads((args.build/'build.json').read_text())
    baseline = build_record['boundaryBuild']
    for name, expected in build_record['executables'].items():
        assert digest(args.build/name) == expected, name
    control_identities = baseline['controlAfter']
    if args.recovery_manifest:
        restored = json.loads(args.recovery_manifest.read_text())
        assert restored['base'] == baseline['base']
        assert restored['dependencies'] == baseline['dependencies']
        assert restored['baselineBuildSha256'] == digest(args.build/'build.json')
        control_identities = restored['controlIdentities']
    for name, expected in baseline['patchedSources'].items():
        assert digest(args.patched/name) == expected
    src = args.stage/'modules/skparagraph/src'
    changes = [('ParagraphImpl.h',), ('ParagraphImpl.cpp',), ('TextWrapper.cpp',)]
    if not args.resume_from:
        shutil.copytree(args.patched, args.stage, symlinks=True)
        args.output.mkdir(parents=True)
        src = args.stage/'modules/skparagraph/src'
        changes = [('ParagraphImpl.h', 'size_t experimentalShapeGeneration() const',
                    'mutable size_t perfCandidates = 0, perfFragments = 0;\n    size_t experimentalShapeGeneration() const'),
                   ('ParagraphImpl.cpp', 'ParagraphImpl::experimentalVisualSlices(TextRange range) const {',
                    'ParagraphImpl::experimentalVisualSlices(TextRange range) const {\n    ++perfFragments;'),
                   ('TextWrapper.cpp', 'for (auto candidate = lineStart; candidate < end; ++candidate) {',
                    'for (auto candidate = lineStart; candidate < end; ++candidate) {\n                ++parent->perfCandidates;')]
        for name, before, after in changes:
            f = src/name
            text = f.read_text()
            assert text.count(before) == 1
            f.write_text(text.replace(before, after))
        patch = subprocess.check_output(['git', 'diff'], cwd=args.stage)
        (args.output/'instrumented.patch').write_bytes(patch)
        subprocess.run(['ninja', '-C', str(args.stage/'out/comparison'), '-j2', 'skparagraph'], check=True, pass_fds=recovery.LOCK_FDS)
        subprocess.run(['cmake', '-S', str(HERE), '-B', str(args.output/'build'), '-G', 'Ninja',
                        '-DCMAKE_BUILD_TYPE=Release', '-DSKIA='+str(args.stage.resolve()), '-DPATCHED=ON', '-DCOUNTERS=ON'], check=True, pass_fds=recovery.LOCK_FDS)
        subprocess.run(['cmake', '--build', str(args.output/'build'), '--target', 'perf-cpu', 'perf-geometry', '-j2'], check=True, pass_fds=recovery.LOCK_FDS)
        geometry_rows = []
        for label, exe in [('instrumented', args.output/'build/perf-geometry'), ('headline', args.build/'patched/perf-geometry')]:
            out = args.output/(label+'-geometry.jsonl')
            result = supervise([exe, HERE.parent/'boundary-experiment/minimal.json', '--enabled'],
                               out, args.output/(label+'-geometry.stderr'))
            assert result['status'] == 'completed', result
            geometry_rows.append(out.read_bytes())
        assert geometry_rows[0] == geometry_rows[1], 'full geometry/source mapping differs'
        (args.output/'full-geometry-equivalence.json').write_text(json.dumps(dict(equal=True,
            manifestSha256=digest(HERE.parent/'boundary-experiment/minimal.json')))+'\n')
        gates = args.output/'gates'
        for label, exe in [('control', args.build/'upstream/perf-geometry'), ('patched', args.output/'build/perf-geometry')]:
            (gates/label).mkdir(parents=True)
            (gates/label/'fragment-preflight').symlink_to(exe.resolve())
        subprocess.run(['python3', str(HERE.parent/'boundary-experiment/verify.py'), str(gates),
                        str(args.output/'full-geometry-gates')], check=True, pass_fds=recovery.LOCK_FDS)
        subprocess.run(['python3', str(HERE.parent/'boundary-experiment/audit.py'),
                        str(args.output/'full-geometry-gates'), str(args.output/'extent-audit.json')], check=True, pass_fds=recovery.LOCK_FDS)
        (args.output/'resume-identities.json').write_text(json.dumps({str(f):digest(f) for f in [args.output/'build/perf-cpu', args.output/'build/perf-geometry', *[src/x[0] for x in changes]]}, indent=2)+'\n')
    skipped = set()
    prior = [json.loads(line) for line in (args.output/'index.jsonl').read_text().splitlines()] if args.resume_from else []
    existing = {(r['case'], r['variant'], r.get('label')):r for r in prior}
    complete = {(r['case'], r['variant']) for r in map(json.loads, (args.output/'equivalence.jsonl').read_text().splitlines())} if args.resume_from else set()
    for case in cases():
        f = args.output/(case['id']+'.json')
        fixture = manifest(case)
        if f.exists():
            assert json.loads(f.read_text()) == fixture, 'fixture changed'
        else:
            f.write_text(json.dumps(fixture, ensure_ascii=False)+'\n')
        for variant in ('zero', 'padded'):
            key = (case['series'], variant)
            previous = [r for r in prior if r['case'] == case['id'] and r['variant'] == variant]
            if any(r['status'] != 'completed' for r in previous):
                skipped.add(key)
                continue
            if (case['id'], variant) in complete:
                continue
            if key in skipped:
                append(args.output/'index.jsonl', dict(case=case['id'], variant=variant, status='skipped-larger-in-failed-series'))
                continue
            rows = []
            for label, exe in [('instrumented', args.output/'build/perf-cpu'), ('headline', args.build/'patched/perf-cpu')]:
                name = f"{case['id']}-{variant}-{label}"
                out = args.output/(name+'.jsonl')
                old = existing.get((case['id'], variant, label))
                if old and old['status'] == 'completed':
                    rows.append(json.loads(out.read_text()))
                    continue
                result = supervise([exe, f, variant, 'diagnostic'], out, args.output/(name+'.stderr'))
                if result['status'] == 'completed':
                    json.loads(out.read_text())['geometry']
                append(args.output/'index.jsonl', dict(case=case['id'], variant=variant, label=label, raw=out.name, rawSha256=digest(out), fixtureSha256=digest(f), **result))
                if result['status'] != 'completed':
                    skipped.add(key)
                    break
                rows.append(json.loads(out.read_text()))
            if len(rows) == 2:
                equal = rows[0]['geometry'] == rows[1]['geometry']
                assert equal, (case['id'], variant)
                append(args.output/'equivalence.jsonl', dict(case=case['id'], variant=variant, equal=equal, counters=rows[0]['counters']))
    # Upstream has no boundary generation counter. A separate diagnostic-only
    # global counts its shape entry point without changing private object layout.
    recovery.checkpoint(args.output, 'patched-comparisons', [args.output/'equivalence.jsonl'])
    control = args.control or Path(build_record['roots']['control'])
    control_stage = args.stage.with_name(args.stage.name+'-control')
    impl = control_stage/'modules/skparagraph/src/ParagraphImpl.cpp'
    if not recovery.phase_complete(args.output, 'control-build'):
        assert not control_stage.exists()
        if (args.output/'control-build').exists():
            (args.output/'control-build').rename(args.output/'interrupted-control-build')
        for name, expected in control_identities.items():
            assert digest(control/name) == expected
        shutil.copytree(control, control_stage, symlinks=True)
        text = impl.read_text()
        marker = 'bool ParagraphImpl::shapeTextIntoEndlessLine() {'
        assert text.count(marker) == 1
        impl.write_text('#include <cstddef>\nextern "C" { std::size_t perfControlShapeCount = 0; }\n'+
                        text.replace(marker, marker+'\n    ++::perfControlShapeCount;'))
        (args.output/'control-instrumented.patch').write_bytes(subprocess.check_output(['git', 'diff'], cwd=control_stage))
        subprocess.run(['ninja', '-C', str(control_stage/'out/comparison'), '-j2', 'skparagraph'], check=True, pass_fds=recovery.LOCK_FDS)
        subprocess.run(['cmake', '-S', str(HERE), '-B', str(args.output/'control-build'), '-G', 'Ninja',
                        '-DCMAKE_BUILD_TYPE=Release', '-DSKIA='+str(control_stage.resolve()), '-DCONTROL_COUNTERS=ON'], check=True, pass_fds=recovery.LOCK_FDS)
        subprocess.run(['cmake', '--build', str(args.output/'control-build'), '--target', 'perf-cpu', 'perf-geometry', '-j2'], check=True, pass_fds=recovery.LOCK_FDS)
        recovery.durable_json(args.output/'control-build-identities.json', dict(source=digest(impl), libraries={f.name:digest(f) for f in (control_stage/'out/comparison').glob('*.a')}))
        recovery.checkpoint(args.output, 'control-build', [args.output/'control-build/perf-cpu', args.output/'control-build/perf-geometry', args.output/'control-instrumented.patch', args.output/'control-build-identities.json'])
    if not recovery.phase_complete(args.output, 'control-correctness'):
        checked = []
        for name in ('regression-control', 'control-disabled'):
            fixture = args.output/'full-geometry-gates'/(name+'.json')
            raw = args.output/('recovered-'+name+'.jsonl')
            if raw.exists():
                interrupted = args.output/'interrupted'
                interrupted.mkdir(exist_ok=True)
                raw.rename(interrupted/raw.name)
            result = supervise([args.output/'control-build/perf-geometry', fixture, '--disabled'], raw,
                               raw.with_suffix('.stderr'))
            assert result['status'] == 'completed', result
            old = gzip.decompress((args.output/'full-geometry-gates'/(name+'.jsonl.gz')).read_bytes())
            assert [json.loads(x) for x in raw.read_text().splitlines()] == [json.loads(x) for x in old.splitlines()]
            checked.append(raw)
        recovery.checkpoint(args.output, 'control-correctness', checked)
    for case in cases():
        if not (case['size'] == 1024 or (case['series'].startswith('document-') and case['size'] == 10)):
            continue
        fixture = args.output/(case['id']+'.json')
        rows = []
        for label, exe in [('instrumented', args.output/'control-build/perf-cpu'), ('headline', args.build/'upstream/perf-cpu')]:
            name = case['id']+'-upstream-'+label
            out = args.output/(name+'.jsonl')
            old = existing.get((case['id'], 'upstream', label))
            if old:
                assert old['status'] == 'completed', old
                rows.append(json.loads(out.read_text()))
                continue
            result = supervise([exe, fixture, 'upstream', 'diagnostic'], out, args.output/(name+'.stderr'))
            if result['status'] == 'completed':
                json.loads(out.read_text())['geometry']
            append(args.output/'index.jsonl', dict(case=case['id'], variant='upstream', label=label, raw=out.name, rawSha256=digest(out), fixtureSha256=digest(fixture), **result))
            assert result['status'] == 'completed', result
            rows.append(json.loads(out.read_text()))
        equal = rows[0]['geometry'] == rows[1]['geometry']
        assert equal and rows[0]['shapingReused']
        if not any(r['case'] == case['id'] for r in recovery.rows(args.output/'control-equivalence.jsonl')):
            append(args.output/'control-equivalence.jsonl', dict(case=case['id'], equal=equal,
            shapingReused=rows[0]['shapingReused'], counters=rows[0]['counters']))
        assert equal and rows[0]['shapingReused']
    recovery.checkpoint(args.output, 'upstream-comparisons', [args.output/'control-equivalence.jsonl'])
    comparisons = []
    for case in cases():
        geometries = {}
        for variant in ('upstream', 'zero', 'padded'):
            path = args.output/(case['id']+'-'+variant+'-headline.jsonl')
            if path.exists() and path.stat().st_size:
                try:
                    geometries[variant] = json.loads(path.read_text())['geometry']
                except json.JSONDecodeError:
                    pass
        for a, b in (('upstream', 'zero'), ('zero', 'padded')):
            if a not in geometries or b not in geometries:
                continue
            ga, gb = geometries[a], geometries[b]
            mapping = all([[p[key] for p in ga] == [p[key] for p in gb]
                           for key in ('sourceToBytes', 'sourceToUnits', 'layoutToSource')])
            assert mapping, 'variant changed source mapping'
            comparisons.append(dict(case=case['id'], variants=[a,b], sourceMappingEqual=mapping,
                glyphRunsEqual=[p['glyphs'] for p in ga] == [p['glyphs'] for p in gb],
                lineGeometryEqual=[p['lines'] for p in ga] == [p['lines'] for p in gb],
                lineCounts=[sum(len(p['lines']) for p in g) for g in (ga,gb)]))
    (args.output/'variant-geometry.json').write_text(json.dumps(comparisons, indent=2)+'\n')
    if not impl.exists():
        # Later-phase recovery reuses a checked executable and its source patch.
        assert recovery.phase_complete(args.output, 'control-build')
    (args.output/'control-identities.json').write_text(json.dumps(dict(
        source=json.loads((args.output/'control-build-identities.json').read_text())['source'], sourcePatch=digest(args.output/'control-instrumented.patch'), executable=digest(args.output/'control-build/perf-cpu'),
        libraries={f.name:digest(f) for f in (control_stage/'out/comparison').glob('*.a')}), indent=2)+'\n')
    for name, expected in control_identities.items():
        assert digest(control/name) == expected
    if not args.recovery_manifest:
        for name, expected in baseline['libraries'].items():
            assert digest(args.patched/name) == expected, 'original patched build changed'
    (args.output/'identities.json').write_text(json.dumps(dict(
        source={name:digest(src/name) for name,*_ in changes},
        libraries={f.name:digest(f) for f in (args.stage/'out/comparison').glob('*.a')},
        baselineBuildSha256=digest(args.build/'build.json'),
        executable=digest(args.output/'build/perf-cpu'), geometryExecutable=digest(args.output/'build/perf-geometry')), indent=2)+'\n')

    recovery.checkpoint(args.output, 'finished', [args.output/'variant-geometry.json', args.output/'identities.json', args.output/'control-identities.json', args.output/'index.jsonl'])


if __name__ == '__main__':
    with recovery.execution_lock(HERE.parents[3]/'build-ci/073-performance-recovery/execution.lock'):
        main()
