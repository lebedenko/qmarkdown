# Shaped-run ownership experiment

Approved Feature 073 O1–O5 / O-T1–O-T4. This standalone experiment changes only
private copied SkParagraph internals; it does not qualify performance or integrate
Skia into QMarkdown. The prior phase study and pinned boundary files remain intact.

```sh
python3 specs/073-rendering-architecture/demonstrator/ownership-experiment/run.py build-ci/073-ownership-new-workspace build-ci/073-ownership-new-evidence
python3 specs/073-rendering-architecture/demonstrator/ownership-experiment/test_ownership.py build-ci/073-ownership-new-evidence
python3 specs/073-rendering-architecture/demonstrator/ownership-experiment/test_verification.py
python3 specs/073-rendering-architecture/demonstrator/ownership-experiment/report.py build-ci/073-ownership-new-evidence build-ci/073-phase-evidence-v3
```

Prerequisites are the retained hash-verified recovered control/boundary sources,
archives, fonts and phase traces. `run.py` verifies the baseline against the archived
file manifest before copying/building or measurement. `ownership.patch` applies
with zero fuzz to fresh boundary copies; `instrument.py` subsequently adds private
counters only to the instrumented copy. Both executables and a separate focused
C++ test executable are built. No prior source, binary or patch is overwritten.

Resume uses the same workspace, a **new** evidence directory and `--resume-from
PRIOR`. All recorded identities/checkpoints/raw traces must validate. Interrupted,
failed, timed-out and RSS-limited traces are terminal; no automatic retry. Children
inherit the exclusive recovery lock. Traces are serial, bounded to 60 seconds and
4 GiB sampled RSS. A build-preparation failure requires a corrected new workspace.
The successful completed-run resume check launches no new traces.

The table is private and indexed by shaped-run index, containing a span index,
no-owner sentinel or legacy-search sentinel. A run with any strictly internal span
edge uses the unchanged clipped-range search. Validity requires the private run
revision; invalid entries also fall back. The span vector supplies live identity
and padding. The original intersection, candidate scan, trimming, bidi ordering,
slice construction and coalescing remain unchanged.

Invalidation releases storage before run clear, shaping restart, metadata mutation
and cache restoration. Successful shaping builds after cluster preparation.
`resetContext()` only resets metrics and deliberately retains ownership. Equal
setters, width reflow and repeat retain storage. Empty metadata allocates nothing.
A rebuild constructs a fresh run-sized vector and releases previous storage.

Counters are snapshotted before geometry queries. `ownerBuilds` counts allocated
builds; `ownerExaminations` counts each build-time span visit; `ownerLookups` counts
calls to ownership resolution (including any fallback). Historical bucketed
`ownerComparisons` now counts fallback containment comparisons. Remaining phase
counters retain their original definitions. Capacity is measured in entries;
payload bytes are size × sizeof(size_t), excluding allocator overhead. Diagnostic
reset does not mutate ownership/layout. Construction, geometry and serialization
remain outside layout clocks; durations are observations only.

The C++ tests compare all full/clipped/empty ranges against legacy search, including
adjacent spans, non-code runs, placeholders and deliberately unsplit runs. They
also exercise stale revisions, invalid entries, identity/padding/range setters,
removal/re-addition, equal setters, empty/failed shaping, discarded runs, cache
restoration and allocation-free width/repeat reuse. Historical gates cover complex
shaping, whitespace, zero width, wrap transitions, selection and hit tests.

See [actual evidence](../../evidence/ownership-experiment/README.md) for work counts,
zero-padding preparation overhead, memory observations and retained build failure.
