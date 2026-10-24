from pathlib import Path
import subprocess, json, os, hashlib
repo=Path.cwd(); work=repo/'build-ci/065-decorated'; proto=repo/'specs/065-padded-code-rendering/retained-prototype'
image='qmarkdown-ci:6.8.0-amd64-7ec4b6bb915bd0a94d722292934bee25ffb63b68e0b0b07188dc86d6bdfbd961'
records=[]
def run(name,cmd):
    print('Starting',name,flush=True)
    with (work/(name+'.log')).open('w') as stream:
        code=subprocess.run(cmd,stdout=stream,stderr=subprocess.STDOUT).returncode
    records.append(dict(name=name,command=cmd,exit=code)); (work/'commands.json').write_text(json.dumps(records,indent=2)+'\n')
    print(name,code,flush=True); return code
base=['docker','run','--rm','--user',str(os.getuid())+':'+str(os.getgid()),'-v',str(repo)+':/source:ro','-v',str(work)+':/work',image]
if run('minimum-configure',base+['cmake','-S','/source/specs/065-padded-code-rendering/retained-prototype','-B','/work/minimum','-G','Ninja','-DQMARKDOWN_STAGE=/work/stage','-DCMAKE_PREFIX_PATH=/opt/Qt/6.8.0/gcc_64','-DCMAKE_BUILD_TYPE=Release']): raise SystemExit(2)
if run('minimum-build',base+['cmake','--build','/work/minimum','--target','decorated-content-gate','whole-run-gate','retained-glyph-gate','retained-glyph-diagnostic','-j2']): raise SystemExit(2)
failed=False
for qt,build in [('6.12.0','host'),('6.8.0','minimum')]:
    for stage in ['historical','basic','extended']:
        cmd=([ 'python3',str(proto/'run_whole.py'),str(work/build),str(work/'fonts'),str(work/'results/prerequisite'/('qt-'+qt)),stage] if build=='host' else base+['python3','/source/specs/065-padded-code-rendering/retained-prototype/run_whole.py','/work/minimum','/work/fonts','/work/results/prerequisite/qt-'+qt,stage])
        failed |= bool(run('prerequisite-'+qt+'-'+stage,cmd))
if failed: raise SystemExit(1)
for qt,build in [('6.12.0','host'),('6.8.0','minimum')]:
    cmd=(['python3',str(proto/'run_decorated.py'),str(work/build),str(work/'fonts'),str(work/'results'),'cpu','--qt',qt] if build=='host' else base+['python3','/source/specs/065-padded-code-rendering/retained-prototype/run_decorated.py','/work/minimum','/work/fonts','/work/results','cpu','--qt',qt])
    failed |= bool(run('cpu-'+qt,cmd))
(work/'results/progression.json').write_text(json.dumps(dict(stoppedAfter='cpu' if failed else None,nextStages=['static','scroll','lifecycle'],qualified=False),indent=2)+'\n')
raise SystemExit(1 if failed else 0)
