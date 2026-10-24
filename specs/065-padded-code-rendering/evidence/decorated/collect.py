"""Collect full structured evidence and representative captures without replacing history."""
from pathlib import Path
import gzip, json, hashlib, shutil, subprocess
repo=Path.cwd(); work=repo/'build-ci/065-decorated'; dest=repo/'specs/065-padded-code-rendering/evidence/decorated'
dest.mkdir(parents=True,exist_ok=False)
def copytree(source,target):
    target.mkdir(parents=True,exist_ok=True)
    for file in source.rglob('*'):
        if not file.is_file(): continue
        rel=file.relative_to(source)
        if file.suffix=='.png':
            # Keep every fixture's representative forward captures at each DPR;
            # retain failing/sample reverse controls via metrics, not duplicate PNGs.
            if not any(part.endswith('-forward') for part in rel.parts): continue
        out=target/rel
        if out.name.startswith('legacy-') and out.name.endswith('.jsonl.gz'): out=out.with_name(out.name.replace('.jsonl.gz','.txt.gz'))
        out.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(file,out)
copytree(work/'results',dest/'final')
for name in ['attempt-1','attempt-2']: copytree(work/name,dest/name)
for file in work.glob('*.log'):
    with gzip.open(dest/(file.name+'.gz'),'wb') as stream:stream.write(file.read_bytes())
for name in ['commands.json','final-commands.json','identities.json','verify-final.py','collect.py','host-runtime-identity.json','minimum-runtime-identity.json']:
    shutil.copy2(work/name,dest/name)
summary={};audit={}
for qt in ['6.8.0','6.12.0']:
    path=dest/'final'/('qt-'+qt)/'software'; report=json.loads((path/'cpu-summary.json').read_text())
    assert report['complete'] and len(report['records'])==8
    rows={}; by_dpr={}
    for record in report['records']:
        records=[json.loads(line) for line in gzip.open(path/(record['name']+'.jsonl.gz'))]
        cases=[r for r in records if r['type']=='case']; assert len(cases)==720
        assert len({c['coverage']['fixture'] for c in cases})==45
        assert all(c['coverage']['passed'] and c['componentEmissions']==[c['componentCount']]*3 for c in cases)
        for c in cases:
            assert [r['ordinal'] for r in c['components']]==list(range(c['componentCount']))
            assert c['glyphCount']==sum(len(r['glyphs']) for r in c['components'] if r['component']=='run')
            assert not c['croppedInk'] and not c['borderInk']
        rows[record['name']]={c['id']:c for c in cases}
        key=record['name'].rsplit('-',1)[0]
        if record['name'].endswith('reverse'):
            forward=rows[key+'-forward'];reverse=rows[key+'-reverse']
            assert forward==reverse, 'Order changes prepared metadata or pixels'
        dpr=cases[0]['dpr']; by_dpr.setdefault(str(dpr),[]).append(dict(cases=720,failures=sum(c['failed'] for c in cases),
            changed={key:sum(c[key]['changed'] for c in cases) for key in ['shippingFull','shippingTile','shippingCrop','tileCrop']},
            maxChannelDelta={key:max(c[key]['maxChannelDelta'] for c in cases) for key in ['shippingFull','shippingTile','shippingCrop','tileCrop']},
            fixtures=sorted({c['coverage']['fixture'] for c in cases if c['failed']})))
    summary[qt]=dict(complete=True,failed=report['failed'],cases=5760,failures=sum(r['failures'] for r in report['records']),dprs=by_dpr)
    audit[qt]=dict(cases=5760,fixtures=45,coverageFailures=0,orderEqual=True,emissionCounts=True,componentOrder=True,sourceIndexes=True,croppedInk=0,borderInk=0)
(dest/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
(dest/'audit.json').write_text(json.dumps(audit,indent=2)+'\n')
manifest={str(p.relative_to(dest)):hashlib.sha256(p.read_bytes()).hexdigest() for p in dest.rglob('*') if p.is_file()}
(dest/'sha256.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(json.dumps(summary,indent=2))
