#!/usr/bin/env python3
"""Work-count summaries; ratios are observations, not CPU-time shares."""
import argparse
import json
from pathlib import Path


def summarize(root):
    results=[]
    for line in (root/'index.jsonl').read_text().splitlines():
        record=json.loads(line)
        if record['label']!='instrumented' or record['status']!='completed':continue
        for line in (root/record['raw']).read_text().splitlines():
            row=json.loads(line)
            counts={key:sum(w[key] for w in row['counters']) for key in ('candidates','whitespaceVisits')}
            for bucket in ('fitting','formatting'):
                for key in row['counters'][0][bucket]:
                    counts[bucket+'.'+key]=sum(w[bucket][key] for w in row['counters'])
            lines=sum(len(g['lines']) for g in row['geometry'])
            results.append(dict(case=record['case'],variant=record['variant'],phase=row['phase'],
                counts=counts,lines=lines,diagnosticMs=row['diagnosticMs'],
                perCandidate={k:v/counts['candidates'] if counts['candidates'] else None for k,v in counts.items()},
                perLine={k:v/lines if lines else None for k,v in counts.items()}))
    growth=[]
    for variant in ('zero','padded'):
        for phase in ('initial','reflow','repeat','metadata'):
            pair=[next(r for r in results if (r['case'],r['variant'],r['phase'])==(f'dense-480-{n}',variant,phase)) for n in (1024,4096)]
            growth.append(dict(variant=variant,phase=phase,sizeRatio=4,
                ratios={k:pair[1]['counts'][k]/v if v else None for k,v in pair[0]['counts'].items()}))
    return dict(phases=results,adjacentDenseGrowth=growth)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('evidence',type=Path);p.add_argument('output',type=Path)
    a=p.parse_args();a.output.write_text(json.dumps(summarize(a.evidence),indent=2)+'\n')
