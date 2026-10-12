import hashlib,json,shutil,subprocess,sys
from pathlib import Path
base=Path('/tmp/qmarkdown-033-hosted'); work=base/(sys.argv[1] if len(sys.argv)>1 else 'published-arch')
for name in ['qmarkdown-1.2.0-1-x86_64.pkg.tar.zst','qmarkdown-1.2.0-1-x86_64.pkg.tar.zst.sha256']: shutil.copy2(base/'published-assets'/name,work/name)
with (work/'image-pull.log').open('w') as out: subprocess.run(['docker','pull','--platform','linux/amd64','archlinux:base'],stdout=out,stderr=subprocess.STDOUT,check=True)
identity=json.loads(subprocess.check_output(['docker','image','inspect','archlinux:base']))[0]
tag=json.loads((work/'tagged-source.json').read_text()); publication=json.loads((base/'publication/arch-evidence/environment.json').read_text())
package=work/'qmarkdown-1.2.0-1-x86_64.pkg.tar.zst'; before=hashlib.sha256(package.read_bytes()).hexdigest()
(work/'environment.json').write_text(json.dumps({'sourceCommit':tag['sourceCommit'],'tagObject':tag['tagObject'],'version':'1.2.0','pkgrel':'1','publishedPackageSha256':before,'imageId':identity['Id'],'repoDigests':identity['RepoDigests'],'architecture':identity['Architecture'],'publicationBuildEnvironment':publication,'fixtureSourceArchiveSha256':tag['sourceSnapshotSha256'],'toolingSha256':tag['toolingSha256'],'publicationRecipeSha256':hashlib.sha256((work/'PKGBUILD').read_bytes()).hexdigest()},indent=2)+'\n')
cache=work/'pacman-cache'; cache.mkdir(exist_ok=True)
command=['docker','run','--rm','--platform','linux/amd64','--mount',f'type=bind,src={work},dst=/evidence','--mount',f'type=bind,src={work}/tooling,dst=/tools,readonly','--mount',f'type=bind,src={cache},dst=/var/cache/pacman/pkg','--env','PKG_VERSION=1.2.0','--env','PKG_RELEASE=1','--env',f'SOURCE_SHA256={publication["sourceSha256"]}',identity['Id'],'bash','/tools/container.sh','verify']
with (work/'verify.log').open('w') as output: result=subprocess.run(command,stdout=output,stderr=subprocess.STDOUT)
assert hashlib.sha256(package.read_bytes()).hexdigest()==before
(work/'result.json').write_text(json.dumps({'exitCode':result.returncode,'command':command,'packageBytesUnchanged':True},indent=2)+'\n')
print('Arch downloaded-package verification exit',result.returncode,flush=True)
sys.exit(result.returncode)
