# Candidate-scan proof experiment

Approved Feature 073 C1–C5 / C-T1–C-T5. This isolated diagnostic uses the verified
ownership implementation and changes no shipping library or Qt integration.

```sh
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-experiment/run.py build-ci/073-candidate-fresh build-ci/073-candidate-fresh-evidence
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-experiment/report.py build-ci/073-candidate-fresh-evidence
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-experiment/test_harness.py
```

Prerequisites are the retained recovered Skia control/boundary build, ownership
workspace, fonts and hash-verified ownership/runtime archives. Identities validate
before copying. `prototype.py` applies checked replacements after the inherited
ownership instrumentation; `candidate.patch` records the exact resulting source
diff against ownership. Oracle, shadow and prototype share one diagnostic binary,
selected by a private driver mode. Historical gates use prototype by default.

The serial schedule is written before builds/execution: twelve fixture IDs in
`run.py`, zero then padded, oracle then shadow then prototype. Each fresh process
covers initial, width +17, unchanged repeat and metadata identity mutation/layout.
There is no timing mode. The inherited lock is held by children, every diagnostic
process has a 60-second deadline and 4 GiB sampled RSS ceiling, and every failure
is terminal. Build-preparation corrections use fresh directories. Resume requires
`--resume-from PRIOR` and a new output directory with the same workspace. It
validates identities, checkpoints, schedule, raw hashes and all comparisons before
copying. Failed/unacknowledged output is rejected; no trace is retried. Separate
build, execution and historical-gate journals are append-only until checkpointed.

## Bound and arithmetic contract

For candidate i let t[i] be its trimmed width computed with exactly the exhaustive
loop's scalar additions and trailing-whitespace subtractions, in the same order.
Let m[i] = min(t[i], …, t[last]) through the first hard break, inclusive. Every
candidate's padding p[i] is a sum of nonnegative slice-edge costs. Under guarded
IEEE arithmetic, fl(t[i] + p[i]) ≥ t[i] ≥ m[i]. Therefore m[i] > available implies
that no remaining candidate fits. Equality is evaluated. The argument permits
negative cluster widths, zero widths, whitespace-induced decreases and arbitrary
padding-cost decreases; it requires no monotonicity of total candidate widths.

The prototype stores endpoints and trimmed endpoints, widths and minima. It
computes the first grapheme-eligible endpoint during complete enumeration, so
stopping evaluation cannot change emergency selection. It preserves the original
legal-endpoint preference and last-fitting fallback. Hard breaks, upstream breaker,
ownership lookup, bidi ordering, visual slices and final formatting are unchanged.

`ScanBound.h` requires binary32 IEEE SkScalar, FLT_EVAL_METHOD=0, no fast/finite
math, x86 SSE2, nearest rounding in both the C environment and MXCSR, and disabled
FTZ/DAZ. Widths, every accumulated/subtracted intermediate and horizontal padding
must be finite; horizontal padding must be nonnegative. A conservative 4 × max pad
× (run count + 1) envelope guards slice sums and required additions against overflow.
Each run contributes at most one slice with two edges. Rounded nonnegative
accumulation can be bounded by twice the sum of its nonnegative addends: at each
step the error cannot exceed that step's addend, since the prior sum is exactly
representable. The envelope and half-range trimmed-width guard leave further
slack. Failure of any assumption routes the line through the original loop.
Unsupported guard cases are tested, rather than treated as proof observations.

## Counts and storage

`preprocessing` counts cheap endpoint visits; `trimming` counts their whitespace
subtractions; `suffixComparisons` counts reverse minimum comparisons. `evaluations`
counts expensive candidate padding calls; `skipped` counts the remaining suffix.
Shadow executes exhaustive evaluation and validates hypothetical skips separately
in `shadowChecks`; its validation visits/lookups are excluded from fitting and
formatting counters. It performs no extra trimming during validation.

`scratchSize` is maximum used entries, `scratchCapacity` is reserved entries and
`scratchBytes` is reserved capacity × sizeof(Candidate), excluding vector header
and allocator overhead. A single layout-local vector reserves paragraph cluster
count (including the sentinel), reuses it across lines and releases it on return.
The prototype asserts capacity ≤ paragraph cluster count. No persistent candidate
cache or API is added. Zero-padding scanner paths have zero endpoint preprocessing,
trimming and allocation. The diagnostic also performs constant environment and
span-padding guard checks outside the scanner; their cost is not included in the
endpoint counter. They should be conditional on entering the padded scanner in any
future retained implementation. Existing zero-padding ownership regressions remain
an explicit constraint; this experiment does not change ownership preparation.

Enumeration and repeated trimming retain their original worst-case costs. The
mechanism removes expensive padding evaluation, not cheap enumeration or all
quadratic work. Diagnostic durations cannot establish a runtime effect.

See [evidence](../../evidence/candidate-scan-experiment/README.md) and the
[separately approvable recommendation](../../candidate-scan-recommendation.md).
