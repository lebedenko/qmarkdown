#!/usr/bin/env python3
"""Copy the pinned control, apply the boundary/layout patch, build CPU and native adapters."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

HERE = Path(__file__).resolve().parent


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(command, cwd=None):
    subprocess.run(list(map(str, command)), cwd=cwd, check=True)


def identities(control):
    files = [control / 'comparison-build.json', control / 'out/comparison/args.gn',
             control / 'bin/gn', *sorted((control / 'out/comparison').glob('*.a'))]
    return {str(p.relative_to(control)): digest(p) for p in files}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('control', type=Path)
    parser.add_argument('stage', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    control, stage, output = [p.resolve() for p in (args.control, args.stage, args.output)]
    pin = json.loads((HERE.parent / 'skia-pin.json').read_text())
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=control, text=True).strip()
    if revision != pin['commit']:
        parser.error('control differs from pinned revision')
    run(['git', 'diff', '--exit-code', 'HEAD', '--'], control)
    if stage.exists() or output.exists():
        parser.error('stage and output must be new paths; existing artifacts are never overwritten')
    for left, right in ((control, stage), (control, output), (stage, output), (output, stage)):
        if left == right or left in right.parents:
            parser.error('control, stage and output must be disjoint')
    before = identities(control)
    patch = HERE / 'fragment-edges.patch'
    expected = json.loads((HERE / 'patch-pin.json').read_text())
    if digest(patch) != expected['sha256']:
        parser.error('patch hash differs from reviewed pin')
    # Copy files, never hardlink: rebuilding the experiment cannot mutate control archives.
    symlinks = {str(p.relative_to(control)): str(p.readlink())
                for p in control.rglob('*') if p.is_symlink()}
    for name, target in symlinks.items():
        if Path(target).is_absolute() or not (control / name).resolve().is_relative_to(control):
            parser.error('control has an external/absolute symlink: ' + name)
    shutil.copytree(control, stage, symlinks=True)
    run(['git', 'diff', '--exit-code', 'HEAD', '--'], stage)
    output.mkdir(parents=True)
    record = dict(base=revision, patchSha256=digest(patch), controlBefore=before, symlinks=symlinks,
                  inheritedBuild=json.loads((control / 'comparison-build.json').read_text()),
                  gnArgs=(stage / 'out/comparison/args.gn').read_text(), commands=[])
    def command(argv, cwd=None):
        record['commands'].append(dict(argv=list(map(str, argv)), cwd=str(cwd) if cwd else None))
        run(argv, cwd)
    try:
        command(['git', 'apply', '--check', patch], stage)
        command(['git', 'apply', patch], stage)
        changed = subprocess.check_output(['git', 'diff', '--name-only'], cwd=stage, text=True).splitlines()
        if sorted(changed) != sorted(expected['files']):
            raise RuntimeError('staged source differs beyond the pinned patch')
        command(['git', 'diff', '--check'], stage)
        record['patchedSources'] = {name: digest(stage / name) for name in expected['files']}
        command(['bin/gn', 'gen', 'out/comparison'], stage)
        command(['ninja', '-C', 'out/comparison', '-j2', 'skia', 'skparagraph', 'skshaper', 'skunicode'], stage)
        for name, checkout, patched in (('control', control, 'OFF'), ('patched', stage, 'ON')):
            command(['cmake', '-S', HERE, '-B', output / name, '-G', 'Ninja',
                     '-DCMAKE_BUILD_TYPE=Release', '-DSKIA=' + str(checkout), '-DPATCHED=' + patched])
            command(['cmake', '--build', output / name, '-j2'])
        record['libraries'] = identities(stage)
        record['executables'] = {name + '/' + exe: digest(output / name / exe)
            for name in ('control', 'patched') for exe in ('fragment-preflight', 'historical-objects')}
        record['executables']['patched/skia-compare'] = digest(output / 'patched/skia-compare')
        record['adapterSources'] = {p.name:digest(p) for p in HERE.iterdir() if p.is_file()}
        record['tools'] = {name: subprocess.check_output([name, '--version'], text=True).splitlines()[0]
                           for name in ('cmake', 'ninja', 'c++')}
        record['systemVersions'] = subprocess.check_output(['pkg-config', '--modversion', 'freetype2',
            'harfbuzz', 'icu-uc', 'icu-i18n', 'libpng', 'fontconfig', 'zlib'], text=True).splitlines()
        record['dependencies'] = {name: subprocess.check_output(['git', 'rev-parse', 'HEAD'],
            cwd=stage / 'third_party/externals' / name, text=True).strip()
            for name in record['inheritedBuild']['dependencies']}
        record['completed'] = True
    finally:
        record['controlAfter'] = identities(control)
        record['controlUnchanged'] = before == record['controlAfter']
        (output / 'build.json').write_text(json.dumps(record, indent=2) + '\n')
    if not record['controlUnchanged']:
        raise SystemExit('control identities changed')


if __name__ == '__main__':
    main()
