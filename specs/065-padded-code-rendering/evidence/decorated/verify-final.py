from pathlib import Path
import json,subprocess,os,hashlib,shutil
repo=Path.cwd(); work=repo/'build-ci/065-decorated'; proto=repo/'specs/065-padded-code-rendering/retained-prototype'
image='qmarkdown-ci:6.8.0-amd64-7ec4b6bb915bd0a94d722292934bee25ffb63b68e0b0b07188dc86d6bdfbd961'
base=['docker','run','--rm','--user',str(os.getuid())+':'+str(os.getgid()),'-v',str(repo)+':/source:ro','-v',str(work)+':/work',image]
records=[]
def run(name,cmd):
    print('Starting',name,flush=True)
    with (work/(name+'.log')).open('w') as log: code=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT).returncode
    records.append(dict(name=name,command=cmd,exit=code)); (work/'final-commands.json').write_text(json.dumps(records,indent=2)+'\n')
    print(name,code,flush=True);return code
# Prior binaries have finished all DPR/order processes before this launcher runs.
for qt,attempt in [('6.8.0','attempt-1'),('6.12.0','attempt-2')]:
    dest=work/attempt/('qt-'+qt);dest.mkdir(exist_ok=True)
    shutil.move(str(work/'results'/('qt-'+qt)/'software'),str(dest/'software'))
if run('final-host-build',['cmake','--build',str(work/'host'),'--target','decorated-content-gate','-j2']): raise SystemExit(2)
if run('final-minimum-build',base+['cmake','--build','/work/minimum','--target','decorated-content-gate','-j2']): raise SystemExit(2)
identities=dict(sourceBase=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),sourceTree=subprocess.check_output(['git','rev-parse','HEAD^{tree}'],text=True).strip(),
 files={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in proto.iterdir() if p.is_file()},
 executables={b:hashlib.sha256((work/b/'decorated-content-gate').read_bytes()).hexdigest() for b in ['host','minimum']},
 fonts={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in (work/'fonts').iterdir() if p.is_file()},
 toolchain={'host':subprocess.check_output(['c++','--version'],text=True),'hostQt':subprocess.check_output(['qmake6','-query','QT_VERSION'],text=True),'minimumImage':image})
(work/'identities.json').write_text(json.dumps(identities,indent=2)+'\n')
failed=False
for qt,b in [('6.12.0','host'),('6.8.0','minimum')]:
    cmd= ['python3',str(proto/'run_decorated.py'),str(work/b),str(work/'fonts'),str(work/'results'),'cpu','--qt',qt] if b=='host' else base+['python3','/source/specs/065-padded-code-rendering/retained-prototype/run_decorated.py','/work/minimum','/work/fonts','/work/results','cpu','--qt',qt]
    failed |= bool(run('final-cpu-'+qt,cmd))
(work/'results/progression.json').write_text(json.dumps(dict(stoppedAfter='cpu' if failed else None,notRun=['static','scroll','lifecycle'] if failed else [],qualified=False),indent=2)+'\n')
raise SystemExit(1 if failed else 0)
