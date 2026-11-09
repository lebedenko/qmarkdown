#!/usr/bin/env python3
"""Reproduce the executed initial input checks in a fresh preflight directory."""
import json
from pathlib import Path
import sys
import prepare_build as b


def verify(output):
    b.p.require(not output.exists(),'fresh preflight directory required')
    output.mkdir(parents=True)
    try:
        with b.p.a.no_launches():
            identities=b.p.historical(output)
            b.p.write(output/'verified-identities.json',identities)
            members=b.p.a.diagnosis.verify_archives(b.FEATURE/'evidence/candidate-scan-batching-refinement',output/'refinement')
            pinned=json.loads((output/'refinement/identities.json').read_text())
            b.p.a.diagnosis.verify_identities(pinned)
            b.p.a.diagnosis.check_hash(b.p.BINARY,members['binary/prepared-reflow'])
            selection=json.loads((output/'refinement/selection.json').read_text())
            b.p.require(selection['complete'] and selection['selection']==96,'unselected size')
        print('Historical inputs and size-96 refinement verified; zero executable launches.')
    except Exception as ex:
        b.p.write(output/'failure.json',dict(status='incomplete',stage='historical-input-verification',
            error=type(ex).__name__+': '+str(ex),executableLaunches=0))
        raise


if __name__=='__main__':verify(Path(sys.argv[1]).resolve())
