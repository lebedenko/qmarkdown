"""Exact differential checks; diagnostic clocks are deliberately ignored."""
import json
from pathlib import Path
import hashlib


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def validate_baseline(root):
    # Archive manifests anchor the retained index and raw traces, not a new hash
    # computed from an untrusted baseline at execution time.
    archive = Path(__file__).resolve().parents[2]/'evidence/phase-experiment'
    manifest = json.loads((archive/'archives.json').read_text())
    return validate_manifest(root, archive, manifest)


def validate_manifest(root, archive, manifest):
    identities = {str(archive/'archives.json'): digest(archive/'archives.json')}
    for name, record in manifest['archives'].items():
        assert digest(archive/name) == record['sha256'], name
        identities[str(archive/name)] = record['sha256']
        for relative, expected in record['files'].items():
            assert digest(root/relative) == expected, relative
            identities[str(root/relative)] = expected
    return identities


def compare(rows, baseline, label):
    assert len(rows) == len(baseline) == 4
    for row, old in zip(rows, baseline):
        for key in ('phase', 'geometry', 'shapeDeltas'):
            assert row[key] == old[key], (row['phase'], key)
        if label != 'instrumented':
            continue
        for index, (work, prior) in enumerate(zip(row['counters'], old['counters'])):
            for key in ('candidates', 'whitespaceVisits'):
                assert work[key] == prior[key], (row['phase'], key)
            for bucket in ('fitting', 'formatting'):
                for key in ('calls', 'runVisits', 'reorderCalls', 'slices'):
                    assert work[bucket][key] == prior[bucket][key], (row['phase'], bucket, key)
                assert work[bucket]['ownerComparisons'] == 0, 'unexpected fallback'
            if row['phase'] in ('reflow', 'repeat'):
                assert work['ownerBuilds'] == work['ownerExaminations'] == 0
            assert work['ownerPayloadBytes'] <= work['ownerCapacity'] * 8
            assert work['ownerCapacity'] <= len(row['geometry'][index]['runs'])
