#!/usr/bin/env python3
"""Offline provenance, archive read-back and unchanged preparation audit."""
import argparse
import json
from pathlib import Path
import tarfile
from study import sha, load, save
from instrument import HERE


def audit(build):
    spec=HERE.parents[1];evidence=spec/'evidence/boundary-experiment'
    historical=load(evidence/'native-summary.json')['results'][0]['inputsAfter']
    manifest=load(build/'fixtures.json')
    for name in manifest['fonts'].values():
        if sha(Path(name))!=historical[name]:raise ValueError('historical font mismatch: '+name)
    for name in ('native.cpp','layout.h','minimal.json'):
        path=HERE.parent/'boundary-experiment'/name
        if sha(path)!=historical[str(path)]:raise ValueError('historical source mismatch: '+name)
    artifacts=load(evidence/'artifacts.json')
    # Check recorded archive hashes before extracting data, never execute archive code.
    archive_hashes={}
    for name in ('native.tar.gz','geometry.tar.gz'):
        path=evidence/name;observed=sha(path)
        # Historical artifact keys are relative filenames.
        expected=artifacts.get(name)
        if isinstance(expected,dict):expected=expected.get('sha256')
        if expected!=observed:raise ValueError('archive identity mismatch: '+name)
        archive_hashes[name]=observed
    archived_manifest=None
    with tarfile.open(evidence/'native.tar.gz') as archive:
        for member in archive:
            if member.isfile() and member.name.endswith('/fixtures.json'):
                raw=archive.extractfile(member).read()
                import hashlib
                if hashlib.sha256(raw).hexdigest()==historical['build-ci/073-boundary-final-native/fixtures.json']:
                    archived_manifest=json.loads(raw);break
    if archived_manifest is None:raise ValueError('verified archived fixture source not found')
    if {f['id']:f for f in archived_manifest['fixtures']}!={f['id']:f for f in manifest['fixtures']}:
        raise ValueError('fixture semantics changed')
    # The only overlays are coordination and observation. Preparation/painting bodies
    # are kept verbatim and linked to unchanged pinned libraries.
    checks={}
    for name in ('qt.cpp','native.cpp'):
        original=(HERE.parent/name if name=='qt.cpp' else HERE.parent/'boundary-experiment'/name).read_text()
        generated=(build/'source'/name).read_text()
        markers=[('    void prepare(', '    void counters(')] if name=='qt.cpp' else [('    void layout(', '    void select(')]
        for start,end in markers:
            a=original[original.index(start):original.index(end)]
            if a not in generated:raise ValueError('preparation/paint behavior changed: '+name)
        checks[name]='original preparation/layout/paint region retained verbatim'
    save(build/'preflight.json',dict(archives=archive_hashes,fixtureArchiveReadBack=True,
        historicalFonts=True,originalLayoutBodies=checks,publicAPIChanges=False))


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('build',type=Path);audit(p.parse_args().build.resolve())
