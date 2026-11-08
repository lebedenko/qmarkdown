#!/usr/bin/env python3
"""Independent full offline reanalysis; compare all deterministic deliverables."""
from pathlib import Path
import sys
import tempfile
import analyze as a

ARTIFACTS = ('matrix.json', 'processes.csv', 'groups.csv', 'cells.csv', 'report.md',
             'pilot-schedule.json', 'batching-pilot-proposal.md', 'input-identities.json', 'integrity.json')


def verify(output):
    with a.no_launches(), tempfile.TemporaryDirectory(prefix='073-characterization-reanalysis-') as tmp:
        destination = Path(tmp)/'reanalysis'
        a.analyze(destination)
        for name in ARTIFACTS:
            a.require((output/name).read_bytes() == (destination/name).read_bytes(), 'reanalysis mismatch: '+name)
        print('Independent full reanalysis: nine deliverables byte-identical; archived revision summary, observations and report exact; zero launches.')


if __name__ == '__main__': verify(Path(sys.argv[1]).resolve())
