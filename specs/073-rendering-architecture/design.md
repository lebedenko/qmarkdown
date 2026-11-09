# Evaluation design

Approval: owner's supplied implementation request, 2026-11-05; evaluation only.

Start with a standalone public-API placement gate. A QTextLayout owns shaped
positions; glyphRuns returns values. Mutating returned QGlyphRun positions must
not be mistaken for updating the source layout. Render the same layout before
and after moving copied glyphs, and compare exact captures. Independently paint
the original/moved runs to establish that the requested placement really differs.
This is a negative capability test for the existing prepared-run adapter, not a
proof that every possible Qt architecture is impossible.

Run independent distance-field/curve processes, retain actual screenshots,
identities and observed API/DPR. A timer moves the item through real rendered
frames; frameSwapped intervals are application swap notifications, not compositor
presentation timestamps or GPU work. Never label these as display qualification.
An external process deadline survives GUI/render-thread stalls. Stage records
flush before application creation, layout, exposure, capture and completion.

The demonstrator uses the actual tested QSGTextNode, with no proxy or shipping
library changes. Interactive mode remains open for inspection; it is a placement
control, not an authoritative full-document 2.x demonstrator. Expansion to padded
composition requires an adapter with explicit cluster/source mapping and bidi
semantics. Per-word layouts, transparent selection emission and synthetic spaces
are not accepted shortcuts. Inline objects must reserve width/ascent/descent in
the same authoritative layout used for selection and hit testing.

Skia must be a native independently built control with explicit font files and
shaping dependencies. Browser CanvasKit or Qt raster output cannot stand in for
native Skia GPU measurements. Missing dependencies are reported as missing
qualification. Production dependency changes remain outside scope.

Full qualification must use paired minimal/realistic fixtures, an actual-source
interactive viewer and browser references. Resource generations are keyed by
content/style/width/fonts/DPR; scroll changes transforms only. Record bounded
fallback work and invalidation, not just average frame times. Measure presentation
feedback separately from captures, CPU timings, GPU queries and resident/texture
memory. No backend is selected from a successful build or a replay-only test.

## Approved demonstrator continuation

The standalone demonstrator lives under demonstrator/. Its manifest indexes
source spans in UTF-16 code units; browser and Skia adapters must translate native
offsets explicitly. Normal blocks advance by measured height. The old overlapping
scene remains a historical probe.

Qt preparation is copied into an isolated build stage from a recorded Git commit;
only the staged copy receives inspection hooks. Prepared glyphs/cursor cells/code
rectangles and object recordings remain authoritative for fallback painting and
selection. Raster origins floor the transformed ink bounds in physical pixels;
painting includes the residual phase, and texture placement cancels that phase.
Native nodes retain their layout until width/style/selection changes. Fallback
work is counted separately.

Skia uses SDL windows, its own GPU context and SkParagraph shaping, with explicit
font files and mapped padding placeholders. Semantics around reservations require
verification before support is claimed. Official source is resolved once and
pinned, with dependency identities recorded by the external build procedure.

Every process flushes stage records. A supervisor retains partial/malformed output,
checks requested against observed identities, hashes inputs before and after runs,
and applies independent startup, operation and shutdown deadlines. GPU runs are
serial and failures do not skip another renderer. Captures are correctness evidence,
not presentation pacing. The inspection title stays outside the document capture.

## Bounded correctness continuation — approved design

Approval: owner explicitly approved C1–C6, this design and mapped tasks on
2026-11-05 ("approved").

### Qt source mapping (C1, C4)

Keep stage.py's original source commit
`43d6bba842bb5b7ab23889e49d5f138c6f22a7fb`. First retain failing geometry traces
on both exact SDKs, including source offsets, cursor-cell bounds, line membership,
visual direction, pointer positions and resolved source offsets. Use these to
separate a geometry defect from an ambiguous edge expectation; never change the
expected copied source to accommodate a failure.

Represent any preparation correction as a checked reproducible patch applied
only to the isolated stage. Fail staging on an unexpected base or patch mismatch;
record original commit, patch SHA-256 and patched output SHA-256 identities.
Interactive selection and offscreen tests consume the same prepared mapping.
No fixture-text branches or independent test-only hit-test correction are allowed.

### Skia fragment reservations (C2, C4)

Use the existing pinned SkParagraph and whole-paragraph shaping. After each
layout, derive code-fragment boundaries and the horizontal/vertical reservations
needed by those fragments; rebuild the mapped layout and repeat. Accept only a
stable boundary/reservation state within eight passes. Track prior states to
reject cycles explicitly, and report non-convergence at the pass limit.

Inserted reservations map to source boundaries without contributing copied text;
collapsed objects retain their complete UTF-16 source interval. Painting,
selection and hit testing all use the final accepted layout and its mapping.
Check horizontal padding on every fragment, vertical space, object bounds and
exact copied source in both directions. Check widths immediately below, at and
above observed wrap transitions, including adjacent spans, bidi, marks and
surrogate pairs. Reject reservations that disrupt bidi order, shaping, legal
wrapping or source mapping. Retain a minimal reproducer and blocker if this
bounded trial fails; do not add synthetic spaces, atomic code spans or another
layout engine.

### Vulkan lifecycle diagnostics (C3)

Start with the existing empty-window reproducer. Exercise default and basic
render loops separately, then explicit scene-graph release with the event loop
still running. Log callback/resource teardown order and collect externally
bounded crash backtraces on each exact SDK. Keep variants diagnostic until
observed Vulkan identity, normal shutdown and three process recreations pass.
Do not switch backend, Qt SDK or system driver to claim a lifecycle correction.
If no valid harness correction passes, keep minimum-Qt Vulkan failed and retain
an independently runnable empty-window reproducer.

### Evidence gates (C4–C6)

Run geometry before affected native fixtures. An unresolved geometry failure
blocks acceptance for that case; retain its failure rather than interpreting
replay as acceptance. Run GPU processes serially, with both Qt renderers/APIs
and each observed Qt DPR; record unavailable Skia scales explicitly. Exercise
stationary and selected captures, resize/reset, motion/replay and recreation.
Shared layout/lifecycle changes trigger the complete demonstrator matrix.

Append evidence separately from historical runs, retaining commands, source and
patch identities, binaries, fonts and before/after results. Update support and
review/migration records from actual outcomes only. Owner visual acceptance
remains separate from automated geometry, process and replay success.

## Fragment-aware upstream experiment — approved design

Owner approval received 2026-11-05 ("approved") for F1–F7 and the mapped tasks.
The following supersedes the
rejected reservation mechanism only for the proposed experiment; its historical
implementation and evidence remain intact.

### Isolation and reproducibility (F1, F6)

Use a separate staged checkout and separate build/output directory. Validate the
pinned base and clean inputs before patching; reject unexpected content and patch
application failures. Keep the original checkout, libraries and control binaries
available and hash them before/after comparison. Record patch SHA-256, patched
source hashes, DEPS revisions, system package/tool versions, GN/CMake options,
font/fixture hashes, library/executable hashes and exact commands. Build with two
workers. Keep experimental metadata/diagnostics confined to the adapter and
patched SkParagraph, with no shipping dependency or API edits.

### Source spans and candidate fit (F2, F3)

The adapter validates ordered, nonoverlapping code intervals and object intervals
against the original UTF-16 source and Unicode grapheme boundaries. Adjacent code
spans remain distinct. Map valid boundaries once to SkParagraph UTF-8 byte
indices, retaining reverse mappings and complete source intervals for collapsed
objects. Reject malformed surrogate sequences, out-of-range/reversed intervals,
partial object intersections and grapheme splits with explicit diagnostics.
Padding metadata must not introduce shaping/style boundaries into otherwise
identical text. A span boundary inside a shaped cluster that cannot receive an
edge without changing its shaping is a capability blocker, not permission to
split or reshape that cluster.

Extend the pinned `TextWrapper` candidate-fit path and `ParagraphImpl` /
`TextLine` formatting path. Reuse upstream clusters, legal break opportunities,
line bidi ordering, trailing-whitespace semantics and emergency-wrap decisions.
For each candidate endpoint, derive contiguous visual fragments by span identity
and visual adjacency after upstream bidi ordering. Charge both padding edges for
each fragment, including disjoint fragments of one logical span. A shared
fragment calculation drives fit and final formatting; font fallback alone does
not create a new fragment.

Scan a finite set of upstream candidate boundaries without assuming fragment
costs are monotonic under bidi reordering. Commit a fitting legal endpoint using
upstream break priority; if none fits, use upstream emergency boundaries while
preserving indivisible graphemes/objects. Each committed nonempty line advances
source; an oversized first indivisible unit advances once and records required
width, available width and unavoidable overflow. Handle hard breaks/empty lines
through upstream semantics. Do not rebuild whole paragraphs to converge, invent
new break opportunities or replace the upstream line-breaking engine. If these
extension points cannot express the behavior, stop and record the limitation.

### One authoritative geometry (F4)

Apply fragment translation and vertical reservations before final line geometry
is published. Use identical adjusted positions for paint, range rectangles,
carets/hit tests and placeholder/object placement. Preserve glyph IDs, fonts,
cluster membership and relative glyph advances within clusters; only fragment
translation and reserved line extents change. Invalidate layout/position caches
when span metadata changes. The empty/disabled-metadata path retains upstream
behavior and is compared with a matching unpatched control using identical
source, styles, object reservations, fonts and widths, without padding inserts.

For `objects@679`, trace unrounded/rounded baseline, ascent/descent, placeholder
rectangle, paragraph bounds and actual object paint bounds. Resolve reservation
and placement together in the enabled experiment; do not add a paint-only shift
or loosen the 0.05 logical-pixel tolerance. Keep any unchanged upstream behavior
in disabled-padding comparisons explicit rather than hiding a mismatch.

### Ordered gates and stop conditions (F5–F7)

1. Disabled-padding equivalence: compare line/source breaks, glyph IDs/fonts,
   positions, object rectangles, caret/source mappings and copied text against
   the pinned unpatched build. Require exact discrete values and record numeric
   comparison precision; do not introduce a new tolerance to mask differences.
2. Geometry: retain all 751 existing cases. Detect changed line-break signatures
   across the width scan, refine transition intervals, and add fractional widths
   below/at/above transitions with recorded probe precision. Cover adjacent spans,
   disjoint bidi fragments, combining marks, ligatures, surrogate pairs, fallback
   fonts, whitespace and objects, including oversized units and invalid ranges.
3. Consistency: assert both padding edges for every visual fragment, vertical
   reservation, object containment, both selection directions and exact copying.
   Compare shaping identities and intracluster advances with matching unpadded
   shaping, and verify hit tests against the same final geometry used for paint.
4. Native: only after the preceding gates pass, run patched Skia serially on
   OpenGL and Vulkan at actually observed DPR. Check stationary/selected captures,
   resize/reset, motion/replay and three process recreations. Preserve deadlines
   and identity checks. Explicitly list unavailable scales; do not rerun or change
   the Qt control to qualify this Skia-only experiment.
5. Review: retain representative matched before/after captures and launcher
   commands. Process/replay success cannot establish owner visual acceptance.

A failing gate retains a minimal fixture, exact command, diagnostics and identities
and blocks claims dependent on that gate. Infrastructure failures remain distinct
from missing layout capabilities. Final support records say feasible only after
all automated gates pass, otherwise blocked with the precise missing capability
or verification obstacle. Document touched upstream internals, patch size,
rebase/regression burden and dependency costs without performance qualification.

## Explicit shaping-boundary continuation — approved design

Owner approval received 2026-11-05 ("approved") for B1–B8 and the mapped tasks. This section supersedes the
historical fragment experiment's prohibition on shaping boundaries and its
requirement to preserve glyphs across those boundaries. It does not retroactively
change the interpretation or results of the diagnostic patch in `d3b1f05`.

### Isolation, boundary ownership and invalidation (B1–B4)

Create fresh stage/build/evidence paths; keep both prior stages and their evidence
unchanged. Base the new checked patch on the same pinned source, preserving the
old diagnostic patch independently. Record base, DEPS, patch and patched-source
hashes, GN/CMake/ABI configuration, tool/package versions, fonts, fixtures,
libraries, executables, outputs and exact reproduction commands. Use two build
workers and verify control identities before/after execution.

The adapter retains validated UTF-16-to-UTF-8 and reverse source mapping, including
complete collapsed-object intervals. Keep original source bytes unchanged.
Reject fractional, negative, reversed, overlapping, out-of-range, malformed
surrogate, grapheme-splitting and partial-object intervals. Carry each span's
identity and UTF-8 edges into private paragraph state independently of style.

Split upstream shaping regions at the union of span starts/ends and existing
upstream region boundaries before shaping; an adjacent same-style edge must
survive style coalescing. Bound shaping context at those edges so ligatures and
joining cannot cross them. Retain upstream shaping within each region, script
segmentation, fallback and paragraph-wide bidi resolution. Do not shape separate
paragraphs or insert characters. Preserve Unicode break flags independently of
shaping splits: a new shaping boundary is not a new legal line-break opportunity.
Real inline objects keep upstream reservation semantics and complete source maps.

For any nonempty boundary metadata, bypass paragraph shaping-cache lookup and
insertion, even with zero padding. Metadata replacement/removal invalidates
shaped runs and all derived clusters, lines, translations and geometry; changing
width invalidates layout only and reuses shaped results. Equal metadata and
repeated layout must remain stable. The metadata-free path retains upstream cache
behavior. Instrument shaping generation privately to verify reuse/invalidation
without depending on timing. Cache optimization is outside this milestone.

### Candidate fitting and authoritative geometry (B5–B6)

Extend the existing TextWrapper candidate and ParagraphImpl/TextLine formatting
paths. One shared calculation consumes upstream visual ordering and source/span
identity to produce contiguous visual fragments. Charge left and right padding
for every fragment; retain disjoint bidi fragments and distinct adjacent spans.
Font fallback alone does not create a fragment. Fitting and formatting must use
identical fragment semantics, including upstream trailing-whitespace handling.

Scan finite upstream candidates without assuming fragment costs are monotonic
under bidi reordering. Retain upstream legal-break priorities and emergency
handling. Each nonempty committed line advances source; preserve indivisible
clusters/graphemes and objects. An oversized first unit advances once and reports
required/available width and unavoidable overflow. Preserve upstream hard-break
and empty-line behavior. No paragraph-level convergence or replacement breaker.

Apply fragment translations before publishing final paragraph geometry. Paint,
range rectangles, caret hit testing, selection and object placement all consume
those positions. Relative glyph advances within each boundary-enabled shaped
cluster, glyph IDs and fonts must remain unchanged by padding. Compute vertical
reservation from full text, object and padding extents. If containment needs a
baseline shift, apply it to every line component, then reserve sufficient rounded
line and paragraph height. Keep the 0.05-pixel tolerance. Diagnose `objects@679`
using raw/rounded metrics, baseline, logical bounds and actual painted bounds;
a paint-only shift or tolerance change is not a fix.

### Ordered verification gates (B7–B8)

1. Boundary semantics: adjacent lam–alef spans must have no cluster crossing the
   shared edge; the whole-span control must retain its ligature. Cover Latin
   ligatures, Arabic joining, identical adjacent styles, mixed bidi, combining
   marks, surrogate pairs, fallback and object edges. Reject grapheme splits
   and verify unchanged source. Record intentional differences from the original
   unpatched shaping, rather than treating those differences as regressions.
2. Separate baselines: disabled experiment without boundary metadata must match
   the unpatched control's line/source breaks, glyph IDs/fonts/positions, object
   placement and source mapping. Establish boundary-enabled zero-padding shaping
   separately. Compare padded glyph identities and intracluster advances against
   that baseline. Require exact discrete equality and record numeric precision;
   do not loosen tolerances to hide discrepancies. Exercise metadata edits,
   removal/re-addition, repeated layout and width changes for stale state.
3. Geometry and consistency: run all 751 existing cases plus boundary regressions.
   Identify wrap transitions by line-break signatures, refine intervals to 1/64
   logical pixel and probe immediately below, at and above each transition using
   that resolution; retain brackets, widths and signatures. Verify every visual
   fragment's padding, vertical reservations, object containment, whitespace,
   oversized-unit overflow and termination. Check exact copied source in both
   selection directions and hit tests against authoritative geometry. Retain the
   complete `objects@679` metric/logical/painted-bound diagnosis.
4. Native: only after preceding gates pass, run patched Skia serially on OpenGL
   and Vulkan at actually observed DPR. Preserve supervisor deadlines and input
   identities. Check stationary/selected captures, resize/reset, motion/replay and
   three process recreations. Explicitly report unavailable DPRs; Qt controls and
   the independent Qt 6.8 Vulkan failure are unchanged.
5. Review: retain representative matched captures and a launcher for the actual
   patched interactive viewer; keep owner visual acceptance separate. Run
   supervisor regressions, lightweight repository checks and whitespace checks.

A failing gate stops dependent gates and retains a minimal fixture, command,
diagnostics and identities, distinguishing infrastructure failure from a missing
layout capability. Declare feasibility only after all automated gates pass.
Document touched upstream internals and patch-maintenance costs. Replacing
upstream shaping, bidi or the line-breaking engine remains outside scope;
using upstream shaping with explicit region boundaries is now permitted.

## Approved measurement continuation (2026-11-06)

P1–P6 implement the owner's supplied plan in a sibling `performance-experiment`
using the unchanged boundary `layout.h`. Existing pinned control and patched
archives are verified against the retained build record before adapter compilation.
Private diagnostic builds add counters in a copied patched checkout; headline
binaries use the original archives. Fixture construction and source mapping stay
identical. Each operation has a distinct timed scope; font initialization is outside
construction. Fresh-process initial measurements include process-local cache state,
not filesystem-cache claims. Warm samples retain paragraph/font state as documented.

A Python supervisor owns deadlines/RSS, incremental JSONL retention, series skipping,
aggregation and hashes. Each case has a stable series/size identity. Native samples
are buffered in memory and emitted after timing. SDL owns its surface and scale
objects; a separate Wayland registry binds only presentation feedback on that surface.
GPU callbacks remain alive until drained; missing callbacks are null. GPU cache bytes
are not device memory. Vulkan submission includes upstream present transitions and
semaphores, separately from queue presentation. Native traversal remains full-document.

Scale diagnosis precedes interpretation, not CPU runs. Protocol tracing and captures
run separately. Reports explicitly retain unknowns; the existing 800/800 ratio alone
is neither desktop scale nor proof of compositor behavior. Pacing uses presented
timestamps, reported refresh and nanosecond representation, never swap-return time.


## Approved recovery — 2026-11-06

Recovery design: snapshot evidence and executable/source identities under build-ci/073-performance-recovery before measurement. Resume into a new directory, validate imported artifacts, retain interrupted outputs, use an exclusive child-inherited execution lock and durable validated phase checkpoints. Preserve original manifests; record relocation and rebuilt prerequisites separately. Restore pinned sources/dependencies and verify hashes before dependent execution. Missing identities stop only affected stages; missing temporary logs remain lost. No production APIs, dependencies or rendering changes.

## Approved diagnostic design (2026-11-06)

Use a new durable build-ci workspace copied from hash-verified recovered pinned
Skia sources. Private counters use two buckets selected only around candidate
padding evaluation; the other bucket counts formatting/traversal. Reset counters
before each operation without touching layout state; snapshot before geometry
queries. Phase clocks enclose only layout (and metadata mutation for invalidation).
Construction, geometry and JSON serialization are excluded. Separate instrumented
and uninstrumented executables use the same new trace driver; original binaries
remain untouched. Reuse supervisor and recovery primitives. Completed operation
records are flushed; interrupted processes remain terminal, never automatically
retried. Historical boundary gates run on the changed diagnostic ABI.

Evidence will rank run ownership precomputation, fragment reuse with invalidation,
and reduced candidate scanning. The proposal must define lifetime, invalidation,
memory, correctness and tasks. Candidate costs are non-monotonic; no binary
search/early termination without proof. Optimization requires separate approval.

Diagnostic execution is complete; see [phase evidence](evidence/phase-experiment/README.md).
The resulting [optimization requirements, design and tasks](optimization-design.md)
are draft and require separate owner approval before implementation.

## Approved ownership optimization — 2026-11-06

The owner instruction to implement the supplied plan explicitly approves O1–O5,
the corrected mechanism and O-T1–O-T4 in [optimization design](optimization-design.md).
This supersedes the earlier draft-only restriction for this bounded isolated step.
Preserve historical evidence, uncommitted diagnosis and commit `4b9fd4c`.
Metric-only `resetContext()` retains ownership; run revision invalidation covers
replacement, restart, metadata and cache restore. Fresh bounded storage, legacy
fallback, exact four-operation differential checks and work counts are required.
No speedup qualification, production change, merge or publication is approved.

## Approved ownership runtime comparison — 2026-11-06

Owner approval is the current instruction to implement the supplied plan. A sibling
`runtime-comparison` experiment preserves earlier experiments. Two uninstrumented
builds use one driver. Construction/font loading occur before clocks. Each operation
uses one steady-clock interval across applicable paragraphs; generation snapshots,
metadata copies, geometry, serialization and flushes occur outside it. Correctness
mode queries complete geometry after each operation; timing mode never does.
Ten fixed passes alternate pair order (five each), serially. Schedule and identities
are durable before execution. Every sample is a fresh process; reflow/repeat reuse
its paragraphs. Failed/interrupted/limited samples are terminal for that cell.
Resume validates immutable inputs, checkpoints and raw records into a new directory.
Reports bootstrap process pairs with seed 73073 and 10,000 resamples, percentile
2.5/97.5 intervals for median times, paired differences and pair ratios. Ratios
are suppressed if either median is below 100 × max(clock period, empirical minimum
positive delta). Timer overhead is never subtracted. RSS is whole-process evidence.

## Approved candidate-scan proof design — 2026-11-06

Owner approval is the current instruction to implement the supplied plan. C1–C5
and C-T1–C-T5 are approved for this isolated experiment.

Enumerate every candidate through the first hard break, inclusive. Reproduce the
exhaustive scalar additions and whitespace subtractions in order. Store endpoint,
trimmed endpoint/width and reverse suffix minimum. Determine first indivisible
endpoint independently of cutoff. Before evaluating candidate i, stop only if
suffix[i] > available. Under guarded IEEE nearest arithmetic, addition of a
nonnegative padding amount cannot decrease a finite trimmed width. Every remaining
required width is therefore at least suffix[i] and cannot fit. Equality must still
be evaluated. This argument does not assume monotonic candidate costs, positive
cluster widths, or endpoint eligibility; legal/fallback/emergency choice is unchanged.

Guard inputs/intermediates conservatively, including a padded-sum overflow envelope;
unsupported arithmetic uses the original exhaustive loop. Oracle executes the
unchanged loop. Shadow also executes it, separately validating every hypothetical
skip. Prototype consumes enumerated endpoints and applies the strict bound.
Use a single layout-local vector reserved to paragraph cluster count, reuse between
lines and release on return; no persistent cache/API. Zero-padding scanner branches
perform no preprocessing. Enumeration remains potentially quadratic; no linear-time
or runtime-effect claim follows from fewer expensive visual-slice evaluations.

Sibling `candidate-scan-experiment` preserves existing sources and archives. Use
hash-anchored ownership sources and historical runtime correctness outputs, immutable
separate build/execution journals, fixed schedule before execution, inherited lock,
60-second process deadline and 4 GiB sampled RSS. Failures are terminal; no retries.
Resume validates identities, checkpoints, complete raw outputs and comparisons into
a new directory. Archive sources, patches, environment, commands, counters and raw
records with per-file/archive hashes. Diagnostic durations are not runtime evidence.

## Retained candidate scanner (R1–R5, approved 2026-11-06)

The owner explicitly approved implementation of the supplied “Feature 073: Retain
the proven candidate-scan cutoff” plan in the current request. Approval covers
requirements, design and tasks below together; existing proof and archives remain
immutable. Runtime measurements, Qt integration, ownership changes, merge and
publication are excluded.

- R1 / R-T1: Produce a sibling clean patch against the hash-verified ownership
  baseline and a separate diagnostic overlay. Preserve exact enumeration through
  the first hard break inclusive, whitespace arithmetic, first indivisible
  endpoint, legal preference, fallback and emergency behavior.
- R2 / R-T2: Use strict suffix-minimum > width, one layout-local vector with the
  prototype entry representation, paragraph-cluster-bounded and reused per line.
  Guard arithmetic and horizontal padding only inside the padded path; zero
  horizontal and vertical padding performs no cutoff preparation or guards. Keep
  vertical-only behavior. Unsupported arithmetic or nonfinite inputs/intermediates
  falls back to the original exhaustive loop after resetting partial endpoints.
- R3 / R-T3: Predeclare and execute twelve fixtures × zero/padded × oracle/shadow/
  instrumented/clean = 96 fresh serial processes, four operations each. Compare
  exact geometry, mappings, visual slices, glyph positions and shaping against
  archived correctness traces. Preserve ownership guards and reject unexpected
  supported-fixture fallback. Audit clean sources for diagnostic state.
- R4 / R-T4: Reuse exhaustive short-sequence and seed-73074 10,000-case numeric
  coverage, focused boundaries, actual fallback, floating-point rejection, zero
  guard/preprocessing/allocation checks, historical gates and tooling regressions.
  Preserve inherited locks, child inheritance, deadlines, RSS limits, terminal
  failure semantics, immutable separate journals and validated new-directory resume.
- R5 / R-T5: Archive sources, patches, identities, configuration, commands,
  environment, raw outputs, counters and reports; verify all file/archive hashes.
  Accept only with no counterexamples, bounded storage, unchanged zero path and
  reduced expensive evaluations on padded long-span and spans-32. Report guard,
  preprocessing, trimming and storage costs without a speedup claim.

Actual results belong in verification.md and the sibling evidence record.

## Candidate-scan runtime study (CR1–CR5, approved 2026-11-06)

The owner's instruction to implement the supplied “Feature 073: Measure the retained
candidate-scan cutoff” plan explicitly approves these requirements, the sibling
design and CR-T1–CR-T5 before implementation or measurement. This supersedes the
prior runtime exclusion for this study only. Existing work and archives remain
unchanged. Further optimization, shipping Qt integration, ownership changes, merge
and publication remain excluded.

- CR1: Compare two fresh uninstrumented Release ownership-baseline copies, one with
  only the verified clean cutoff patch. Audit sources, flags and binaries before
  execution. Preserve pinned dependencies, identical driver and inherited affinity.
- CR2: Keep twelve fixtures byte-for-byte; add prose/dense/mixed/long/objects at
  64/256 UTF-16 units, width 480, each with 32 independently constructed paragraphs.
  Persist 88 correctness then 1,760 timing processes before building: twenty adjacent
  baseline/retained pairs per fixture/padding cell, alternating order each pass.
- CR3: Initial, width +17, repeat and metadata identity mutation/layout use existing
  operation-wide steady-clock intervals including guards/preprocessing/allocation
  and metadata application. Construction, copying, snapshots, queries and output
  stay outside clocks; timing never queries geometry. Metadata-free operations null.
- CR4: Require exact paired output and archived existing-fixture traces, shaping
  reuse/invalidation, unchanged historical boundary/extent gates and archived proof
  verification. Retain lock inheritance, 60-second deadlines, 4 GiB sampled RSS,
  terminal study failure without retry, separate immutable journals and validated
  new-directory completed resume with fatal launch sentinels.
- CR5: Report every cell, absolute paired differences and relative improvements,
  timer resolution, whole-process RSS and small per-applicable-paragraph times.
  Bootstrap 10,000 whole-process pairs with seed 73075, preserving operations.
  Padded long/span-32 initial/reflow/metadata each require median paired improvement
  >=10% and 95% lower bound >0. Every other applicable cell requires 95% upper bound
  of median paired overhead per paragraph <=max(5% baseline median per paragraph,
  2 microseconds). Suppress ratios below 100 times larger period/resolution;
  timer-limited primary cells cannot pass. Complete sampling and correctness are
  mandatory. Any failure/uncertainty means retention is not established; no added
  samples, tuning or substitute optimization. Archive and read back every file/hash.

The sibling `candidate-scan-runtime` reuses the deterministic block generator and
runtime driver's exact timing boundaries. Both arms are exhaustive ownership
sources until the clean patch is applied to retained only. Binary symbol/source
checks exclude diagnostic selectors, phase counters and shadow evaluation. Fixed
serial schedule, environment (including floating-point state), preflight audits,
raw execution journal and historical gate journal are separate checkpointed records.
Each process pair is the resampling unit; the same bootstrap indices apply to all
four operations. Percentile intervals are descriptive per cell, never a pooled
headline speedup. Absolute overhead gates remain active when ratios are suppressed.
RSS is whole-process memory, reported alongside the verified layout-local scratch
bound; preprocessing/allocation costs are included without separate attribution.

## Cache-aware runtime continuation (CC1–CC4, approved 2026-11-07)

The owner's request to implement “Resume rendering performance qualification”
approves these requirements, design and tasks before implementation. Preserve the
terminal CR study and all historical identities. Only the sibling
`candidate-scan-continuation` harness changes; no renderer/public API changes,
production integration, visual acceptance, fractional-DPR qualification, Vulkan
shutdown diagnosis, merge or publication are authorized.

- CC1: First reproduce small-prose initial layout in both arms with the existing
  paragraph-cache checker. Record paragraph, phase, metadata presence, cache
  events and shaping delta. Stop with the precise counterexample if reuse does
  not explain the failure.
- CC2: Metadata-bearing initial layouts shape once; metadata-free initial layouts
  either shape once with a cache miss or restore with a hit. Width/repeat changes
  never reshape. Metadata identity changes reshape affected paragraphs exactly
  once and leave others unchanged. Enforce this in C++, Python and completed-run
  validation with paragraph/phase/expected/observed errors. Correctness verifies
  cache events and exact paired geometry/shaping plus historical comparisons.
  Timing retains lifecycle assertions and matches correctness shaping signatures,
  with no cache checker or geometry queries in timed processes.
- CC3: After diagnostic/regression gates, predeclare the unchanged 22 fixtures,
  patches, operation boundaries and 88 correctness then 1,760 timing schedule in
  new build/evidence directories. Preserve serial alternating pairs, inherited
  locking, 60-second deadline, 4 GiB sampled RSS and terminal failure/no retries.
  Preserve CR5 thresholds and missing-sample/timer-limited rejection unchanged.
- CC4: Cover misses/hits, repeated paragraphs, zero/padded metadata, reuse and
  invalidation, invalid deltas and arm disagreement; run historical extent/geometry,
  recovery/supervisor regressions, lightweight and whitespace checks. Validate
  completed resume with fatal launch sentinels; archive and read back identities,
  raw results, reports and hashes. Report only isolated cutoff qualification.

## Candidate-scan diagnosis design (approved 2026-11-08)

Implement the owner's approved diagnosis plan in sibling `candidate-scan-diagnosis`
with separate outputs/evidence. First verify every continuation archive and member,
then schedule/raw/fixture and existing source/binary identities. Stop on any
integrity discrepancy and recommend evidence repair without regenerating inputs.
Only after integrity passes, run unchanged reporting against an isolated copy and
require exact archived summary agreement. Describe every pair and cell with raw
arm timings, paragraph normalization, differences, median/MAD/range/sign counts,
order and early/late medians, timer limits, duration and RSS. Keep all observations
and original gates. Order is coupled to parity; drift comparisons are exploratory.
Inspect affected source paths and retain nm/objdump versions, commands, symbol
sizes/disassembly; static differences cannot establish runtime slowdown. Recommend
path isolation for plausible added work, calibration for variability without added
work, or calibration first for mixed evidence. No proposed experiment runs here.

## Candidate-scan calibration design (approved 2026-11-08)

### Decision and scope

Approved on 2026-11-08 by the owner’s supplied-plan implementation request, together
with CB1–CB4 requirements and CB-T1–CB-T5 tasks. No broader experiment is authorized.

The diagnosis identifies additional compiled false-padding work, but every failed
interval includes zero and order/chronological summaries vary. Calibrate before
attributing a patch's apparent improvement. This is the smallest next measurement:
two fixtures, one existing binary, no C++ edits or compilation. It implements the
calibration prerequisite of the diagnosis recommendation; path isolation remains
a subsequent experiment, not an automatically authorized stage.

Use sibling `demonstrator/candidate-scan-calibration`, with fresh output and evidence
directories. Reuse the existing supervisor and output/lifecycle validators via a
small adapter. Do not modify the continuation or diagnosis tools or their inputs.
Both A and B execute the verified continuation `baseline-build/runtime-trace` with
identical manifest, zero padding and timing mode; labels live in the journal only.
Require equal binary/driver/fixture hashes and command arguments across each pair.
Existing source paths must match recorded identities. Stop on a mismatch; do not
rebuild or regenerate historical inputs.

### Fixed execution

Before launch, record tool/source/binary/fixture identities, inherited CPU affinity,
host/environment snapshot and the complete schedule. Use deterministic Python
`random.Random(73076)` and archive the generated schedule as the authoritative
input. For each fixture, construct five consecutive blocks of four sample rounds;
each block contains two A-first and two B-first pairs, shuffled independently.
Within each round, shuffle the order of the two fixture pairs. Each pair remains
adjacent and execution is serial. This breaks deterministic parity/order coupling
while retaining local balance; it does not eliminate chronological confounding.

First launch one correctness process per fixture (two total), comparing the full
geometry and shaping output against the verified historical baseline correctness
record for the same fixture/variant. Validate cache reuse and metadata lifecycle
with the existing checker. Then execute twenty timing pairs per fixture: eighty
timing processes, eighty-two processes total. Each process retains initial,
width+17 reflow, unchanged repeat and metadata operations. Timing has no geometry
queries/checker and must match the correctness shaping signature. Preserve null
prose metadata records. Primary calibration interpretation concerns only prose
reflow and dense metadata; all sixteen fixture/operation/arm combinations remain
visible in raw outputs and all eight operation cells are summarized.

Keep inherited exclusive locking, serial launch, 60-second process deadlines and
4 GiB sampled RSS ceiling. No CPU pinning, governor changes, forced cache flushing,
warm-up additions, retries or optional extra samples. Record stderr, status,
duration, sampled/high-water RSS, timer metadata, affinity and exact commands.
Any process, correctness or integrity failure makes the study terminal; no partial
calibration decision. Resume may restore completed immutable records into a new
directory after hash validation, but cannot retry a failed process. Before execution,
test recovery and completed-run no-launch resume with fatal launch sentinels.

### Statistics and interpretation

For each fixture/operation, retain all twenty pairs and compute
`(B.elapsedNs - A.elapsedNs) / applicableParagraphs`. Null operations remain null.
Use whole-pair percentile bootstrap, 10,000 resamples, seed 73077, with identical
resampling indices across operations of each fixture. Report median, unscaled MAD,
range, sign counts, 95% interval, per-arm timings, order medians and first-ten versus
last-ten summaries. Keep the historical timer rule: suppress ratios when either arm
median is below 100 times the larger recorded clock period/empirical resolution.
Absolute differences remain visible. No new pooled headline or causal claim.

For the two target cells only, compare the entire overhead interval against the
symmetric reference envelope `[-L, +L]`. Freeze L to the original archived
nonprimary overhead limit: prose reflow 4,819.525 ns/paragraph; dense metadata
101,542.075 ns/paragraph. Validate these exact values against the archived summary
before launch; do not round values for decisions. This is a new diagnostic
precision criterion, not a change to the original one-sided candidate gate and
not a proof of equivalence. Do not subtract calibration estimates from historical
candidate observations or combine separate runs as matched pairs.

- Integrity/correctness/execution failure: stop and report the precise problem.
- Either target is timer-limited or its interval is not wholly inside its reference
  envelope: calibration is unresolved at the historical budget. Propose a separate
  measurement-design revision before drawing patch-cost conclusions or performing
  another qualification. Do not tune or extend this run.
- Both target intervals lie inside their envelopes: the sampled baseline control
  resolves those reference budgets on this run only. Propose a separately approved
  path-isolation experiment, comparing original baseline, unchanged candidate and
  a candidate with padded-only scratch/guard setup, with contemporaneous controls.
  Scope the patch around verified false-padding behavior and preserve padded-path
  semantics; do not promise instruction identity or infer candidate retention.

Even a resolved control cannot show that historical variability had the same cause
or magnitude. Actual CPU placement, scheduling, frequency and thermal behavior
remain unmeasured; order and drift summaries are exploratory.

### Verification and delivery

Test exact A/B binary identity, fixture identity, schedule determinism/balance and
adjacency, all eighty-two launches, pairing, nulls, normalization, timer suppression,
bootstrap boundary equality, envelope decisions, extreme samples, failed/missing
records and raw-hash rejection. Validate immutable resume and fatal launch sentinels.
Run focused tests and repository lightweight/whitespace checks. No shipping build
is needed because renderer/build inputs do not change. Archive tooling, input
hashes, schedule, environment, journals, raw outputs and all reports; read back all
members/hashes and verify historical preservation. Record actual results separately
in verification.md. No commit, merge, publication or production integration.

## Calibration revision design (MR1–MR4, approved 2026-11-08)

Approval: the owner's supplied-plan implementation request authorizes this design
and MR-T1–MR-T5. Implement a sibling `candidate-scan-calibration-revision` tool
with separate output/evidence directories, retaining supervisor, lock, strict raw
parser, lifecycle checks and single-correctness-reference adapter.

Use random.Random(73078): in prose-480-4096 then dense-240-4096 order,
generate ten independently shuffled [ABBA, ABBA, BAAB, BAAB] blocks for each
fixture; then shuffle fixture order in each of forty rounds with the same RNG.
Keep each four-process group contiguous. Launch one correctness process per
fixture first. All arms use the verified continuation baseline and original
fixture paths; retain initial → width+17 reflow → unchanged repeat → metadata.
Persist authoritative schedule, generator/runtime identities, exact commands,
environment fingerprints and input hashes before launch. Durable attempted launches
are terminal on ambiguity/failure. Resume only a validated completed prefix into
a fresh directory, never repeating an attempted launch.

For each group require equal applicable paragraph counts in all four processes.
D = ((B1 + B2) − (A1 + A2)) / (2 × applicableParagraphs).
Preserve prose metadata nulls. Central estimate is median(D) across forty groups.
Bootstrap entire groups 10,000 times, seed 73079 reset per fixture, indices shared
across operations; percentile index int(p × (N − 1)). Record raw observations,
group differences, unscaled MAD, range, signs, 95% intervals, per-arm timings,
ABBA/BAAB and first-20/last-20 summaries. Each arm timing is its two-process mean.
Report median relative improvement; suppress ratios if either median arm group
timing is below 100 times maximum recorded clock period/empirical resolution.

Load exact prose reflow and dense metadata limits from continuation summary. A
target resolves only if not timer-limited and its entire interval is inclusively
within [-L, +L]. Either failure means unresolved calibration: stop and recommend
reconsidering measurement procedure before another experiment. Both resolved means
recommend a separately approved path-isolation experiment with baseline, unchanged
candidate, isolated candidate and contemporaneous controls using this grouping.
Retention remains not established. Never pool estimates with historical median
paired differences or claim historical variability explained. Balanced groups
cancel ideal linear execution-position effects at equal spacing, not necessarily
real elapsed-time drift. Forty groups are a budget, not a power/independence
guarantee; CPU placement, frequency, thermals and scheduling remain unmeasured.

## Workstation measurement procedure update (2026-11-08)

The owner accepted the characterization direction with a same-machine constraint:
these experiments always run on this development workstation, which is not a
dedicated benchmark host. See the [measurement procedure](measurement-procedure.md)
for execution conditions, preparation/cache semantics, separate fresh-process and
batched modes, whole-group analysis, telemetry and evidence requirements.

This updates future measurement direction; it does not amend historical CB/MR
protocols or archives. Available telemetry is observational, not proof of isolation.
Inherited affinity/frequency policy is the default; no quiet-host prerequisite or
system-wide tuning. Exact pilot sizes/counts, timing objectives, selection rule and
next calibration budget remain implementation-plan decisions requiring approval.

## Offline workstation characterization design (approved 2026-11-08)

WC1–WC4 implement MP-T2 and the proposal portion of MP-T3. A new sibling
`candidate-scan-characterization` imports unchanged historical validators/report
code under fatal process-launch sentinels. It extracts verified archives into fresh
output, validates closed journals and reconstructs the revision report in a scratch
copy before diagnostics. No historical directory is an output destination.

Use integer nanosecond start/end arithmetic, deriving midpoint as start plus half
the duration. Group elapsed time is its four-process mean midpoint relative to the
first timing-process start across the revision. Imbalance is mean B process midpoint
minus mean A process midpoint; gaps are successive starts minus preceding ends.
These are parent-observed process timestamps, never operation timestamps.

Retain all 320 group-operation rows, 1,280 process-operation rows and eight cell
summaries in JSON and CSV. Normalize operation time by applicable paragraph count;
null metadata remains null. Spearman is Pearson correlation of average tied ranks;
constant or wholly null vectors yield null, and mixed null vectors are rejected.
Order and first/last twenty groups remain per fixture; no causal inference or new
inferential procedure is added. Historical bootstrap is reproduced solely to check
exact existing results.

Generate a data-only draft pilot schedule using Python Random(73080): construct two
independently shuffled balanced four-order blocks for each configuration in fixed
configuration order, then shuffle configurations per round. Serialize 297 command
specifications without creating or running a harness. The draft protocol defines
selection over all 32 timing processes per telemetry-enabled batch, and matched-round
instrumentation comparisons. Its implementation and execution require later approval.

## Fixed batching pilot — approved 2026-11-08

The owner's supplied-plan implementation request explicitly approves the requirements,
design and tasks in [batching-pilot-proposal.md](batching-pilot-proposal.md), the
isolated Release harness build against verified existing baseline libraries, and
one fixed 297-process pilot (nine correctness, 288 timing). This supersedes the
proposal's historical draft approval status only. Preserve all prior work and
evidence. No renderer/public API changes, candidate comparisons, later calibration,
commits, merges or publication are authorized. Retention remains **not established**.

## Approved fixed batching refinement — 2026-11-08

The supplied-plan implementation request approves the requirements, design and tasks
in [the refinement protocol](batching-refinement-protocol.md), including its exclusions.
Historical 297-process pilot and no-selection result remain unchanged.

## Approved prepared-state calibration — 2026-11-08

The owner’s supplied-plan implementation request explicitly approves PC1–PC5 and
PC-T1–PC-T5 in [the calibration protocol](prepared-calibration-protocol.md). This
grants one 483-process calibration at size 96 and evidence delivery only. Earlier
exclusions of later calibration remain historical; this approval is bounded to
this new run. Retention remains **not established** under every outcome.

## Prepared-state path-isolation proposal — draft, 2026-11-08

The owner approved designing the next experiment and committing the completed
calibration (“both” after those two explicit choices). Calibration commit is
`f7879b3`. The [new proposal](prepared-path-isolation-proposal.md) defines draft
PI1–PI6 requirements, source/build design and PI-T1–PI-T6 tasks, with a proposed
2,001-process budget and contemporaneous controls. Only protocol drafting is
authorized. Implementation/build/execution approval is pending; no next-experiment
sample has been collected. Retention remains **not established**.

## Prepared-state path-isolation implementation approval — 2026-11-09

The owner's supplied-plan implementation request approves PI1–PI6, the design,
PI-T2–PI-T6 and the fixed 2,001-process budget in the
[path-isolation protocol](prepared-path-isolation-proposal.md). Historical approvals
and evidence remain intact. Execution is conditional on every preflight gate;
failed or ambiguous attempts are terminal. Evidence delivery is authorized;
commits, push, merge, publication and production integration are excluded.
Retention remains **not established**.

## Architecture decision review — approved 2026-11-09

Use `ba843e0` as the frozen evidence cutoff. Put the current decision first in
[migration.md](migration.md#current-decision--evidence-through-ba843e0), followed by
the preserved historical record with its superseded status labelled. Map R1–R8
to explicit subgate statuses, configuration, evidence and remaining limitations.
Resolve apparent conflicts by date/mechanism/scale; preserve genuine blockers.
Read machine-readable summaries and archived frozen sources when identity matters;
do not execute historical runners or regenerate analyses. Compare public Qt
composition, patched Skia and continued shipping-renderer use. Propose one bounded
diagnostic-design milestone targeting comparable-scale prerequisites; approval
for this review does not approve that milestone or any executable protocol.

## Coordinated four-scale study — approved 2026-11-09

The supplied-plan implementation request approves SS1–SS5, the design and
SS-T1–SS-T5 in the [study protocol](scale-study-protocol.md), including the fixed
690-process ceiling. This supersedes the proposed design-only milestone; all
historical approvals and evidence remain unchanged.

## Corrected complete scale study — approved 2026-11-09

The owner's “Well, let's proceed” approves correcting the harness and running a
new complete study, following the explicit proposal of 691 total graphical launches:
one preserved failed attempt plus a fresh 690-process schedule. This supersedes the
suggested untouched-cell continuation. The original terminal run and frozen sources
remain immutable. Renderer/platform failures retain the existing independent-cell
and dependent-recreation rules; no additional adaptive samples are authorized.

Requirements: SS1–SS5 remain unchanged except the explicit replacement study budget.
Design: use a sibling `scale-study-v2` supervisor with both wl_output names and
xdg-output object bindings; reject conflicts, missing bindings and wrong outputs.
Persist exit, elapsed time, sampled peak RSS and capture hashes before validation;
record a failed result and journal completion even if validation raises. Reuse the
byte-identical verified viewers/libraries; no renderer or instrumentation rebuild.
Reserve the original failed attempt's 120 seconds conservatively against the same
four-hour automated budget, excluding build time and human waits.

Tasks: test the actual retained trace and negative identity cases; test failure
recording and terminal recovery; verify original evidence and reused binary hashes;
freeze a separately identified full schedule; coordinate all four scales and live
transitions, validate/archive results and restore the original scale. No commits,
publication, performance qualification or backend selection are authorized.

## Atomic live handoff repair and bounded continuation proposal — 2026-11-09

The v2 live supervisor stopped at live-5 when it observed a confirmation file
between creation and JSON write. All fresh sets and live-0 through live-4 remain
immutable. The owner restored 125%; read-back matched the original output.
Fixing the harness within the approved study is authorized; replacing terminated
windows is a separate, concrete execution amendment pending owner approval.

Repair requirements/design: write a sibling temporary file, serialize/flush/fsync
it, atomically publish using a no-overwrite hard link on the same filesystem, sync
the directory, and remove the temporary name. Final filenames are never exposed
with partial content; duplicates must fail without changing existing evidence.
Test the original race, a deliberately paused writer, concurrent publishers,
serialization/fsync/link failures and duplicate rejection without graphical runs.
Reuse the frozen v2 supervisor and byte-identical viewers via an isolated wrapper;
change only publication and the recovery schedule/budget enforcement.

Proposed execution: ten new persistent bidi-objects windows, initial desktop 2,
then 1.5 → 1.25 → 1 → 1.25 → 1.5 → 2, with the original checks/limits. No fresh
matrix rerun. Prior actual launches are 675 (one original, 664 v2 fresh, ten v2
live); the proposal totals 685, within the ceiling of 691. This explicitly replaces
ten live attempts; unused fresh recreation slots do not silently grant approval.
Conservatively reserve 120 seconds for each historical harness failure alongside
completed fresh automated time within the unchanged four-hour budget.

Tasks: preserve/validate terminal evidence and restoration; implement/test atomic
publication; prepare/hash a ten-cell continuation with no-launch preflight; obtain
explicit approval of that replacement scope; execute once, restore 125%, archive
and reconcile complete versus failed/untested coverage. No publication or commit.

Owner approval (2026-11-09): **“Proceed with ten replacement windows”** explicitly
approves the prepared live-only continuation above, its atomic handoff repair and
ten-cell frozen schedule. No fresh-window rerun; total actual launches after this
continuation are bounded to 685 within the existing ceiling of 691. All other
checks, exclusions and restoration requirements remain unchanged.
