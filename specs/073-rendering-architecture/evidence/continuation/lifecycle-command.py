import json,os,sys
from pathlib import Path
sys.path.insert(0,str(Path('specs/073-rendering-architecture/demonstrator').resolve()))
from run import supervise,HERE
root=Path('build-ci/073-lifecycle-final')
root.mkdir(exist_ok=False)
results=[]
manifest=HERE/'fixtures.json'
fonts=list(map(Path,json.loads(manifest.read_text())['fonts'].values()))
for sdk,version in [('development','6.12.0'),('minimum','6.8.0')]:
 exe=Path('build-ci',f'073-demo-{sdk}','qt-compare').resolve()
 for api in ['opengl','vulkan']:
  for renderer in ['distance','curve']:
   for cycle in range(3):
    folder=root/f'{sdk}-{api}-{renderer}-{cycle}'
    command=[str(exe),str(manifest),'document',renderer,str(folder.resolve())]
    results.append(supervise(command,folder,dict(os.environ,QT_QPA_PLATFORM='wayland',QSG_RHI_BACKEND=api,QT_SCALE_FACTOR='0.8'),
                             dict(qt=version,api={'opengl':3,'vulkan':5}[api],dpr=1,platform='wayland'),
                             [exe,manifest,HERE/'qt.cpp',*fonts]))
# Exercise real application write errors, including before the event loop.
exe=Path('build-ci/073-demo-development/qt-compare').resolve()
for name,destination in [('capture-write','/proc/1'),('before-loop-write','/dev/null/impossible')]:
 folder=root/name
 command=[str(exe),str(manifest),'padded','curve',destination]
 results.append(supervise(command,folder,dict(os.environ,QT_QPA_PLATFORM='wayland',QSG_RHI_BACKEND='opengl',QT_SCALE_FACTOR='0.8'),
                          {},[exe,manifest,*fonts]))
(root/'summary.json').write_text(json.dumps(results,indent=2)+'\n')
print([(r['exit'],r['lastStage']) for r in results])
