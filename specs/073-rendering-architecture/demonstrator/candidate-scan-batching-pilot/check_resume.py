#!/usr/bin/env python3
"""Completed-prefix recovery and deterministic reanalysis under fatal launch sentinels."""
from pathlib import Path
import sys
import run as p
import report


def verify(prior,destination):
    with p.a.no_launches():
        records=p.validate(prior,True)
        before={name:(prior/name).read_bytes() for name in ('analysis.json','selection.json','report.md')}
        recovered=p.resume(prior,destination)
        p.require(records==recovered,'resume mismatch')
        report.report(destination)
        for name,data in before.items():p.require((destination/name).read_bytes()==data,'reanalysis mismatch: '+name)
    print('Completed resume: zero launches; full validation and three byte-identical reanalysis deliverables.')

if __name__=='__main__':verify(Path(sys.argv[1]).resolve(),Path(sys.argv[2]).resolve())
