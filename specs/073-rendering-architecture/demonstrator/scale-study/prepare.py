#!/usr/bin/env python3
"""Verify retained inputs and build only isolated viewer overlays with two workers."""
import hashlib
import json
from pathlib import Path
import subprocess
from instrument import generate

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
SPEC = HERE.parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write(path, data):
    with path.open('x') as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
        f.write('\n')


def prepare(output):
    output.mkdir(parents=True, exist_ok=False)
    evidence = SPEC/'evidence/boundary-experiment'
    pin = json.loads((evidence/'build.json').read_text())
    skia = ROOT/'build-ci/073-phase-workspace-v3/headline'
    verified = {}
    # The later retained build metadata records another original control build.
    # Bind exact compiled libraries and all five patched runtime files separately.
    for name, expected in {**pin['patchedSources'], **pin['libraries']}.items():
        if name == 'comparison-build.json':
            continue
        p = skia/name
        if sha(p) != expected:
            raise ValueError('Skia identity mismatch: ' + str(p))
        verified[str(p)] = expected
    metadata = json.loads((skia/'comparison-build.json').read_text())
    if metadata['commit'] != pin['base'] or metadata['dependencies'] != pin['dependencies']:
        raise ValueError('Skia provenance metadata mismatch')
    stage = ROOT/'build-ci/073-correctness-prepared'
    stage_pin = json.loads((stage/'demo-source.json').read_text())
    for name, expected in stage_pin['files'].items():
        if sha(stage/name) != expected:
            raise ValueError('Qt preparation identity mismatch: ' + name)
        verified[str(stage/name)] = expected
    data = json.loads((HERE.parent/'fixtures.json').read_text())
    data['fixtures'] += json.loads((HERE.parent/'boundary-experiment/minimal.json').read_text())['fixtures'][:2]
    data['fixtures'].sort(key=lambda f: f['id'] != 'document')
    if len(data['fixtures']) != 15 or len({f['id'] for f in data['fixtures']}) != 15:
        raise ValueError('fixture matrix changed')
    for p in map(Path, data['fonts'].values()):
        verified[str(p)] = sha(p)
    write(output/'fixtures.json', data)
    generate(output/'source')
    cmake = '''cmake_minimum_required(VERSION 3.21)
project(scale_study LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
if(STUDY_QT)
find_package(Qt6 REQUIRED COMPONENTS Quick)
if(NOT Qt6_VERSION VERSION_EQUAL EXPECTED_QT)
message(FATAL_ERROR "Exact Qt mismatch")
endif()
add_executable(qt-compare qt.cpp)
target_include_directories(qt-compare PRIVATE "${PREPARED_STAGE}/src/QMarkdown")
target_link_libraries(qt-compare PRIVATE "${PREPARED_LIBRARY}" Qt6::Quick)
else()
find_package(PkgConfig REQUIRED)
pkg_check_modules(NATIVE_DEPS REQUIRED IMPORTED_TARGET sdl2 freetype2 harfbuzz icu-uc icu-i18n libpng fontconfig zlib vulkan gl)
'''
    original = (HERE.parent/'boundary-experiment/CMakeLists.txt').read_text()
    cmake += original[original.index('# Reuse the pinned upstream swapchain'):]
    (output/'source/CMakeLists.txt').write_text(cmake)
    commands = []
    for role, version, sdk in [('minimum', '6.8.0', ROOT/'build-ci/065-scenegraph/native-sdk'),
                               ('development', '6.12.0', Path('/usr'))]:
        lib = ROOT/f'build-ci/073-demo-{role}/prepared/src/QMarkdown/libQMarkdown.so'
        verified[str(lib)] = sha(lib)
        command = ['cmake','-S',str(output/'source'),'-B',str(output/role),'-G','Ninja',
            '-DCMAKE_BUILD_TYPE=Release','-DSTUDY_QT=ON',f'-DEXPECTED_QT={version}',
            f'-DCMAKE_PREFIX_PATH={sdk}',f'-DPREPARED_STAGE={stage}',f'-DPREPARED_LIBRARY={lib}']
        commands += [command, ['cmake','--build',str(output/role),'--target','qt-compare','-j','2']]
    commands += [['cmake','-S',str(output/'source'),'-B',str(output/'skia'),'-G','Ninja',
                  '-DCMAKE_BUILD_TYPE=Release',f'-DSKIA={skia}'],
                 ['cmake','--build',str(output/'skia'),'--target','skia-compare','-j','2']]
    write(output/'preparation.json',dict(verified=verified,skiaMetadata=metadata,
        inheritedBoundaryBuild=str(evidence/'build.json'),commands=commands))
    for i, command in enumerate(commands):
        with (output/f'build-{i}.log').open('xb') as log:
            subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True)
    for p,h in verified.items():
        if sha(Path(p)) != h:
            raise ValueError('input changed during build: ' + p)
    for p in output.rglob('*'):
        if p.is_file() and ('source' in p.parts or p.name in ('qt-compare','skia-compare','fixtures.json')):
            verified[str(p)] = sha(p)
    write(output/'identities.json',verified)


if __name__ == '__main__':
    import sys
    prepare(Path(sys.argv[1]).resolve())
