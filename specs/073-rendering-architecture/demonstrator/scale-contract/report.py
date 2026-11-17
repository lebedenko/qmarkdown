#!/usr/bin/env python3
"""Independent physical-size, raster-density, correctness and lifecycle accounting."""
import collections
import json
from pathlib import Path
import sys

def load(p):return json.loads(p.read_text())
def gates(mapping,scale):
    w,h=mapping['surface'];bw,bh=mapping['buffer'];sx,sy,_,_=mapping['forward']
    zoom=mapping['zoom'];logical=mapping['logical']
    # Convert rendered units back through the actual integer buffer/surface ratio.
    physical=(abs(sx*w/bw-zoom)<=1e-6 and abs(sy*h/bh-zoom)<=1e-6
              and logical==[w,h])
    density=(abs(bw-w*scale)<=.5 and abs(bh-h*scale)<=.5
             and abs(mapping['density']-scale)<=1e-6)
    return dict(physicalComparability=physical,rasterDensity=density)

def report(directory):
    cells=load(directory/'schedule.json');counts=collections.Counter();out=[]
    fresh={}
    for cell in cells:
        folder=directory/'attempts'/cell['id'];p=folder/'result.json'
        if not p.exists():
            out.append(dict(id=cell['id'],status='unlaunched'));counts['unlaunched']+=1;continue
        result=load(p);counts[result['status']]+=1
        rows=[json.loads(line) for line in (folder/'stdout.jsonl').read_text().splitlines()]
        mappings={r['generation']:r for r in rows if r['stage']=='scale-contract'}
        observations=[]
        for r in rows:
            if r['stage']!='capture-mapping':continue
            mapping=mappings[r['generation']];scale=cell['scale']
            if r['name'].startswith('checkpoint-live-'):
                index=int(r['name'].split('-')[-1]);scale=[2,1.5,1.25,1,1.25,1.5,2][index]
            observation=dict(name=r['name'],scale=scale,generation=r['generation'],**gates(mapping,scale))
            observations.append(observation)
            if r['name']=='stationary' and cell['cycle']==0:
                fresh[(cell['config']['id'],cell['fixture'],scale)]=(mapping,folder/'stationary.png')
        # Native checkpoints use their own capture record.
        if cell['config']['role']=='skia':
            for r in rows:
                if r['stage']=='checkpoint':
                    scale=[2,1.5,1.25,1,1.25,1.5,2][int(r['id'].split('-')[-1])]
                    observations.append(dict(name='checkpoint-'+r['id'],scale=scale,
                        generation=r['generation'],**gates(mappings[r['generation']],scale)))
        out.append(dict(id=cell['id'],configuration=cell['config']['id'],fixture=cell['fixture'],
            kind=cell['kind'],status=result['status'],exit=result.get('exit'),
            correctnessErrors=result.get('correctnessErrors',result.get('errors',[])),observations=observations))
    comparisons=[]
    for cell in cells:
        if cell['kind']!='live':continue
        folder=directory/'attempts'/cell['id']
        for index,scale in enumerate([2,1.5,1.25,1,1.25,1.5,2]):
            p=folder/f'checkpoint-live-{index}.png';reference=fresh.get((cell['config']['id'],cell['fixture'],scale))
            if p.exists() and reference:
                comparisons.append(dict(id=cell['id'],checkpoint=index,scale=scale,
                    exactFreshCapture=p.read_bytes()==reference[1].read_bytes()))
    return dict(counts=dict(counts),cells=out,freshLiveComparisons=comparisons,
        disclaimer='Physical mapping and raster density are independent; visual acceptance remains separate.')
if __name__=='__main__':print(json.dumps(report(Path(sys.argv[1])),indent=2))
