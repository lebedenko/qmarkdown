# Phase attribution study

Feature 073 D1–D4 is diagnostic only. Historical sources and binaries are preserved.
The runner uses recovered pinned control prerequisites and verifies boundary source
hashes from the historical build before applying private instrumentation to copies.
It requires the recovered `build-ci/073-performance-recovery` prerequisites.

```sh
python3 specs/073-rendering-architecture/demonstrator/phase-experiment/run.py build-ci/073-phase-new-workspace build-ci/073-phase-new-evidence
python3 specs/073-rendering-architecture/demonstrator/phase-experiment/test_phase.py build-ci/073-phase-new-evidence
python3 specs/073-rendering-architecture/demonstrator/phase-experiment/report.py build-ci/073-phase-new-evidence build-ci/073-phase-new-evidence/summary.json
```

For resume, use the same workspace, a **new** evidence directory and
`--resume-from PRIOR`. Identities, checkpoints and acknowledged raw/fixture hashes
must match. A partial process is terminal; its completed phase lines and truncated
tail are retained, never silently rerun. Build-preparation failures are retained
and require a corrected new workspace. The exclusive historical execution lock is
inherited by build and measurement children. Each trace has a 60-second deadline
and 4 GiB sampled RSS ceiling; no CPU/native timing matrices are repeated.

Counters are reset per paragraph before each operation and snapshotted before
geometry queries. `fitting` selects only the call to visual slices through candidate
padding; `formatting` covers every other visual-slice invocation during layout.
`runVisits` counts entries in the shaped-run scan, not shaper invocations.
`ownerComparisons` counts span containment tests in that function; it does not
include the separate vertical-padding/padded-state scans. `whitespaceVisits`
counts successful trimming iterations, not failed loop-condition tests. `slices`
counts intermediate emitted visual slices, not unique document fragments.
Unchanged repeat counts zero when layout reuses its lines. Line denominators are
resulting lines (including repeat's reused lines), not newly constructed lines.

Construction and source validation precede clocks. Metadata spans are copied
before clocks; the mutation and consequent layout are timed together. Only
phase-level clocks are used. Geometry includes glyphs/positions, lines, visual
slices, tight range boxes and bidirectional source mappings after every operation.
Runtime reset checks repeat geometry extraction outside measurement. The historical
boundary inspector additionally exercises complete correctness gates. Instrumented
durations are diagnostic observations only. Counts and adjacent-size ratios do
not establish asymptotic complexity or exact CPU-time percentages.
