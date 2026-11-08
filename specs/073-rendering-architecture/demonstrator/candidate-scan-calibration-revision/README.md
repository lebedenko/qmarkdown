# Baseline/self calibration revision

MR1–MR4 and MR-T1–MR-T5 were approved by the owner's supplied-plan implementation
request on 2026-11-08. Retention remains not established under every outcome.

Run from the repository root, in order:

```sh
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-calibration-revision/test_calibration.py
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-calibration-revision/run.py build-ci/073-candidate-calibration-revision
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-calibration-revision/report.py build-ci/073-candidate-calibration-revision
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-calibration-revision/check_resume.py build-ci/073-candidate-calibration-revision build-ci/073-candidate-calibration-revision-resume
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-calibration-revision/package.py build-ci/073-candidate-calibration-revision specs/073-rendering-architecture/evidence/candidate-scan-calibration-revision
```

Before launch, verify continuation, diagnosis and calibration archive/member hashes
and pinned source/binary/fixture identities. Both logical arms use the unchanged
continuation baseline binary with identical arguments and inherited environment.
No rebuilds or regenerated inputs. Freeze all 322 commands, schedule, environment
fingerprints, runtime/generator identities and exact archived limits.

After two correctness processes, execute forty contiguous ABBA/BAAB groups per
fixture. Seed 73078 generates ten shuffled [ABBA, ABBA, BAAB, BAAB] blocks for
prose then dense, followed by shuffled fixture order for each round. Preserve
initial, width+17 reflow, unchanged repeat and metadata, zero padding, serial
execution, inherited affinity, 60-second deadline and 4 GiB sampled RSS ceiling.
Parent monotonic start/end timestamps accompany duration/RSS evidence.

Durable attempted launches are never retried. Failed/ambiguous attempts are
terminal. Resume accepts only a fully validated completed prefix in a new output
directory; further launches require identical environment and affinity. Completed
resume uses fatal supervisor/Popen sentinels to prove no launches.

The estimate is median of forty ((B1+B2)−(A1+A2))/(2×paragraphs) values.
Whole-group bootstrap: 10,000 resamples, seed 73079 reset per fixture, shared
indices across operations; historical percentile indexing. All eight cells,
nulls, raw observations, group differences, arm means, MAD/range/signs, order and
half summaries are retained. Ratios use median relative improvement across groups
and are suppressed below the 100× clock period/resolution threshold.

Both exact target envelopes must resolve without timer limitation to recommend a
separately approved path-isolation experiment with contemporaneous controls using
this grouping. Otherwise stop and reconsider measurement procedure. Forty groups
are a fixed budget, not a power/independence guarantee. Ideal linear position
effects cancel; actual elapsed-time drift need not. No pooling, changed historical
gates, calibration subtraction, historical variability explanation, renderer edits,
candidate comparisons, further samples, commits, merges or publication.
