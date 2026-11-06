# Mapped tasks

## Initial placement-stage record (historical)

Approval: owner supplied implementation request, 2026-11-05.

- [x] Audit the coordinate pipeline and preserve historical defect evidence (R1).
- [x] Implement and run the public Qt placement gate on both exact SDKs (R2–R3).
- [ ] Implement independent native Skia and browser reference controls (R2).
- [ ] Build authoritative padded/mixed/object minimal and document fixtures (R3–R6).
- [x] Implement stage deadlines and observed backend/scale identities (R5).
- [ ] Run motion/replay/resize/scale and visual review matrix (R5–R6).
- [ ] Measure presentation, CPU/GPU, preparation and memory independently (R7).
- [ ] Deliver selected architecture or precise blockers and migration decisions (R8).
- [x] Run local verification and record actual results separately.

The placement gate ran on both SDKs; Qt 6.8 Vulkan exit failures remain failed.
Completed task execution does not imply backend acceptance. Remaining tasks
include Skia, full-document composition, performance and final migration decisions.

## Approved continuation tasks (D1–D7)

- [x] Shared manifest and generated browser reference, preserving source mappings.
- [x] Isolated Qt prepared-geometry viewer, actual padding/objects, native/fallback labels.
- [x] Pinned independent SDL/Skia GL/Vulkan build and viewer.
- [x] Crash reductions and lifecycle audit with retained before/after evidence.
- [x] Stage-aware supervisor and failure-path tests, pre/post input identities.
- [x] Exact-SDK builds, available DPR/API capture/motion/replay/resize/recreation matrix; unavailable SDL fractional DPRs explicitly recorded.
- [x] Actual verification, support matrix, dependency costs and owner review commands.

These are planned checks; completion requires recorded execution. Final performance
qualification and production architecture selection are explicitly deferred.

Owner visual acceptance remains pending. Known unsupported cases are retained in the
continuation support matrix; successful process replay is not composition acceptance.

## Bounded correctness continuation — approved 2026-11-05 (C1–C6)

Owner approval received 2026-11-05 ("approved") for C1–C6, design and tasks.
Task completion requires actual implementation and verification records.

- [x] Inspect current demonstrator, staging and historical failures; draft C1–C6,
  bounded design and mapped tasks.
- [x] Record explicit owner approval and its scope in these feature documents.
- [x] Reproduce Qt bidi/code failure on both exact SDKs; retain cursor-cell,
  line/direction and hit-test traces (C1). Check unchanged exact copied source.
- [x] Implement a checked stage-only preparation patch if justified by traces;
  retain original commit and patch/output hashes, and share mapping between
  interactive selection and geometry tests (C1).
- [x] Extend geometry assertions and fixtures for every Skia code fragment,
  vertical reservation, objects, wrap-transition widths, adjacent spans, bidi,
  marks and surrogate pairs; check both directions and exact copy (C2, C4).
- [x] Trial whole-paragraph per-fragment reservations with stable-state checks,
  cycle rejection and an eight-pass limit; verify UTF-16 mappings and shared
  final painting/selection/hit-test layout. Retain a minimal reproducer and
  required-next-capability blocker if rejected (C2).
- [x] Run CPU/offscreen geometry on exact Qt 6.8.0, Qt 6.12.0 and pinned Skia;
  retain failures and passing evidence separately (C1, C2, C4).
- [x] Compare empty-window default/basic render loops and live-event-loop
  scene-graph release on both SDKs; record teardown order and bounded crash
  backtraces. Accept a harness change only after preserved Vulkan identity,
  normal shutdown and recreation checks; otherwise retain standalone failure
  reproducer (C3).
- [x] After geometry passes, run affected native fixtures serially across both
  APIs/renderers and observed Qt DPR 1/1.25/1.5/2, plus actual Skia DPR. Check
  stationary capture, selected text, resize/reset, motion/replay and three
  process recreations; run the complete matrix after shared layout/lifecycle
  changes (C5).
- [x] Append before/after verification and identities; update support matrix,
  owner review commands and migration blockers without overwriting historical
  failures. Identify failing case, attempted correction and capability required
  next for each unresolved gap (C6).
- [x] Run supervisor tests, repository lightweight checks and whitespace
  validation; record actual results and any unavailable checks (C6).

Owner visual acceptance, merge and publication are not included in implementation
approval. No production migration or backend selection is authorized here.


Bounded continuation execution is complete. Qt geometry passes on both SDKs;
Skia's per-fragment trial is rejected and minimum-Qt Vulkan remains failed.
The full matrix has 884 clean process passes and 272 retained shutdown failures.
Task completion records execution, not composition, backend or visual acceptance.
See the appended [actual verification](verification.md) for precise blockers.

## Fragment-aware upstream experiment — approved tasks (F1–F7)

- [x] Inspect pinned build, existing scan and historical blockers; draft F1–F7,
  design and mapped tasks while preserving historical records.
- [x] Obtain explicit owner approval of F1–F7, design and these tasks;
  recorded 2026-11-05 ("approved") before experimental implementation.
- [x] Stage a separate pinned checkout, checked patch/build procedure and
  independent output paths; retain control and full identities (F1).
- [x] Implement validated UTF-16/SkParagraph boundary mapping and distinct
  source-span metadata; test invalid/grapheme-splitting intervals (F2).
- [ ] Extend upstream candidate fit/formatting with shared visual fragments,
  finite scans, source-progress assertions and explicit oversized-unit overflow;
  stop if upstream engine replacement is necessary (F3, F7).
- [ ] Integrate authoritative fragment positions and vertical reservations into
  painting, rectangles, carets and objects; diagnose fractional baseline/paint
  bounds without changing containment tolerance (F4).
- [x] Pass disabled-padding control equivalence with matching inputs, including
  glyphs, breaks, objects and source mapping (F5 gate 1).
- [ ] Run all 751 existing cases and fractional transition/edge fixtures; check
  fragment padding, shaping preservation, vertical bounds, forward/reverse copy
  and hit tests. Retain minimal failures (F5 gates 2–3, F7).
- [ ] Only after geometry/consistency pass, run serial Skia GL/Vulkan stationary,
  selected, resize/reset, motion/replay and three recreations at observed DPR;
  record unavailable DPRs (F5 gate 4).
- [x] Retain diagnostic before/counterfactual images and interactive review launcher, reproducible
  instructions, identities and patch maintenance costs. Update support/migration
  records with feasible or blocked outcome; keep visual acceptance and Qt 6.8
  Vulkan shutdown separate (F6–F7, gate 5).
- [x] Run regression/supervisor tests, repository lightweight checks and whitespace
  validation; record actual results separately from this plan (F6).

One reviewable change contains implementation, checks, instructions and evidence.
Unchecked items are planned, not executed. No merge/publication is authorized.

### Execution outcome — blocked at F2/F4 cluster boundary

The private upstream patch implements source-span edge preflight only. The
minimal adjacent lam–alef spans have valid grapheme edges inside one shaped glyph;
the approved design explicitly treats such an unsatisfied edge as a capability
blocker. The finite padded candidate scan, authoritative padded positions and
vertical correction were therefore not implemented. The padded 751-case scan,
fractional transition probes, consistency and native gates were not reached.
Unchecked tasks above remain unimplemented, rather than being marked as passed.

Disabled equivalence passed for 751 fixtures / 775 blocks for this diagnostic
patch. An empty preflight issue list does not imply layout support. Historical
object rounding was diagnosed without changing the control or tolerance.
The review artifact shows unchanged shaping and separately shaped counterfactuals,
not an accepted after-fix capture. The blocked outcome and required design choice
are recorded in [verification](verification.md) and [migration](migration.md).

## Explicit shaping-boundary continuation — approved (B1–B8)

These tasks supersede the remaining F-series mechanism for the next isolated
stage only. Prior completion and blocked evidence remain historical. No task
below implies that a new implementation or successful feasibility result exists.

- [x] Inspect the historical experiment and revise requirements/design/tasks to
  select explicit shaping boundaries and separate equivalence baselines.
- [x] Obtain explicit approval of B1–B8, revised design and these mapped tasks;
  received 2026-11-05 ("approved") before implementation.
- [x] Create fresh pinned stage/build/evidence paths with checked patch application
  and complete identities; preserve `d3b1f05`, diagnostic patch and control (B1).
- [x] Carry distinct validated source-span identities into shaping; preserve
  original text/object intervals and range rejection checks (B2).
- [x] Split upstream shaping/context at all span edges while retaining paragraph
  bidi, script/fallback and Unicode breaks. Pass lam–alef whole/adjacent and
  Latin/Arabic/style/bidi/marks/surrogate/fallback/object regressions (B3, gate 1).
- [x] Bypass shaping-cache reads/writes for boundary metadata; verify metadata
  invalidation, removal/re-addition, repeated layout and width-only shaped-result
  reuse. Verify metadata-free control equivalence and establish zero-padding
  boundary-enabled baseline (B4, B7, gate 2).
- [x] Implement shared visual-fragment candidate fit/formatting, finite scans,
  upstream break priorities/emergency handling, source-progress checks and
  explicit indivisible-unit overflow (B5).
- [x] Integrate authoritative translations and vertical reservations across paint,
  rectangles, carets, selection and objects. Resolve complete extents with common
  baseline adjustment and rounded heights at unchanged tolerance (B6).
- [x] Run all 751 cases plus boundary regressions and recorded 1/64-pixel wrap
  transition probes. Check baseline glyph/intracluster equivalence, padding,
  whitespace, overflow, termination, exact copy both directions and authoritative
  hit tests; recheck `objects@679` metrics and painted/logical bounds (B7, gate 3).
- [x] Only after gates 1–3 pass, run serial native GL/Vulkan at observed DPR:
  stationary/selected, resize/reset, motion/replay and three recreations, retaining
  deadlines, identities and unavailable DPRs (B8, gate 4).
- [x] Retain matched captures and an interactive patched viewer launcher after
  geometry passes; record owner visual acceptance separately (B8, gate 5).
- [x] Run supervisor regressions, repository lightweight checks and whitespace
  validation; append actual evidence and upkeep costs. Declare feasible only
  with all automated gates passed; otherwise retain minimal reproducible blocker,
  attempted mechanism and missing capability, and stop dependent gates (B8).

No production/backend/performance claim, Qt-control change, merge or publication
is authorized by this task list.

Execution complete for B1–B8: the tested isolated slice passes all automated
boundary, equivalence, geometry, extent, selection and native gates. The corrected
fresh stage preserves the pinned symlink and produces byte-identical native
binaries. Owner visual acceptance remains pending; see [actual verification](verification.md).

## Approved measurement tasks (2026-11-06)

Owner instruction to implement the supplied plan approves P1–P6 and this mapping.

- [x] P1: Separate scale diagnostic runner, serial API traces and corrected explanation.
- [x] P2–P3: Pinned three-variant builds, deterministic fixture matrix, operation timers,
  separate counters, 10 process-cold and 5+30 warm measurements with bounded supervisor.
- [x] P4: Native scenarios, supported GPU callbacks and presentation feedback; three
  repetitions, separate captures, memory and submission/swap reporting.
- [x] P5: Unit/regression checks, instrumentation/source equivalence, raw samples,
  provenance, summaries, static curves and adjacent-size ratios.
- [x] P6: Actual verification, qualification gaps, bottlenecks and prioritized follow-ups.

Historical evidence remains intact. No merge or publication is authorized.


## Approved recovery — 2026-11-06

- [x] R1: Snapshot and hash surviving evidence; restore pinned prerequisites and record relocations.
- [x] R2: Harden resume, locking and checkpoints; add focused recovery tests.
- [x] R3: Finish 116 outstanding diagnostic variant/cases and 18 upstream comparisons serially.
- [x] R4: Record post-reboot environment/scale, one closed-window retry and corrected CPU smoke.
- [x] R5: Validate mappings/counters/windows; regenerate reports, inspect visuals, archive and verify hashes; run regression/lightweight checks.

No full CPU/native rerun, optimization, integration, merge or publication is authorized.

Recovery execution and reporting are complete with explicit historical artifact
availability gaps. This is measurement completion, not performance acceptance.
See the recovery section in [actual verification](verification.md).
