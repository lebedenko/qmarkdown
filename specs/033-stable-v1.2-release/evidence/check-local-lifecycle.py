import hashlib,json,subprocess,tarfile
from pathlib import Path
b=Path('/tmp/qmarkdown-033-hosted'); archive=b/'published-assets/qmarkdown-1.2.0-ubuntu24.04-x86_64-qt6.8.tar.gz'; records={}
with tarfile.open(archive) as t:
 parent=archive.name.removesuffix('.tar.gz')
 for qt in ['6.8.0','6.12.0']:
  work=b/f'published-ubuntu-{qt}'; result=json.loads((work/'result.json').read_text()); assert result['exitCode']==0
  assert (work/'qt-version.txt').read_text().strip()==qt
  log=(work/'verification.log').read_text(); assert 'PASS: archive, installer lifecycle and installed consumers' in log
  for root in ['consumers','real-consumers']:
   for link,style,action in [('ON','OFF','OFF'),('OFF','OFF','OFF'),('ON','ON','OFF'),('OFF','ON','OFF'),('ON','OFF','ON'),('OFF','OFF','ON')]: assert '/'+root+'/'+link+'-styling-'+style+'-actions-'+action+'/qmarkdown-consumer' in log
  for m in t.getmembers():
   if m.isfile(): assert (work/'results/extracted'/m.name).read_bytes()==t.extractfile(m).read(),m.name
  records[qt]={'result':result,'environment':json.loads((work/'environment.json').read_text()),'allRegularArchiveFilesMatch':True,'stagedAndRealLifecycle':True,'sixLinkedPluginOnlyConsumersPerInstall':True,'logSha256':hashlib.sha256((work/'verification.log').read_bytes()).hexdigest()}
work=b/'published-arch-cached'; result=json.loads((work/'result.json').read_text()); assert result['exitCode']==0 and result['packageBytesUnchanged']
log=(work/'verify.log').read_text(); assert '0 altered files' in log and 'PASS: Arch metadata, payload, namcap, consumers and pacman lifecycle' in log
assert 'qmarkdown-1.2.0-2-x86_64.pkg.tar.zst' in log
for link,style,action in [('ON','OFF','OFF'),('OFF','OFF','OFF'),('ON','ON','OFF'),('OFF','ON','OFF'),('ON','OFF','ON'),('OFF','OFF','ON')]: assert 'consumer-'+link+'-'+style+'-'+action in log
package=work/'qmarkdown-1.2.0-1-x86_64.pkg.tar.zst'; info=subprocess.check_output(['bsdtar','-xOf',str(package),'.PKGINFO'],text=True); assert info==(work/'package-info.txt').read_text(); assert 'pkgver = 1.2.0-1\n' in info
buildInfo=subprocess.check_output(['bsdtar','-xOf',str(package),'.BUILDINFO'],text=True)
assert 'pkgbuild_sha256sum = '+hashlib.sha256((work/'PKGBUILD').read_bytes()).hexdigest()+'\n' in buildInfo
(work/'package-build-info.txt').write_text(buildInfo)
records['arch']={'result':result,'environment':json.loads((work/'environment.json').read_text()),'taggedSource':json.loads((work/'tagged-source.json').read_text()),'packageInfo':info,'buildInfo':buildInfo,'sixLinkedPluginOnlyConsumers':True,'metadataPayloadOwnershipIntegrityUpgradeRemoval':True,'logSha256':hashlib.sha256((work/'verify.log').read_bytes()).hexdigest()}
(b/'local-lifecycle-validation.json').write_text(json.dumps(records,indent=2)+'\n')
print('PASS: downloaded Ubuntu both exact runtimes and fresh Arch lifecycle; published bytes unchanged')
