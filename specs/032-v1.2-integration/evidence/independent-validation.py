import hashlib,json,tarfile,subprocess,sys,re
from pathlib import Path
phase,sha=sys.argv[1:]; base=Path('/tmp/qmarkdown-032-hosted'); root=base/phase
records={}
for workflow in ['ci','release']:
 d=json.loads((base/f'{phase}-{workflow}.json').read_text()); assert d['headSha']==sha and d['conclusion']=='success'
 assert all(j['conclusion']=='success' or (j['name']=='upload' and j['conclusion']=='skipped') for j in d['jobs'])
 if workflow=='release': assert next(j for j in d['jobs'] if j['name']=='upload')['conclusion']=='skipped'
 records[workflow]=d
 if workflow=='ci':
  text=(base/f'{phase}-ci.log').read_text()
  for job in ['packaging (minimum)','packaging (development)']:
   lines='\n'.join(line for line in text.splitlines() if line.startswith(job+'\t'))
   assert lines.count('100% tests passed, 0 tests failed out of 12')>=3
   for marker in ['Built target all_qmllint','bundled external definitions prefixed','actions-shared-ON/qmarkdown-consumer','actions-shared-OFF/qmarkdown-consumer','actions-static-ON/qmarkdown-consumer','PASS: Ubuntu container packaging and benchmark smoke','PASS: static package request 1.2.0, exact=True, accepted=True']:
    assert marker in lines,(job,marker)
packages={}
for label,folder,suffix in [('ubuntu','release-6.8.0','.tar.gz'),('arch','release-arch','.pkg.tar.zst')]:
 paths=list((root/folder).glob('*'+suffix)); assert len(paths)==1,paths
 p=paths[0]; expected=p.with_name(p.name+'.sha256').read_text().split()[0]; digest=hashlib.sha256(p.read_bytes()).hexdigest(); assert digest==expected
 packages[label]={'name':p.name,'sha256':digest,'bytes':p.stat().st_size}
 if label=='ubuntu':
  with tarfile.open(p) as t:
   parent=p.name.removesuffix('.tar.gz'); metadata=json.load(t.extractfile(parent+'/metadata.json')); manifest=json.load(t.extractfile(parent+'/manifest.json'))
   assert metadata['sourceCommit']==sha and metadata['version']=='1.2.0' and metadata['qtVersion']=='6.8.0',metadata
   for name,v in manifest.items():
    m=t.getmember(parent+'/payload/'+name)
    if v['kind']=='file': assert hashlib.sha256(t.extractfile(m).read()).hexdigest()==v['sha256'],name
    else: assert m.issym() and m.linkname==v['target']
   for qt in ['6.8.0','6.12.0']:
    matches=list((root/f'binary-check-{qt}').rglob('metadata.json'))
    found=[m.parent for m in matches if m.parent.name==parent]; assert len(found)==1,found
    for m in t.getmembers():
     if m.isfile(): assert (found[0]/Path(m.name).relative_to(parent)).read_bytes()==t.extractfile(m).read(),m.name
  packages[label].update(metadata=metadata,manifest=manifest)
 else:
  info=subprocess.check_output(['bsdtar','-xOf',str(p),'.PKGINFO'],text=True); assert 'pkgver = 1.2.0-1\n' in info
  packages[label]['packageInfo']=info
  assert info==(root/'arch-evidence/package-info.txt').read_text()
  packages[label]['manifest']=subprocess.check_output(['bsdtar','-tf',str(p)],text=True).splitlines()
  packages[label]['buildInfo']=subprocess.check_output(['bsdtar','-xOf',str(p),'.BUILDINFO'],text=True)
arch=root/'arch-evidence'; env=json.loads((arch/'environment.json').read_text()); assert env['sourceCommit']==sha and env['version']=='1.2.0'
recipe=(arch/'PKGBUILD').read_text(); assert env['sourceSha256'] in recipe
for name,digest in env['toolingSha256'].items():
 mapping={'PKGBUILD.in':'scripts/arch/PKGBUILD.in','build-arch-package.py':'scripts/build-arch-package.py','container.sh':'scripts/arch/container.sh','verify.py':'scripts/arch/verify.py'}
 data=subprocess.check_output(['git','show',sha+':'+mapping[name]],cwd='/tmp/qmarkdown-032'); assert hashlib.sha256(data).hexdigest()==digest,name
log=(arch/'verify.log').read_text(); assert '0 altered files' in log and 'PASS: Arch metadata, payload, namcap, consumers and pacman lifecycle' in log
assert 'qmarkdown-1.2.0-2-x86_64.pkg.tar.zst' in log
for link,style,action in [('ON','OFF','OFF'),('OFF','OFF','OFF'),('ON','ON','OFF'),('OFF','ON','OFF'),('ON','OFF','ON'),('OFF','OFF','ON')]: assert f'consumer-{link}-{style}-{action}' in log
images={}
for qt in ['6.8.0','6.12.0']:
 directory=root/f'binary-check-{qt}'; logs=list(directory.rglob('output.log')); assert len(logs)==1
 text=logs[0].read_text(); assert 'PASS: archive, installer lifecycle and installed consumers' in text and '/real-consumers/' in text
 assert 'QMARKDOWN_ACTIONS_CONSUMER=ON' in text and 'QMARKDOWN_REQUEST_EXACT=ON' in text
 builds=list(directory.rglob('build.json')); assert len(builds)==1; images[qt]=json.loads(builds[0].read_text()); assert images[qt]['IMAGE_OUTCOME']=='success' and images[qt]['IMAGE_ID'].startswith('sha256:')
 ci=(base/f'{phase}-ci.log').read_text(); release=(base/f'{phase}-release.log').read_text()
 role='minimum' if qt=='6.8.0' else 'development'
 for label,workflowLog in [(f'packaging ({role})',ci),(f'build ({role}, {qt})',release)]:
  lines='\n'.join(line for line in workflowLog.splitlines() if line.startswith(label+'\t'))
  assert 'IMAGE_ID: '+images[qt]['IMAGE_ID'] in lines,(label,images[qt]['IMAGE_ID'])
result={'sha':sha,'runs':records,'packages':packages,'archEnvironment':env,'runtimeImages':images,'assertions':['workflow SHA matches','all jobs pass; upload skipped','distribution hashes match checksum companions','Ubuntu manifest hashes match','identical minimum-built Ubuntu archive files on both runtimes','package versions/source identities match','Arch source recipe/helper hashes match commit','six Arch consumers and ownership/upgrade/removal lifecycle passed','Ubuntu staged/real consumers/install/removal passed on both runtimes']}
(base/f'{phase}-validation.json').write_text(json.dumps(result,indent=2)+'\n'); print(phase, 'independent validation passed:', [(k,v['sha256']) for k,v in packages.items()])
