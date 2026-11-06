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
