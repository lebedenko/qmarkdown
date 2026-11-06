from pathlib import Path
import json
import sys
from unittest.mock import patch

root = Path(__file__).resolve().parent
repo = root.parents[1]
sys.path.insert(0, str(repo/'specs/073-rendering-architecture/demonstrator/performance-experiment'))
import diagnostics
import recovery
from build import digest

output = root/'diagnostics-resume-validation'
sys.argv = ['diagnostics.py', str(root/'sources/patched'), str(repo/'build-ci/073-performance-final'),
            str(root/'sources/counter'), str(output), '--resume-from', str(root/'diagnostics'),
            '--identity-map', str(root/'identity-map.json'), '--control', str(root/'sources/control'),
            '--recovery-manifest', str(root/'recovery-manifest.json')]
with recovery.execution_lock(root/'execution.lock'), \
     patch.object(diagnostics, 'supervise', side_effect=AssertionError('completed measurement was rerun')), \
     patch.object(diagnostics.subprocess, 'run', side_effect=AssertionError('completed build was rerun')):
    diagnostics.main()
assert recovery.phase_complete(output, 'finished')
for name in ['index.jsonl', 'equivalence.jsonl', 'control-equivalence.jsonl', 'variant-geometry.json']:
    assert digest(output/name) == digest(root/'diagnostics'/name), name
(root/'resume-validation.json').write_text(json.dumps(dict(
    finishedPhaseResumePassed=True, noMeasurementOrBuildSubprocesses=True,
    indexesAndComparisonsByteIdentical=True,
    runnerSha256=digest(Path(diagnostics.__file__))), indent=2)+'\n')
print('Finished-phase resume passed without rerunning any measurement or build')
