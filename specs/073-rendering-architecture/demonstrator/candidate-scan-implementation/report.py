#!/usr/bin/env python3
"""Exact diagnostic work and storage report; makes no runtime-effect claim."""
import json
from pathlib import Path
import sys
import run as runner

def report(output):
    for cp in output.glob('*-checkpoint.json'):runner.recovery.phase_complete(output,cp.name.removesuffix('-checkpoint.json'))
    for name,expected in json.loads((output/'identities.json').read_text()).items():assert runner.digest(Path(name))==expected,name
    records=runner.recovery.rows(output/'index.jsonl')
    assert len(records)==96 and all(r['status']=='completed' for r in records)
    result=[]
    for e in records:
        assert runner.digest(output/e['raw'])==e['rawSha256']
        assert runner.digest(output/(e['case']+'.json'))==e['fixtureSha256']
        rows=runner.validate(output/e['raw'],e,output)
        for r in rows:
            if e['mode']=='clean':continue
            scans=[w['scan'] for w in r['counters']]
            record=dict(case=e['case'],variant=e['variant'],mode=e['mode'],phase=r['phase'])
            for key in ('guardVisits','preprocessing','trimming','suffixComparisons','evaluations','skipped','shadowChecks','scratchSize','scratchCapacity','scratchBytes','lines'):
                record[key]=sum(s[key] for s in scans)
            record['maxParagraphScratchBytes']=max(s['scratchBytes'] for s in scans)
            record['fallback']=sum(sum(s['fallback']) for s in scans)
            result.append(record)
    for case in ('long-480-16384','spans-480-32'):
        for phase in ('initial','reflow','metadata'):
            a=next(r for r in result if (r['case'],r['variant'],r['mode'],r['phase'])==(case,'padded','oracle',phase))
            b=next(r for r in result if (r['case'],r['variant'],r['mode'],r['phase'])==(case,'padded','prototype',phase))
            assert 0<b['evaluations']<a['evaluations'],'insufficient reduction'
    return dict(processes=96,operationRecords=384,maxSampledRssBytes=max(r['peakObservedRssBytes'] for r in records),
                maxParagraphScratchBytes=max(r['maxParagraphScratchBytes'] for r in result),work=result,
                conclusion='Positive work reduction only; runtime effect unmeasured.')

if __name__=='__main__':print(json.dumps(report(Path(sys.argv[1])),indent=2))
