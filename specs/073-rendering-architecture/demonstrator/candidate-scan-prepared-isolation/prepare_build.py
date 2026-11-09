"""One journaled PI2 build and read-only audits. Never launches experiment binaries."""
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import sys
import split

HERE = Path(__file__).resolve().parent
FEATURE = HERE.parents[1]
ROOT = FEATURE.parents[1]
WORK = ROOT/'build-ci/candidate-scan-prepared-isolation'
SOURCE = ROOT/'build-ci/073-candidate-continuation'


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec); spec.loader.exec_module(result)
    return result


p = module('pi_calibration', HERE.parent/'candidate-scan-prepared-calibration/run.py')
impl = module('pi_implementation', HERE.parent/'candidate-scan-implementation/implementation.py')


def command(argv, name):
    argv = list(map(str, argv))
    p.r.supervisor.append(WORK/'build-commands.jsonl', dict(command=argv, log=name, status='attempted'))
    with (WORK/name).open('w') as log:
        result = subprocess.run(argv, stdout=log, stderr=subprocess.STDOUT)
    p.require(result.returncode == 0, 'build/audit failed: '+name)


def flags(path, source):
    return [line.strip().replace(str(source), 'SKIA') for line in path.read_text().splitlines()
            if line.strip().startswith(('FLAGS =', 'DEFINES =', 'INCLUDES ='))]


def build():
    p.require(not (WORK/'build-started.json').exists(), 'build already attempted')
    p.write(WORK/'build-started.json', dict(workers=2, experimentLaunches=0))
    with p.a.no_launches():
        ids = json.loads((WORK/'preflight/verified-identities.json').read_text())
        ids.update(json.loads((WORK/'preflight/refinement/identities.json').read_text()))
        p.a.diagnosis.verify_identities(ids)
        p.write(WORK/'input-identities.json', ids)
    left = flags(SOURCE/'baseline-build/build.ninja', SOURCE/'baseline')
    right = flags(SOURCE/'retained-build/build.ninja', SOURCE/'retained')
    p.require(left == right, 'unexplained B/C driver flags')
    stage = WORK/'isolated'
    shutil.copytree(SOURCE/'retained', stage, symlinks=True)
    relative = Path('modules/skparagraph/src')
    audit = split.apply(SOURCE/'baseline'/relative/'TextWrapper.cpp',
                       SOURCE/'retained'/relative/'TextWrapper.cpp', stage/relative/'TextWrapper.cpp')
    (WORK/'isolated.patch').write_text(impl.diff(SOURCE/'retained'/relative, stage/relative))
    p.write(WORK/'source-audit.json', audit)
    shutil.copytree(stage/relative, WORK/'isolated-sources')
    command(['ninja', '-C', stage/'out/comparison', '-j2', 'skparagraph'], 'isolated-library.log')
    for path in (SOURCE/'retained/out/comparison').glob('*.a'):
        if path.name != 'libskparagraph.a':
            p.require(p.digest(path) == p.digest(stage/'out/comparison'/path.name), 'dependency rebuilt')
    for label, src in [('C', SOURCE/'retained'), ('I', stage)]:
        build_dir = WORK/(label+'-prepared-build')
        command(['cmake','-S',HERE.parent/'candidate-scan-batching-refinement','-B',build_dir,
                 '-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DSKIA='+str(src)], label+'-prepared-configure.log')
        command(['cmake','--build',build_dir,'-j2','--verbose'], label+'-prepared-build.log')
    p.require(flags(WORK/'C-prepared-build/build.ninja',SOURCE/'retained') ==
              flags(WORK/'I-prepared-build/build.ninja',stage), 'prepared flag mismatch')
    p.require(flags(WORK/'C-prepared-build/build.ninja',SOURCE/'retained') ==
              flags(p.BUILD/'build.ninja',p.SKIA), 'archived prepared flag mismatch')
    command(['cmake','-S',HERE.parent/'candidate-scan-continuation','-B',WORK/'I-original-build',
             '-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DSKIA='+str(stage)], 'I-original-configure.log')
    command(['cmake','--build',WORK/'I-original-build','-j2','--target','runtime-trace','--verbose'], 'I-original-build.log')
    p.require(flags(WORK/'I-original-build/build.ninja',stage) == left, 'original flag mismatch')
    for label, binary in [('prepared',WORK/'I-prepared-build/prepared-reflow'),
                          ('original',WORK/'I-original-build/runtime-trace')]:
        command(['objdump','-d','-C',binary], label+'-disassembly.txt')
        command(['nm','-C',binary], label+'-symbols.txt')
    # This stage deliberately stops for an explicit offline compiled-control-flow
    # audit before any diagnostic or experimental executable can be launched.
    p.write(WORK/'build-status.json',dict(status='requires-compiled-audit',experimentLaunches=0))


if __name__ == '__main__':
    try:
        build()
    except Exception as ex:
        p.write(WORK/'failure.json',dict(status='incomplete',stage='build-preflight',
                error=type(ex).__name__+': '+str(ex),executableLaunches=0))
        raise
