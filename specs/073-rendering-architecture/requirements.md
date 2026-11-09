# 073 — QMarkdown 2.x rendering architecture

## Approval and scope

The owner's 2026-11-05 instruction to implement the supplied architecture plan
approves this evaluation, its design and mapped tasks. It does not approve a
production rewrite, API change, release or backend selection without evidence.
Feature 065's existing experiments and working-tree changes remain historical.

- R1: Audit layout, shaping, padding, transforms, allocation, sampling and display.
  Isolate defects before attributing them; retain the overlapping historical scene.
- R2: Independently evaluate public Qt distance-field, Qt curve and native Skia
  controls with matching fonts/content/scales. Browser output is a visual reference.
- R3: Prove padded code, mixed formatting and inline-object placement without
  selection-foreground emission, private Qt APIs, QTextDocument or WebEngine.
- R4: Keep glyphs, backgrounds, decorations, selection and clips explicit; cache
  preparation/resources, allowing bounded raster work with measured costs.
- R5: Deadline initialization/rendering stages. Qualify Linux Wayland OpenGL and
  Vulkan at observed DPR 1, 1.25, 1.5, 2; resting, slow motion, flick, reversal,
  settling, resize and scale changes. Software/other platforms are deferred.
- R6: Cover shaping, fallback, overhang, rounded code, rules, tables and objects.
  Inspect actual-source images/motion; exact replay is a separate correctness gate.
- R7: Separate capture from pacing; measure CPU/GPU/preparation/memory and hardware
  refresh. Retain the existing p95 <= one refresh / maximum <= two refresh budget.
- R8: Select only after both native APIs pass quality and performance. Otherwise
  record blockers and unresolved decisions without authorizing a rewrite.

See [design](design.md), [tasks](tasks.md), [verification](verification.md).

## Approved demonstrator continuation — 2026-11-05

The owner explicitly requested implementation of the supplied “Build the native
rendering comparison demonstrator” plan, then asked to retry. That instruction
approves this bounded continuation, including requirements, design and tasks.

- D1: Shared source-indexed fixtures, explicit fonts, padded/adjacent code, mixed
  formatting, wrap, bidi, ligatures, marks, fallback, overhang, objects and document.
- D2: Independent Qt distance/curve controls; cached public nodes where supported,
  labelled physical-phase raster fallback using isolated prepared geometry elsewhere.
- D3: Standalone SDL2 Wayland Skia Ganesh GL/Vulkan with independently shaped
  SkParagraph; pin official source/dependencies outside shipping dependencies.
- D4: Browser visual references and source-preserving selection; unsupported
  reservation semantics must retain a reproducer and explicit status.
- D5: Vulkan reduction cases, observed exposure/frame readiness, robust capture
  errors, monotonic records, pre/post hashes and external 20/10/10-second deadlines.
- D6: Interactive movement, replay, resize, scale, clipping and recreation controls;
  actual DPR matrix on both exact Qt SDKs and both native APIs. Owner visual
  acceptance remains separate from automated checks.
- D7: Retain captures and support matrix, preparation/resource/dependency costs.
  Performance qualification, backend selection and production migration follow later.

Shipping APIs, dependencies and behavior remain unchanged. No merge or publication
is authorized by this milestone.

## Bounded correctness continuation — approved 2026-11-05

The owner explicitly approved these updated requirements, design and mapped
tasks on 2026-11-05 ("approved"). Approval covers C1–C6 only; prior demonstrator
approvals and failures remain unchanged.

- C1: Reproduce `bidi-objects` code-edge selection on exact Qt 6.8.0 and 6.12.0.
  Record prepared cursor cells, line membership, visual direction and hit tests.
  Correct only isolated staged preparation, with one mapping shared by interactive
  selection and geometry tests. Exact copied source must remain `code`.
- C2: Test padding on every visual Skia code fragment, vertical reservation,
  object placement and exact source copying. Trial per-fragment reservations in
  the existing whole-paragraph SkParagraph layout, with at most eight layout
  passes and explicit cycle/non-convergence rejection. Preserve UTF-16 mappings.
- C3: Isolate minimum-Qt Vulkan shutdown using the existing empty-window case.
  Compare default/basic render loops and scene-graph release while the event loop
  remains active, retaining teardown order and bounded backtraces on both SDKs.
  Adopt a harness correction only with preserved backend and successful normal
  shutdown and recreation evidence.
- C4: Begin with CPU/offscreen geometry on both exact SDKs and pinned Skia.
  Cover original failures, widths immediately around wrapping transitions,
  adjacent code, mixed RTL/LTR, combining marks, surrogate pairs and objects,
  with both selection directions and exact copied source.
- C5: After geometry passes, run affected fixtures serially on OpenGL/Vulkan,
  both Qt renderers and observed Qt DPR 1, 1.25, 1.5 and 2. Skia uses only its
  observed DPR; unavailable scales remain explicit. Check stationary capture,
  selected text, resize/reset, motion/replay and three process recreations.
  Shared layout or lifecycle changes require the complete demonstrator matrix.
- C6: Preserve historical failures and input/binary/font identities; append
  before/after results, update support and review/migration records, and run
  supervisor tests, repository lightweight checks and whitespace validation.
  Close gaps only with passing behavior; otherwise name the failing case,
  attempted correction and capability required next.

No shipping API, dependency, renderer, version or release changes are proposed.
Broader layout replacement, production fixes, cross-paragraph selection, general
tables, performance qualification and backend selection remain outside scope.
Owner visual acceptance, merge and publication are separate authorization gates.

## Fragment-aware upstream experiment — approved 2026-11-05

The supplied continuation plan selects an isolated upstream patch experiment.
The owner explicitly approved F1–F7, the updated design and mapped tasks on
2026-11-05 ("approved"). This approves the new isolated mechanism and its bounded
feasibility evaluation, not production adoption or a successful outcome.

- F1: Stage a separate copy of pinned Skia revision
  `00227028b0c3cb3052432a456af2027a49e598b2`; retain the unpatched control.
  Apply a checked reproducible patch, recording base, patch, dependencies, build
  configuration, tools, libraries and executable/output hashes.
- F2: Express padding as source-span metadata with distinct adjacent-span
  identities. Convert UTF-16 fixture intervals at one validated SkParagraph
  offset boundary; reject invalid and grapheme-splitting ranges. Do not insert
  padding characters or padding placeholders. Retain source-mapped inline objects.
- F3: Extend upstream line breaking and formatting using existing shaping,
  Unicode opportunities, bidi resolution and emergency wrapping. Candidate fit
  includes both horizontal edges of every contiguous visual code fragment.
  Use finite candidate scans with monotonic source progress. Preserve indivisible
  graphemes/objects and report unavoidable overflow for an oversized padded unit.
- F4: Store adjusted positions and vertical reservations in authoritative
  paragraph layout. Painting, rectangles, caret hit testing and objects consume
  those positions. Preserve shaped glyph identities and intracluster advances.
  Diagnose object baseline rounding against actual painted bounds and retain the
  existing 0.05 logical-pixel containment tolerance.
- F5: Pass disabled-padding equivalence against the unpatched pinned control,
  then the existing 751-case geometry scan plus fractional transition probes,
  then consistency checks, before serial native OpenGL/Vulkan runs at observed
  DPR. Preserve unavailable DPRs and owner visual acceptance as separate gaps.
- F6: Retain representative before/after images, an interactive launcher,
  reproducible commands, regression checks and evidence identities. Preserve the
  rejected placeholder trial and all historical failures; update support and
  migration records only from actual results.
- F7: Report feasible only when every automated gate passes, with patch upkeep
  costs and visual acceptance outstanding. Otherwise retain a minimal failing
  case, attempted mechanism and precise missing capability. Stop executable work
  if it requires replacing upstream shaping, bidi or the line-breaking engine;
  document that architectural change for a subsequent decision.

No shipping API/dependency change, Qt-control change, backend selection,
performance qualification, Qt integration, general tables, cross-paragraph
selection, production migration, merge or publication belongs to F1–F7.
The Qt 6.8 Vulkan shutdown failure remains an independent blocker.

## Explicit shaping-boundary continuation — approved 2026-11-05

The supplied continuation plan selects code-span edges as shaping boundaries.
The following B1–B8 revise F1–F7 for the next isolated experiment and explicitly
supersede F4's cross-span glyph-preservation constraint and the corresponding
F2/F4 cluster-edge stop condition. Historical approvals, commit `d3b1f05`, its
diagnostic patch and all failures remain intact. The owner explicitly approved B1–B8, the revised design and mapped tasks on
2026-11-05 ("approved") before implementation.

- B1: Create a new isolated stage of pinned Skia revision
  `00227028b0c3cb3052432a456af2027a49e598b2`. Retain the unpatched control and
  previous experiment. Record checked patch application, source/dependency
  identities, build configuration, tools, fonts and input/output hashes.
- B2: Carry distinct span identities and validated UTF-8 boundaries into upstream
  shaping. Validate UTF-16 fixture ranges against grapheme boundaries, preserve
  complete object-source intervals and original source text, and reject malformed
  ranges. Insert no padding characters or padding placeholders.
- B3: Split shaping regions at every code-span start/end, including adjacent
  identically styled spans. Ligatures and Arabic joining may change across edges;
  shaping within each region remains upstream-owned. Retain paragraph-wide bidi,
  script/font fallback and original Unicode breaks. Edges do not create breaks.
- B4: Bypass paragraph shaping-cache reads and writes whenever boundary metadata
  is present, including zero padding. Metadata changes invalidate shaping/layout;
  width changes reuse shaped results. Preserve the metadata-free control path.
  Cache optimization is deferred.
- B5: Share contiguous visual-fragment calculation between upstream candidate fit
  and formatting, charging both edges of each fragment. Preserve adjacent span
  identities and disjoint bidi fragments. Use finite scans with monotonic source
  progress, upstream legal-break priorities and emergency wrapping; preserve
  indivisible units and report unavoidable overflow. No paragraph convergence or
  replacement line-breaking engine is permitted.
- B6: Store translations and vertical reservations in authoritative paragraph
  geometry shared by painting, rectangles, caret hit tests, selection and objects.
  Resolve containment from complete text/object/padding extents; apply baseline
  adjustments to all line content and reserve rounded line/paragraph height.
  Keep the 0.05 logical-pixel containment tolerance.
- B7: Pass boundary semantics, separate equivalence baselines, all 751 existing
  geometry cases plus boundary regressions and 1/64-pixel transition probes,
  then consistency checks before native gates. Verify exact copying in both
  directions, authoritative hit testing, metadata/width/repeated-layout behavior,
  and `objects@679` raw/rounded metrics and logical/painted bounds.
- B8: After geometry passes, run serial native GL/Vulkan at observed DPR with
  stationary/selected captures, resize/reset, motion/replay and three process
  recreations. Retain deadlines, identities, matched captures and an interactive
  patched viewer launcher; report unavailable DPRs. Run supervisor regressions,
  repository lightweight checks and whitespace validation. Declare feasible only
  after all automated gates pass; otherwise retain a reproducible minimal failure
  and precise missing capability and stop dependent gates.

All new interfaces and diagnostics are private to the experimental adapter and
patched SkParagraph. Document upstream touchpoints and patch upkeep costs.
Owner visual acceptance stays separate. This continuation does not select a
production backend or qualify performance. Qt controls remain unchanged and the
Qt 6.8 Vulkan shutdown failure remains independent. Production dependencies,
Qt integration, general tables, cross-paragraph selection, merge and publication
remain outside scope.

## Measurement continuation approved 2026-11-06

The owner explicitly requested implementation of the scale diagnosis and performance
qualification plan. This approves P1–P6 below, their design and mapped tasks. Visual
review is accepted only for the cases the owner inspected; no additional output,
scale, API or fixture configuration inherits that acceptance.

- P1: Diagnose the unchanged 1.25 desktop on the same output, serial GL/Vulkan,
  zoom 1. Retain SDL version/flags, window and drawable dimensions, both ratios,
  output identity and separate Wayland protocol evidence (output/buffer scale,
  geometry, buffers, viewport and fractional events). Separate desktop scale,
  surface mapping, buffer density and zoom. Unknown mapping is a qualification gap.
- P2: Compare clean upstream, boundaries with zero padding, and boundaries with
  existing padding using identical sources, fonts, styles, widths and objects.
  Reuse validated construction and pinned builds. Keep historical runners intact.
- P3: Generate grapheme-safe prose, dense spans, mixed bidi/marks/fallback, long
  code and objects at 1/4/16/64/256 Ki UTF-16 units, widths 240/480/960; sweep
  1/8/32/128 spans at fixed 16 Ki and fixed coverage; documents 10/100/1000 ×
  approximately 256 units. Separate construction, first layout, width reflow,
  repeated layout and metadata invalidation. Use 10 fresh processes (not cold
  filesystem caches), five warmups and 30 samples; diagnostic counters separate.
  Limit cases to 60 seconds and 4 GiB RSS, preserve partial data, skip larger
  sizes in failed series and continue independent series.
- P4: Serial native three variants × two APIs × typical/dense/large documents ×
  stationary/slow/flick-reversal/settling/resize/selection, three repetitions,
  two seconds warmup and five measured. No captures, trace or frame console I/O
  during timing. Separate CPU layout/paint/submission/swap, asynchronous supported
  Ganesh GPU elapsed time, Wayland presented/discarded feedback, RSS and GPU cache.
  Preserve traversal; no culling/caching/layout optimizations.
- P5: Validate fixtures, aggregation/nearest-rank p95, deadlines, missing metrics,
  scale consistency, instrumentation equivalence/source mapping and invalidation.
  Capture outside timing; retain source/font/patch/library/executable/config/output
  hashes. Report raw samples, median/p95/max, growth ratios and static charts.
  Apply existing pacing target only to actual presentation intervals with precision,
  per scenario/repetition; missing scale/GPU/feedback never passes qualification.
- P6: Report observed bottlenecks and prioritized proposals, without new initial
  layout/memory thresholds or asymptotic proof. Run supervisor and lightweight
  regressions and whitespace checks. Preserve Qt 6.8 Vulkan shutdown blocker.

No SDL migration, compositor change, competing scale/viewport objects, shipping
change, Qt integration, optimization, backend selection, merge or publication.


## Approved recovery — 2026-11-06

Recovery R1–R5: preserve the completed CPU sweep, 323 native processes, 18 captures, historical gates and 59 clean diagnostic comparisons. Exclude overlapping diagnostics from conclusions. Finish only outstanding diagnostics, upstream counters, post-reboot scale/environment, one closed-window retry and the corrected ten-paragraph CPU smoke. Preserve timeout/skip outcomes, four feedback gaps, strict pacing and the independent Qt 6.8 Vulkan blocker.

## Approved phase diagnosis (2026-11-06)

The owner's supplied implementation plan authorizes D1–D4: a bounded diagnostic
study, not optimization implementation. Preserve commit `4b9fd4c`, historical
evidence and qualification gaps. Attribute initial, +17 width reflow, unchanged
repeat and fresh metadata identity invalidation separately. Count candidate and
whitespace visits, run visits, owner comparisons, reorder calls and emitted slices,
splitting fitting from formatting/traversal. Compare every operation against an
uninstrumented trace for the twelve specified fixtures, zero and padded. Retain
60-second/4 GiB limits, inherited exclusive lock, validated new-directory resume
and terminal outcomes. No public API, rendering or production dependency changes.

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

## Approved ownership runtime study — 2026-11-06

The owner instruction to implement the supplied Feature 073 runtime plan approves
RT1–RT4 below, the design and tasks in this section's sibling documents.

- RT1: Compare original boundary and retained ownership patch with identical uninstrumented drivers, pinned sources, release flags, dependencies and fonts.
- RT2: Require 48 fresh correctness processes and historical gates before 480 fresh timing processes: twelve fixtures × zero/padded × ten adjacent pairs. Four operations are initial, width +17, repeat and first-span identity mutation/layout; metadata-free results are null.
- RT3: Preserve exclusive inherited lock, 60-second deadline, 4 GiB sampled RSS ceiling, durable terminal records and validated new-directory resume. Stop on geometry/shaping failure; other failures terminate that cell without retries.
- RT4: Archive provenance/raw evidence and report per-cell paired medians and descriptive 95% bootstrap intervals (10,000 whole-pair resamples), timer limits, zero-padding overhead and single-span behavior. No performance threshold, warm-cache claim, further optimization, native/Qt integration, backend choice, merge or publication.

## Approved candidate-scan proof — 2026-11-06

The owner request to implement the supplied Feature 073 plan approves C1–C5,
the design and C-T1–C-T5 below before implementation.

- C1: Preserve exact exhaustive endpoint choice, geometry, glyph positions, visual
  slices, mappings and shaping lifecycle; use the verified ownership baseline.
- C2: Prove and prototype strict suffix-minimum cutoff using exact enumeration and
  trimming through the first hard break. Keep first indivisible/emergency behavior.
- C3: Guard finite arithmetic, nonnegative finite horizontal padding, IEEE scalar,
  nearest rounding, no fast math and x86 FTZ/DAZ off; otherwise use exhaustive scan.
  Scratch is layout-local, bounded by cluster count, absent on zero-padding paths.
- C4: Run fixed serial 12 × 2 × 3 oracle/shadow/prototype diagnostics, four operations
  each, exact archived runtime comparison, historical gates, exhaustive short and
  10,000 seeded numeric cases. Retain inherited supervision and durable provenance.
- C5: Deliver proof, work/storage trade-offs and separately approvable recommendation
  or reproducible rejection. No retained optimization, runtime matrix, ownership
  changes, Qt integration, speedup claim, merge or publication.

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

## Candidate-scan diagnosis (CD1–CD4, approved 2026-11-08)

The owner's supplied-plan implementation request explicitly approves this bounded
continuation before coding. CD1: verify existing archives/member hashes, schedules,
raw hashes and binary/source identities; stop on missing or inconsistent evidence.
CD2: reproduce the unchanged report exactly and describe all 176 cells and twenty
pairs each, preserving nulls, timer limits and the eight failed overhead gates.
CD3: inspect verified sources and existing binaries without executing or rebuilding
benchmark arms; distinguish facts from runtime-cost hypotheses. CD4: test and
archive reproducible analysis, preserve historical/uncommitted work and recommend
only a separately approved follow-up. Retention remains not established. No new
measurements, thresholds, renderer/API changes, integration or publication.

## Candidate-scan calibration (CB1–CB4, approved 2026-11-08)

Status: planning only; implementation and execution are not approved. The owner's
“plan the next step” request authorizes this draft, not measurements. This is a
bounded follow-up within Feature 073. Retention remains not established.

- CB1: Verify the diagnosis/continuation archive hashes, required source and binary
  identities and fixtures before proceeding. Use the existing uninstrumented
  continuation baseline binary for both logical arms. No rebuild, renderer change,
  environment tuning or replacement of historical evidence is permitted.
- CB2: Measure baseline versus itself for two predeclared targets:
  `prose-480-4096 / zero / reflow` and
  `dense-240-4096 / zero / metadata`. These distinguish empty metadata from
  178 zero-padding spans. Use twenty adjacent fresh-process pairs per fixture,
  retain the existing four-operation sequence and preserve all outputs/nulls.
- CB3: Fix and archive a balanced randomized schedule before any launch, with
  independent logical arm labels A/B pointing to the exact same binary. Report
  paired B-minus-A overhead, uncertainty, timer limitations, order and chronological
  summaries without trimming, pooling, adding samples or changing historical gates.
- CB4: Fail closed on integrity, correctness, execution or completeness problems.
  Preserve bounded supervision, immutable journaling and archive read-back.
  Calibration can inform a subsequent experiment but cannot establish candidate
  retention or repair any of the eight historical failures.

Only this calibration is approved. A path-isolation patch,
new candidate build, direct candidate comparison and full qualification require
separate requirements/design/tasks and approval after the calibration is reviewed.

## Calibration revision (MR1–MR4, approved 2026-11-08)

The owner's supplied-plan implementation request approves these requirements,
the design below and MR-T1–MR-T5 before implementation. Retention remains not
established under every outcome. This is a new diagnostic, not historical repair.

- MR1: Preserve historical/uncommitted work. Verify continuation, diagnosis and
  calibration archives/member hashes, existing sources/binary, fixtures and exact
  archived limits before launch; stop on discrepancies without rebuilding.
- MR2: Use identical baseline arms, zero padding and unchanged four operations.
  Execute two correctness processes then forty contiguous ABBA/BAAB groups per
  fixture (322 total), serially with inherited affinity/environment, 60-second
  deadlines and 4 GiB sampled RSS. Freeze schedule, commands, identities and
  environment before launch; record parent monotonic start/end timestamps.
- MR3: Report the median of forty group differences, whole-group bootstrap and all
  eight cells, preserving nulls, timer suppression and exact inclusive envelopes.
  No pooling, historical gate revision, subtraction or explanation of historical
  variability is permitted. Either unresolved target ends this experiment.
- MR4: Test schedule, estimator, lifecycle, integrity and terminal recovery before
  measurement; verify deterministic reanalysis and no-launch completed resume;
  archive sources, observations, identities, journals and checks with member hashes.
  No warm-ups, retries, extra samples, tuning, renderer changes, builds, candidate
  comparisons, integration, commits, merges or publication are authorized.

## Workstation measurement procedure (MP1–MP4, direction accepted 2026-11-08)

The owner agreed to measurement characterization and requested a procedure update,
correcting the machine assumption: all experiments use this development workstation;
there is no dedicated machine. Approval covers this procedure direction, not an
unspecified pilot budget or an unreviewed harness implementation.

- MP1: Use the same workstation under ordinary shared use. Record machine identity,
  software/hardware changes and available execution telemetry outside timing regions;
  do not require exclusive access, idle conditions or privileged system tuning.
- MP2: Inspect existing evidence first, then plan a bounded pilot of genuine
  width-changing reflow batches with explicit preparation/cache policy. Keep
  fresh-process single-operation results separate; do not treat inner iterations
  as independent samples or substitute unchanged-layout calls for reflow.
- MP3: Freeze a new baseline/self protocol after characterization; retain balanced
  four-process groups, whole-group analysis, terminal recovery and historical
  evidence/gates. Neither batching nor use of the same machine establishes precision.
- MP4: Specify and approve finite pilot parameters, selection rule, instrumentation
  and verification tasks before implementation/execution. Record unavailable
  telemetry, preserve every attempted sample and stop at the fixed budget.

The [measurement procedure](measurement-procedure.md) defines the accepted direction.
Retention remains not established; historical calibrations remain closed.

## Offline workstation characterization (approved 2026-11-08)

The owner's supplied-plan implementation request approves WC1–WC4 and WC-T1–WC-T4
before implementation. It authorizes existing-evidence analysis and a pilot proposal,
not a harness, build, measurement, commit, merge or publication.

- WC1: Verify continuation, diagnosis, calibration and revision archives/member
  hashes, pinned identities, schedules and raw records. Fail closed; never regenerate
  missing inputs. Reproduce all eight revision cells and published estimates exactly.
- WC2: Preserve nulls and timer limitations. Derive parent process midpoint/duration,
  group span/gaps and mean B minus mean A midpoint imbalance; report chronological
  operation timings and group differences against elapsed time and imbalance.
  Include order/half summaries and descriptive Spearman correlations with average
  tied ranks and null constant inputs. No tests of significance, exclusions or causes.
- WC3: Deliver a decision-complete, separately approved future pilot proposal:
  zero-padding prose reflow; sizes 1/16/64/256 × telemetry off/on plus original
  reference; 297 processes; seed 73080; ordinary shared use; deterministic duration,
  variability and resource selection. Retention stays not established.
- WC4: Isolate sources/output/evidence, use fatal launch sentinels, verify arithmetic,
  integrity rejection, deterministic reproduction and schedule without execution;
  archive sources, input identities, diagnostics, protocol and checks; preserve history.

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

The owner's supplied-plan implementation request approves AR1–AR4, the review
design and AR-T1–AR-T4. Evidence cutoff: `ba843e0`.

- AR1: Reconcile R1–R8 against existing reports and machine-readable evidence,
  preserving configuration, limitations, historical approvals and genuine blockers.
- AR2: Publish a current decision and candidate comparison without backend selection
  or changing support, host styling, architectural boundaries or quality thresholds.
- AR3: Recommend one bounded next milestone by prerequisite order, explicitly
  proposed and awaiting approval; no future execution is authorized.
- AR4: Validate documentation/links, repository Light checks once and tracked/new
  whitespace. No measurements, regeneration, runtime tests, archive changes,
  implementation, commits, push, merge or publication.

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

## Correct scale mapping — approved 2026-11-09

SC1–SC4 define the approved bounded requirements.
The owner explicitly requested implementation of the supplied plan. See the
[approved protocol](scale-contract-protocol.md). Historical approvals remain intact.
