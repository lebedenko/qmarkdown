#!/usr/bin/env python3
"""Build private adapters from hash-verified existing pinned boundary builds."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import shutil
import tarfile

HERE = Path(__file__).resolve().parent


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('control', type=Path)
    p.add_argument('patched', type=Path)
    p.add_argument('boundary_build', type=Path)
    p.add_argument('output', type=Path)
    args = p.parse_args()
    record = json.loads((args.boundary_build/'build.json').read_text())
    assert record['completed'] and record['controlUnchanged']
    subprocess.run(['git', 'diff', '--exit-code', 'HEAD', '--'], cwd=args.control, check=True)
    for root in (args.control, args.patched):
        assert subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip() == record['base']
    changed = subprocess.check_output(['git', 'diff', '--name-only'], cwd=args.patched, text=True).splitlines()
    assert sorted(changed) == sorted(record['patchedSources'])
    for name in ('layout.h', 'native.cpp', 'main.cpp'):
        assert digest(HERE.parent/'boundary-experiment'/name) == record['adapterSources'][name]
    assert digest(HERE.parent/'boundary-experiment/fragment-edges.patch') == record['patchSha256']
    for root, identities in [(args.control, record['controlAfter']), (args.patched, record['libraries'])]:
        for name, expected in identities.items():
            assert digest(root/name) == expected, (root, name)
    for name, expected in record['patchedSources'].items():
        assert digest(args.patched/name) == expected, name
    assert not args.output.exists(), 'output must be new'
    args.output.mkdir(parents=True)
    result = dict(roots=dict(control=str(args.control.resolve()), patched=str(args.patched.resolve())), boundaryBuild=record, sources={}, executables={}, commands=[])
    for variant, root in [('upstream', args.control), ('patched', args.patched)]:
        commands = [['cmake', '-S', str(HERE), '-B', str(args.output/variant), '-G', 'Ninja',
                     '-DCMAKE_BUILD_TYPE=Release', '-DSKIA='+str(root.resolve()),
                     '-DPATCHED='+('OFF' if variant == 'upstream' else 'ON')],
                    ['cmake', '--build', str(args.output/variant), '-j2']]
        for cmd in commands:
            result['commands'].append(cmd)
            subprocess.run(cmd, check=True)
        for exe in ('perf-cpu', 'perf-native', 'perf-geometry'):
            result['executables'][variant+'/'+exe] = digest(args.output/variant/exe)
    result['tools'] = {name:subprocess.check_output([name, '--version'], text=True, stderr=subprocess.STDOUT).splitlines()[0]
                       for name in ('cmake', 'ninja', 'c++', 'wayland-scanner')}
    result['systemVersions'] = {name:subprocess.check_output(['pkg-config', '--modversion', name], text=True).strip()
        for name in ('sdl2', 'wayland-client', 'wayland-protocols', 'freetype2', 'harfbuzz',
                     'icu-uc', 'icu-i18n', 'libpng', 'fontconfig', 'zlib', 'vulkan', 'gl')}
    if shutil.which('pacman'):
        packages = subprocess.run(['pacman', '-Q', 'sdl2-compat', 'sdl3'], text=True, capture_output=True)
        result['sdlPackages'] = dict(returncode=packages.returncode, stdout=packages.stdout, stderr=packages.stderr)
    result['protocolXml'] = {str(path):digest(path) for path in (
        Path('/usr/share/wayland/wayland.xml'),
        Path('/usr/share/wayland-protocols/stable/presentation-time/presentation-time.xml'),
        Path('/usr/share/wayland-protocols/stable/viewporter/viewporter.xml'),
        Path('/usr/share/wayland-protocols/staging/fractional-scale/fractional-scale-v1.xml'))}
    result['sources'] = {str(f.relative_to(HERE.parent)):digest(f) for f in
                         [*HERE.glob('*.*'), HERE.parent/'skia/main.cpp', HERE.parent/'fixtures.json', *(HERE.parent/'boundary-experiment'/name for name in ('layout.h', 'native.cpp', 'main.cpp'))] if f.is_file()}
    with tarfile.open(args.output/'adapter-sources.tar.gz', 'w:gz') as archive:
        for name in result['sources']:
            archive.add(HERE.parent/name, arcname=name)
    result['generatedSources'] = {str(f.relative_to(args.output)):digest(f)
        for variant in ('upstream', 'patched') for f in (args.output/variant).iterdir()
        if f.suffix in ('.cpp', '.c', '.h')}
    result['inputsUnchanged'] = all(digest(root/name) == expected
        for root, identities in ((args.control, record['controlAfter']), (args.patched, record['libraries']))
        for name, expected in identities.items())
    assert result['inputsUnchanged']
    (args.output/'build.json').write_text(json.dumps(result, indent=2)+'\n')


if __name__ == '__main__':
    main()
