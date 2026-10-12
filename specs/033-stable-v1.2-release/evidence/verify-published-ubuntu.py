import json,os,subprocess,sys
from pathlib import Path
base=Path('/tmp/qmarkdown-033-hosted')
qt=sys.argv[1]
images={'6.8.0':'sha256:8353e388d4cc266b012162399672c81d8fac279dfda17d0bd06ae6a7377d31f7','6.12.0':'sha256:0e9094d808f66e6df9b70028d815649e989eaa44ba2bdb27df56dd2cc8bebd13'}
work=base/f'published-ubuntu-{qt}'; work.mkdir(exist_ok=True)
image=images[qt]
identity=json.loads(subprocess.check_output(['docker','image','inspect',image]))[0]
(work/'environment.json').write_text(json.dumps({'qt':qt,'imageId':image,'architecture':identity['Architecture'],'network':'none','sourceCommit':'bdd0cf666b98433fccb2428b86aab8b0dce5cb29','imageConfig':identity},indent=2)+'\n')
command=['docker','run','--rm','--network','none','--platform','linux/amd64','--mount',f'type=bind,src={base}/source,dst=/source,readonly','--mount',f'type=bind,src={base}/published-assets,dst=/binary,readonly','--mount',f'type=bind,src={work},dst=/checks','--env',f'CI_QT_VERSION={qt}',image,'bash','-euc','export PATH="/opt/Qt/$CI_QT_VERSION/gcc_64/bin:$PATH"; qmake -query QT_VERSION > /checks/qt-version.txt; cat /etc/os-release > /checks/os-release; dpkg-query -W > /checks/packages.txt; python3 /source/scripts/verify-release-package.py --archive /binary/qmarkdown-1.2.0-ubuntu24.04-x86_64-qt6.8.tar.gz --qt-root "/opt/Qt/$CI_QT_VERSION/gcc_64" --work-dir /checks/results --real-install']
with (work/'verification.log').open('w') as output: result=subprocess.run(command,stdout=output,stderr=subprocess.STDOUT)
(work/'result.json').write_text(json.dumps({'exitCode':result.returncode,'command':command},indent=2)+'\n')
print('Ubuntu',qt,'verification exit',result.returncode,flush=True)
sys.exit(result.returncode)
