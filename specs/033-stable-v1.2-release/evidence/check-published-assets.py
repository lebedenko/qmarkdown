import hashlib,json,subprocess,tarfile
from pathlib import Path
base=Path('/tmp/qmarkdown-033-hosted'); sha='bdd0cf666b98433fccb2428b86aab8b0dce5cb29'
d=json.loads((base/'release-after.json').read_text()); latest=json.loads((base/'latest-after.json').read_text())
assert d['id']==latest['id'] and d['tag_name']=='v1.2.0' and not d['draft'] and not d['prerelease']
assert d['body'].strip()==(base/'source/docs/release-notes-1.2.0.md').read_text().strip()
assert d['published_at'].startswith('2026-10-12')
assets={a['name']:a for a in d['assets']}; expected={}
for directory,suffix in [('release-6.8.0','.tar.gz'),('release-arch','.pkg.tar.zst')]:
 package=next((base/'publication'/directory).glob('*'+suffix))
 for p in [package,package.with_name(package.name+'.sha256')]: expected[p.name]=p
assert set(assets)==set(expected)
records={}
for name,artifact in expected.items():
 p=base/'published-assets'/name; digest=hashlib.sha256(p.read_bytes()).hexdigest()
 assert p.read_bytes()==artifact.read_bytes(),name
 a=assets[name]; assert a['size']==p.stat().st_size and a['digest']=='sha256:'+digest
 records[name]={'asset':a,'sha256':digest,'bytes':p.stat().st_size,'publicationArtifactEqual':True}
for name in expected:
 if name.endswith('.sha256'):
  p=base/'published-assets'/name; digest,filename=p.read_text().split(); assert filename==name.removesuffix('.sha256'); assert digest==records[filename]['sha256']
archive=base/'published-assets/qmarkdown-1.2.0-ubuntu24.04-x86_64-qt6.8.tar.gz'
with tarfile.open(archive) as t:
 parent=archive.name.removesuffix('.tar.gz'); meta=json.load(t.extractfile(parent+'/metadata.json')); assert meta['version']=='1.2.0' and meta['sourceCommit']==sha and meta['qtVersion']=='6.8.0'
(base/'published-assets-validation.json').write_text(json.dumps({'release':d,'latest':latest,'assets':records,'ubuntuMetadata':meta,'assertions':['all four assets match publication artifacts','GitHub sizes/digests match downloaded bytes','both companion checksums match','release stable/latest and body/date correct','Ubuntu source identity matches fixed tag']},indent=2)+'\n')
print('PASS: four published assets, checksums, GitHub identities and publication-artifact equality')
