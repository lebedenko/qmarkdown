"""Read-only dimensional review of the collected stationary document cases."""
import json
from pathlib import Path
import re
import sys
from PIL import Image, ImageDraw

run=Path(sys.argv[1]);output=Path(__file__).resolve().parent
cells=[c for c in json.loads((run/'schedule.json').read_text()) if c['scale']==1.25 and c['fixture']=='document' and c['cycle']==0]
mappings=[];sheet=Image.new('RGB',(1600,990),'#dedede');draw=ImageDraw.Draw(sheet)
for i,cell in enumerate(cells):
    folder=run/'attempts'/cell['id']
    records=[json.loads(s) for s in (folder/'stdout.jsonl').read_text().splitlines()]
    ready=next(s for s in records if s['stage']=='mapping' and s['reason']=='ready')
    shot=next(s for s in records if s['stage']=='capture-mapping' and s['name']=='stationary')
    result=json.loads((folder/'result.json').read_text());trace=(folder/'wayland.log').read_text()
    surfaces={s[0] for s in result['wayland']['surfaceEnters']};assert len(surfaces)==1
    surface=surfaces.pop()
    viewports=[v for v,s in re.findall(r'get_viewport\(new id wp_viewport[@#](\d+), wl_surface[@#](\d+)\)',trace) if s==surface]
    assert len(viewports)==1
    viewport=viewports[0]
    destinations=re.findall(r'wp_viewport[@#]'+viewport+r'\.set_destination\((\d+), (\d+)\)',trace)
    assert destinations and destinations[0]==destinations[-1]
    sources=[list(map(float,s.split(','))) for s in re.findall(r'wp_viewport[@#]'+viewport+r'\.set_source\(([^)]+)\)',trace)]
    assert all(s==[-1,-1,-1,-1] for s in sources), 'unexpected source crop'
    destination=list(map(int,destinations[0]));window=shot['window'];physical=[v*1.25 for v in destination]
    factors=[physical[k]/window[k] for k in range(2)]
    mappings.append(dict(id=cell['id'],configuration=cell['config']['id'],window=window,buffer=shot['buffer'],surface=surface,viewport=viewport,destination=destination,sourceRequests=sources,desktopScale=1.25,zoom=shot['zoom'],nominalFontSize=ready['nominalFontSize'],physicalWindow=physical,physicalPixelsPerDocumentUnit=factors,nominalFontPhysicalPixels=ready['nominalFontSize']*factors[1]))
    image=Image.open(folder/'stationary.png').convert('RGB')
    image=image.resize((round(image.width*.4),round(image.height*.4)))
    x=(i%4)*400;y=(i//4)*330;sheet.paste(image,(x,y+25));draw.text((x+3,y+5),cell['config']['id'],fill='black')
assert len(mappings)==10 and {m['nominalFontPhysicalPixels'] for m in mappings}=={27.5,22.0}
sheet.save(output/'document-contact-sheet.png')
result=dict(gate='failed',coverage='Stationary document, all ten configurations, desktop 125%. First/restored viewport destinations agree; source crop unset.',method='physical window = viewport destination × verified desktop scale; document unit = physical window / window coordinates at zoom 1',qtNominalFontPhysicalPixels=27.5,skiaNominalFontPhysicalPixels=22,integerRoundingExplanation='All compared dimensions and destinations are exact integers. The 25% size ratio cannot be integer buffer rounding.',rows=mappings)
with (output/'physical-mappings.json').open('x') as f:json.dump(result,f,indent=2);f.write('\n')
print('Ten configurations reviewed; physical-size comparability fails: Qt 27.5 versus Skia 22 nominal-font pixels.')
