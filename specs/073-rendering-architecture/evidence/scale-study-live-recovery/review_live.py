"""Offline checkpoint/fresh mapping comparison; never launches a viewer."""
import json
from pathlib import Path
import re
import sys

run=Path(sys.argv[1]);output=Path(sys.argv[2])
load=lambda p:json.loads(p.read_text())
schedule=load(run/'schedule.json');states=[];missing=[]
fresh_run=Path(load(run/'freeze.json')['priorStudy'])
fresh_schedule=load(fresh_run/'schedule.json')

def destination(folder):
    trace=(folder/'wayland.log').read_text()
    enters=re.findall(r'wl_surface[@#](\d+)\.enter\(wl_output[@#](\d+)\)',trace)
    surfaces={s for s,o in enters};assert len(surfaces)==1
    surface=surfaces.pop()
    links=[v for v,s in re.findall(r'get_viewport\(new id wp_viewport[@#](\d+), wl_surface[@#](\d+)\)',trace) if s==surface]
    assert len(links)==1
    viewport=links[0]
    values=[list(map(int,v)) for v in re.findall(r'wp_viewport[@#]'+viewport+r'\.set_destination\((\d+), (\d+)\)',trace)]
    assert values
    return values, [int(s) for s in re.findall(r'preferred_scale\((\d+)\)',trace)]

for cell in (c for c in schedule if c['kind']=='live'):
    folder=run/'attempts'/cell['id'];destinations,preferences=destination(folder)
    assert all(d==destinations[0] for d in destinations), 'review changing live viewport destinations separately'
    initial=load(folder/'live-0.json')
    for index,scale in enumerate([2,1.5,1.25,1,1.25,1.5,2]):
        path=folder/f'live-{index}.json'
        if not path.exists():
            missing.append(dict(id=cell['id'],checkpoint=index,desktopScale=scale,status='untested'))
            continue
        checkpoint=load(path);mapping=checkpoint['mapping'];check=checkpoint['check']
        fresh=next(c for c in fresh_schedule if c['kind']=='fresh' and c['scale']==scale and c['config']==cell['config'] and c['fixture']=='bidi-objects' and c['cycle']==0)
        fresh_folder=fresh_run/'attempts'/fresh['id'];fresh_destinations,_=destination(fresh_folder)
        fresh_records=[json.loads(line) for line in (fresh_folder/'stdout.jsonl').read_text().splitlines()]
        fresh_mapping=next(r for r in fresh_records if r['stage']=='mapping' and r['reason']=='ready')
        font=mapping['nominalFontSize']*destinations[0][1]*scale/mapping['window'][1]
        fresh_font=fresh_mapping['nominalFontSize']*fresh_destinations[0][1]*scale/fresh_mapping['window'][1]
        states.append(dict(id=cell['id'],configuration=cell['config']['id'],checkpoint=index,desktopScale=scale,
            sameWindowId=mapping['windowId']==initial['mapping']['windowId'],
            sourcePreserved=check['forward']==check['reverse']==initial['check']['forward'],
            inverse=check['inverse'],contained=check['contained'],repeatPassed=checkpoint.get('repeatPassed'),
            window=mapping['window'],dpr=mapping.get('dpr'),ratio=mapping.get('ratio'),
            surface=destinations[0],fractionalPreferences120ths=preferences,
            nominalFontPhysicalPixels=font,freshNominalFontPhysicalPixels=fresh_font,
            matchesFreshPhysicalFontSize=font==fresh_font))
assert len(states)+len(missing)==70
result=dict(checkpoints=len(states),missingCheckpoints=missing,stableWindowIdentities=all(r['sameWindowId'] for r in states),
    sourceAndCoordinateChecks=all(r['sourcePreserved'] and r['inverse'] and r['contained'] for r in states),
    repeatedChecks=sum(r['repeatPassed'] is not None for r in states),
    repeatedPasses=sum(r['repeatPassed'] is True for r in states),
    freshPhysicalSizeMismatches=sum(not r['matchesFreshPhysicalFontSize'] for r in states),
    basis='Physical nominal font height inferred from verified desktop scale and recorded viewport/window mapping; no compositor image-quality or sampling-kernel claim.',states=states)
with output.open('x') as f:json.dump(result,f,indent=2);f.write('\n')
print({k:v for k,v in result.items() if k!='states'})
