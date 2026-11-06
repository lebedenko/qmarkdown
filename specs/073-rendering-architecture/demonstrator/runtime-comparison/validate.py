#!/usr/bin/env python3
"""Audit a closed study, including symmetric build flags and complete paired output."""
import argparse
import importlib.util
import json
from pathlib import Path

HERE=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('runtime_runner',HERE/'run.py')
runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)


def validate(workspace, evidence):
    assert runner.recovery.phase_complete(evidence,'finished')
    assert runner.recovery.phase_complete(evidence,'correctness')
    assert runner.recovery.phase_complete(evidence,'gates')
    identities=json.loads((evidence/'identities.json').read_text())
    for name,expected in identities.items():assert runner.digest(Path(name))==expected,name
    rows=runner.recovery.rows(evidence/'index.jsonl')
    scheduled=runner.schedule()
    assert json.loads((evidence/'schedule.json').read_text())==scheduled
    # This audit deliberately demands complete evidence; reporting also supports incomplete cells.
    assert len(rows)==528 and all(r['status']=='completed' for r in rows)
    assert [r['id'] for r in rows]==[e['id'] for e in scheduled], 'nonserial or reordered schedule'
    baseline=runner.ROOT/'build-ci/073-phase-evidence-v3'
    nulls=0
    for i in range(0,len(rows),2):
        pair=[]
        for row in rows[i:i+2]:
            for key,value in scheduled[i+len(pair)].items():assert row[key]==value
            raw=evidence/row['raw']
            assert runner.digest(raw)==row['rawSha256']
            assert runner.digest(evidence/(row['case']+'.json'))==row['fixtureSha256']
            pair.append(runner.validate_output(raw,row,evidence/(row['case']+'.json'),baseline))
            nulls+=sum(r['elapsedNs'] is None for r in pair[-1])
            assert row['wallSeconds']<60 and row['peakObservedRssBytes']<=4*1024**3
        for a,b in zip(*pair):
            assert a['shapeDeltas']==b['shapeDeltas']
            assert a['applicableParagraphs']==b['applicableParagraphs']
            if rows[i]['mode']=='correctness':assert a['geometry']==b['geometry']
    flags=[]
    for label in ('baseline','optimized'):
        build=(workspace/(label+'-build')/'build.ninja').read_text()
        assert 'PHASE_COUNTERS' not in build
        flags.append([line.strip().replace(str(workspace/label),'STAGE') for line in build.splitlines()
                      if line.strip().startswith(('FLAGS =','DEFINES =','INCLUDES ='))])
        for source in (workspace/label/'modules/skparagraph/src').glob('*'):
            if source.suffix in ('.cpp','.h'):
                text=source.read_text()
                assert 'phaseCandidates' not in text and 'ownerExaminations' not in text
    assert flags[0]==flags[1], 'asymmetric compile flags'
    for relative in ('args.gn','obj/modules/skparagraph/skparagraph.ninja'):
        assert (workspace/'baseline/out/comparison'/relative).read_bytes()==(workspace/'optimized/out/comparison'/relative).read_bytes()
    for f in (workspace/'baseline/out/comparison').glob('*.a'):
        if f.name!='libskparagraph.a':assert runner.digest(f)==runner.digest(workspace/'optimized/out/comparison'/f.name)
    print(f'PASS: 48 correctness + 480 timing processes, 264 adjacent pairs, {nulls} null metadata records; symmetric uninstrumented flags/libraries')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('workspace',type=Path);p.add_argument('evidence',type=Path)
    a=p.parse_args();validate(a.workspace.resolve(),a.evidence.resolve())
