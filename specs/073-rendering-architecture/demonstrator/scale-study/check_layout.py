#!/usr/bin/env python3
"""Build CPU/offscreen observers and verify every fixture without graphical launches."""
import json
import os
from pathlib import Path
import subprocess
import sys
from study import save, sha

HERE=Path(__file__).resolve().parent
build=Path(sys.argv[1]).resolve()
source=build/'source'
(source/'check.cpp').write_bytes((HERE/'check.cpp').read_bytes())
cmake=source/'CMakeLists.txt'
with cmake.open('a') as f:
    f.write('''
add_executable(study-layout-check check.cpp)
if(STUDY_QT)
set(VIEWER qt-compare)
target_compile_definitions(study-layout-check PRIVATE STUDY_QT)
else()
set(VIEWER skia-compare)
endif()
foreach(PROP INCLUDE_DIRECTORIES COMPILE_OPTIONS COMPILE_DEFINITIONS LINK_LIBRARIES)
get_target_property(VALUE ${VIEWER} ${PROP})
if(VALUE)
set_target_properties(study-layout-check PROPERTIES ${PROP} "${VALUE}")
endif()
endforeach()
if(STUDY_QT)
target_compile_definitions(study-layout-check PRIVATE STUDY_QT)
else()
target_compile_options(study-layout-check PRIVATE -ffunction-sections -fdata-sections)
target_link_options(study-layout-check PRIVATE -Wl,--gc-sections)
endif()
''')
rows=[]
for role in ('minimum','development','skia'):
    with (build/f'check-build-{role}.log').open('xb') as f:
        subprocess.run(['cmake','--build',str(build/role),'--target','study-layout-check','-j','2'],stdout=f,stderr=subprocess.STDOUT,check=True)
    run=subprocess.run([str(build/role/'study-layout-check'),str(build/'fixtures.json')],
        env=dict(os.environ,QT_QPA_PLATFORM='offscreen'),capture_output=True,timeout=120)
    (build/f'check-{role}.jsonl').write_bytes(run.stdout)
    (build/f'check-{role}.stderr').write_bytes(run.stderr)
    records=[json.loads(line) for line in run.stdout.splitlines()]
    checks=[r for r in records if r.get('stage')=='study-check']
    failures=[r['checkpoint'] for r in checks if not r['inverse'] or not r.get('contained',True) or r['forward']!=r['reverse']]
    rows.append(dict(role=role,exit=run.returncode,checks=len(checks),failures=failures))
    if run.returncode or len(checks)!=15 or failures:
        save(build/'layout-check-failure.json',rows)
        raise RuntimeError('offline observer/layout check failed: '+str(rows[-1]))
save(build/'layout-check.json',rows)
identities=json.loads((build/'identities.json').read_text())
for p in source.iterdir():
    if p.is_file():identities[str(p)]=sha(p)
identities[str(build/'layout-check.json')]=sha(build/'layout-check.json')
# Build identities are finalized before the execution freeze, not attempt evidence.
(build/'identities.json').write_text(json.dumps(identities,indent=2)+'\n')
