#!/usr/bin/env python3
"""Build exact Qt controls and SDL3 candidate; retain offline layout checks."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
from prepare import ROOT,HERE,generate
from diagnose import sha,save

def build(output):
    output.mkdir(parents=True,exist_ok=False)
    baseline=ROOT/'build-ci/073-scale-study-final'
    identities=json.loads((baseline/'identities.json').read_text())
    for name,expected in identities.items():
        if sha(Path(name))!=expected:raise ValueError('historical input changed: '+name)
    generate(output/'source')
    shutil.copyfile(baseline/'fixtures.json',output/'fixtures.json')
    rows=[]
    for role,version,sdk in [('minimum','6.8.0',ROOT/'build-ci/065-scenegraph/native-sdk'),
                             ('development','6.12.0',Path('/usr')),('skia',None,None)]:
        command=['cmake','-S',str(output/'source'),'-B',str(output/role),'-G','Ninja','-DCMAKE_BUILD_TYPE=Release']
        if version:
            command+=['-DSTUDY_QT=ON',f'-DEXPECTED_QT={version}',f'-DCMAKE_PREFIX_PATH={sdk}',
                f'-DPREPARED_STAGE={ROOT}/build-ci/073-correctness-prepared',
                f'-DPREPARED_LIBRARY={ROOT}/build-ci/073-demo-{role}/prepared/src/QMarkdown/libQMarkdown.so']
        else:command+=[f'-DSKIA={ROOT}/build-ci/073-phase-workspace-v3/headline']
        for label,cmd in [('configure',command),('build',['cmake','--build',str(output/role),'-j','2'])]:
            with (output/f'{role}-{label}.log').open('xb') as log:
                subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True)
        result=subprocess.run([str(output/role/'study-layout-check'),str(output/'fixtures.json')],
            env=dict(os.environ,QT_QPA_PLATFORM='offscreen'),capture_output=True,timeout=120)
        (output/f'{role}-layout.jsonl').write_bytes(result.stdout)
        (output/f'{role}-layout.stderr').write_bytes(result.stderr)
        checks=[json.loads(line) for line in result.stdout.splitlines() if json.loads(line).get('stage')=='study-check']
        failures=[r for r in checks if not r['inverse'] or not r.get('contained',True) or r['forward']!=r['reverse']]
        rows.append(dict(role=role,exit=result.returncode,checks=len(checks),failures=failures))
        if result.returncode or len(checks)!=15 or failures:raise RuntimeError('layout check failed: '+role)
    save(output/'layout-check.json',rows)
    save(output/'preflight.json',dict(historicalInputsVerified=True,fixturesUnchanged=True,
        shippingChanges=False,qtVersions=['6.8.0','6.12.0'],sdl3='3.4.18'))
    for p in output.rglob('*'):
        if p.is_file() and (p.name in ('qt-compare','skia-compare','fixtures.json','layout-check.json','preflight.json') or p.parent.name=='source'):
            identities[str(p)]=sha(p)
    save(output/'identities.json',identities)
if __name__=='__main__':build(Path(sys.argv[1]).resolve())
