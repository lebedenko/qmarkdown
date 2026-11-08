# Baseline/self candidate-scan calibration

CB1–CB4 and CB-T1–CB-T5 approved by the owner's supplied-plan implementation
request on 2026-11-08. Retention remains not established under every outcome.
No renderer edits, builds, candidate comparisons or qualification are authorized.

Run from the repository root, in order:

```sh
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-calibration/test_calibration.py
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-calibration/run.py build-ci/073-candidate-calibration
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-calibration/report.py build-ci/073-candidate-calibration
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-calibration/check_resume.py build-ci/073-candidate-calibration build-ci/073-candidate-calibration-resume
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-calibration/package.py build-ci/073-candidate-calibration specs/073-rendering-architecture/evidence/candidate-scan-calibration
```

Both logical arms use the existing verified baseline binary and immutable fixture
paths. Before launching, preparation verifies historical archives, current source
and binary identities, and fixture hashes; it freezes the complete schedule,
commands, exact numeric limits and runtime identity. Environment values are hashed
rather than disclosing potential credentials, with ordinary locale/path settings
also visible. Every process inherits the same unmodified environment snapshot.
The execution lock, 60-second deadline and 4 GiB sampled RSS ceiling are inherited.

The durable attempts journal precedes process launch. Missing, partial, failed or
unacknowledged launches are terminal, even when no output file exists. Resume into
a new directory accepts only a fully validated completed prefix, never repeats an
attempt and requires unchanged environment/affinity for further launches. A complete
resume validates evidence without launching. The authoritative persisted schedule
is regenerated and compared, not silently replaced.

Analysis validates all 82 records before generating eight cells and 160 paired
operation observations. Historical reporter metrics and percentile indexing are
reused with seed 73077; only target envelopes decide resolved/unresolved control.
No historical timing correction or retention claim follows from this diagnostic.
