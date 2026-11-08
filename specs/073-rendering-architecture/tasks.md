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

## Approved phase diagnosis tasks (2026-11-06)

The owner explicitly requested implementation of this bounded plan.

- [x] D1: Record scope, build isolated hash-identified phase instrumentation.
- [x] D2: Run twelve fixtures × two variants × two binaries serially; retain
  phase deltas, geometry equivalence, shaping guards and terminal outcomes.
- [x] D3: Historical gates; attribution/reset/query-exclusion/resume tests;
  measurement/recovery/supervisor regressions, lightweight and whitespace checks.
- [x] D4: Archive verified raw evidence and produce approval-ready optimization
  requirements/design/tasks or a specific unresolved question and minimal probe.

No optimization, adapter/paint/scheduling work, Vulkan timing, Qt integration,
backend selection, merge or publication is authorized.

## Approved ownership optimization — 2026-11-06

The owner instruction to implement the supplied plan explicitly approves O1–O5,
the corrected mechanism and O-T1–O-T4 in [optimization design](optimization-design.md).
This supersedes the earlier draft-only restriction for this bounded isolated step.
Preserve historical evidence, uncommitted diagnosis and commit `4b9fd4c`.
Metric-only `resetContext()` retains ownership; run revision invalidation covers
replacement, restart, metadata and cache restore. Fresh bounded storage, legacy
fallback, exact four-operation differential checks and work counts are required.
No speedup qualification, production change, merge or publication is approved.

- [x] O-T1: Private checked patch, run/cache lifecycle audit and range/fallback tests.
- [x] O-T2: Lifecycle, stale rejection, failed/empty shaping and bounded-storage checks.
- [x] O-T3: 48 exact differential traces, preserved phase counts and historical gates.
- [x] O-T4: Work/memory reporting, regression checks and hash-verified evidence archive.

Completion is an isolated verified optimization, not a speedup qualification.

## Approved ownership runtime tasks — 2026-11-06

The current owner instruction explicitly approves RT1–RT4 requirements/design and:

- [x] RT-T1: Add symmetric driver, pinned builds/provenance and correctness gates.
- [x] RT-T2: Add durable schedule, bounded serial runner and terminal new-directory resume.
- [x] RT-T3: Add paired bootstrap reports and focused/regression/lightweight checks.
- [x] RT-T4: Execute 48 correctness and 480 timing processes, archive verified evidence and record results/recommendation and limitations.

## Approved candidate-scan proof tasks — 2026-11-06

The current owner request approves C1–C5 and the bounded design above before coding.

- [x] C-T1 (C1–C3): Isolated guarded suffix prototype and oracle/shadow modes; proof.
- [x] C-T2 (C1–C3): Focused endpoint/Unicode/padding tests, exhaustive length ≤4
  width/whitespace/eligibility patterns and 10,000 seed-73074 numeric cases.
- [x] C-T3 (C1, C4): Fixed 72-process four-operation diagnostics, runtime archived
  exact comparisons, historical boundary/extent and lifecycle guards.
- [x] C-T4 (C4): Supervisor/recovery/harness/light checks, whitespace and completed
  resume, immutable journals and verified source/raw evidence archives.
- [x] C-T5 (C5): Report preprocessing/storage and long/spans-32 evaluation reduction;
  recommend separate implementation approval or reject with reproducible evidence.

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

## Candidate-scan runtime tasks (approved 2026-11-06)

The owner's supplied-plan implementation request approves CR1–CR5 and the design
above before implementation and measurement.

- [x] CR-T1 (CR1–CR3): Sibling driver, pinned fresh builds, preflight source/binary
  audit, fixed fixtures and persisted complete serial schedule.
- [x] CR-T2 (CR2–CR4): Strict output/shaping and archived trace validation, historical
  gates, bounded execution, terminal failure and immutable validated resume.
- [x] CR-T3 (CR5): Whole-pair statistics, timer suppression, per-paragraph overhead
  and exact primary/regression gates; missing-sample and boundary tests.
- [ ] CR-T4 (CR4): Execute correctness and timing, harness/report/recovery/supervisor
  regressions, lightweight checks, whitespace and fatal-sentinel completed resume.
- [x] CR-T5 (CR5): Verify all archived files/hashes and record recommendation
  distinguishing benefit, regression and uncertainty without integration.

CR-T4 actual outcome: 48 correctness processes pass; process 49 fails the inherited
initial-shape assertion on small prose. The study is terminal with 39 correctness
and 1,760 timing processes unlaunched. Historical gates and tooling/light checks
pass; fatal-sentinel terminal-resume rejection passes. Completed-run resume/full
validator cannot execute. CR-T4 is intentionally incomplete; no retry is authorized
by this plan. CR-T5 delivers verified failure evidence and a no-retention decision.

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

- [x] CC-T1 (CC1): Diagnose both arms and retain evidence.
- [x] CC-T2 (CC2): Correct and regression-test lifecycle validation.
- [x] CC-T3 (CC3): Execute fresh fixed study after gates.
- [x] CC-T4 (CC4): Validate, archive, read back and record outcome.

## Candidate-scan diagnosis tasks (approved 2026-11-08)

Approval: the owner's request to implement the supplied diagnosis plan authorizes
CD1–CD4 requirements, design and these tasks before coding.

- [x] CD-T1 (CD1): Preserve historical state; verify archives, members, schedule,
  raw/fixture hashes and existing source/binary identities; stop on discrepancies.
- [x] CD-T2 (CD2): Recompute unchanged report exactly in isolation; produce the
  complete pair/cell matrix, failed-cell detail and descriptive order/drift context.
- [x] CD-T3 (CD3): Inspect verified source paths and binary code; retain tool
  versions/commands/disassembly and separate facts, hypotheses and uncertainties.
- [x] CD-T4 (CD4): Test pairing/nulls/normalization/timer/extremes/grouping and
  integrity rejection; check determinism, historical preservation, lightweight
  checks and whitespace; archive/read back evidence and record recommendation.

## Candidate-scan calibration tasks (CB1–CB4, approved 2026-11-08)

The owner approved CB1–CB4 requirements, design and CB-T1–CB-T5 tasks via the
supplied-plan implementation request on 2026-11-08. Path isolation, extra samples,
production integration, commits, merges and publication remain outside approval.

- [x] CB-T1 (CB1): Obtain explicit approval of these requirements, design and tasks;
  record its scope. Verify historical archives, two fixtures and existing baseline
  source/binary identities. Stop on discrepancies without rebuilding.
- [x] CB-T2 (CB2–CB4): Implement the isolated A/B adapter and complete fixed schedule;
  enforce identical executable/arguments, balanced randomized order, process bounds,
  raw identity checks, lifecycle/geometry validation and immutable terminal journals.
- [x] CB-T3 (CB3–CB4): Implement all-cell descriptive reporting, whole-pair bootstrap,
  frozen target reference envelopes and explicit unresolved/resolved-control wording.
  Test timer/null/normalization/extreme and boundary cases, schedule/pair identities,
  missing/failed records, altered hashes and fatal-sentinel recovery before launch.
- [x] CB-T4 (CB2–CB4): After integrity and tooling gates, run two correctness and
  eighty timing processes exactly once. Stop terminally on failure. Preserve all
  four operations and observations; no retries, tuning or additional samples.
- [x] CB-T5 (CB4): Verify completeness, deterministic reporting and completed-run
  no-launch resume. Run lightweight and whitespace checks; archive/read back
  sources, input hashes, schedules, raw outputs, reports and verification evidence.
  Record results and recommend the separately approved next experiment according
  to the design decision rules. Preserve retention not established.

Planned checks above are not claims of executed verification; actual results are
recorded separately in verification.md.

## Calibration revision tasks (approved 2026-11-08)

The owner's supplied-plan implementation request approves MR1–MR4 requirements,
design and these tasks before implementation, with the exclusions stated above.

- [x] MR-T1 (MR1): Record approval; preserve existing work and inspect dependencies.
- [x] MR-T2 (MR1–MR2): Implement sibling runner, frozen four-process schedule,
  historical verification, parent timestamps and terminal prefix recovery.
- [x] MR-T3 (MR3–MR4): Implement/test group estimator, bootstrap, nulls, timer
  suppression, extremes, inclusive boundaries, constant offsets and linear drift;
  test all commands, balance/contiguity, lifecycle, altered identities/hashes,
  failed/interrupted attempts and missing/duplicate/misordered groups.
- [x] MR-T4 (MR2): After preflight gates execute exactly 322 processes once.
- [x] MR-T5 (MR4): Verify completed resume with fatal launch sentinels,
  deterministic reanalysis, focused/lightweight/whitespace checks, archive and
  member read-back; revalidate historical evidence and record decision.

Actual verification is recorded separately in verification.md.

## Workstation measurement procedure tasks (2026-11-08)

Approval scope: the owner accepted the characterization direction and requested this
procedure update, with all experiments on the same non-dedicated workstation.
The future execution tasks below remain planned, not authorized benchmark launches.

- [x] MP-T1 (MP1–MP4): Record the accepted direction, same-machine constraint,
  telemetry policy, genuine-batch semantics and historical-preservation rules in
  [measurement-procedure.md](measurement-procedure.md).
- [x] MP-T2 (MP2): Analyze existing observations/timestamps and describe patterns
  without unsupported causal conclusions or new measurements.
- [ ] MP-T3 (MP2–MP4): Specify the finite pilot size set/counts, seeds, timing and
  precision objectives, preparation/cache rules, instrumentation overhead checks,
  deterministic selection/stop rule and tests; obtain implementation-plan approval.
- [ ] MP-T4 (MP1–MP4): After approval, implement/test the isolated characterization
  harness and run the fixed pilot, retaining all samples and unavailable telemetry.
- [ ] MP-T5 (MP3–MP4): Freeze and approve the resulting baseline/self calibration
  protocol separately; execute only that budget and verify/archive all evidence.

The procedure update above performed MP-T1 only. The later offline characterization
below completes MP-T2 and the proposal portion of MP-T3; separate pilot approval
remains pending. No new benchmark or renderer build was launched.

## Offline workstation characterization tasks (approved 2026-11-08)

Approval: the owner's supplied-plan implementation request approves WC1–WC4, the
above design and these tasks; pilot implementation/execution is expressly excluded.

- [x] WC-T1 (WC1): Record scope, snapshot preservation identities, verify all four
  evidence chains and reproduce the closed revision exactly.
- [x] WC-T2 (WC2): Implement isolated timestamp/statistical analysis and complete
  JSON/CSV matrix plus concise observations/hypotheses report.
- [x] WC-T3 (WC3): Write the concrete draft pilot protocol and data-only 297-command
  schedule; test determinism, balance, identity and contiguity without execution.
- [x] WC-T4 (WC4): Run focused, lightweight and whitespace checks with fatal launch
  sentinels; prove byte-identical reanalysis; archive/read back every member;
  recheck history and update recommendation/actual verification.

## Fixed batching pilot — approved 2026-11-08

The owner's supplied-plan implementation request explicitly approves the requirements,
design and tasks in [batching-pilot-proposal.md](batching-pilot-proposal.md), the
isolated Release harness build against verified existing baseline libraries, and
one fixed 297-process pilot (nine correctness, 288 timing). This supersedes the
proposal's historical draft approval status only. Preserve all prior work and
evidence. No renderer/public API changes, candidate comparisons, later calibration,
commits, merges or publication are authorized. Retention remains **not established**.

## Fixed batching pilot delivery (2026-11-08)

- [x] BP-T1: Record the supplied-plan approval; verify historical archive/member
  hashes and pinned baseline inputs; build only the sibling Release harness with
  two workers and matching compile definitions.
- [x] BP-T2: Implement prepared-state lifecycle/cache checks, timer boundaries,
  enabled/disabled telemetry, fixed schedule, durable journals and descriptive
  analysis/selection; pass synthetic preflight without benchmark launches.
- [x] BP-T3: Execute the frozen nine correctness and 288 timing processes once;
  retain every observation and apply the deterministic selection rule.
- [x] BP-T4: Verify byte-identical reanalysis/completed resume with fatal launch
  sentinels; finish lightweight/whitespace checks and archive member read-back.

MP-T3's pilot implementation/execution approval is now granted by the supplied-plan
request above. MP-T4's fixed pilot executed completely. MP-T5 remains unapproved:
no later calibration dataset, sampling, renderer change or publication follows.

## Approved fixed batching refinement — 2026-11-08

The supplied-plan implementation request approves the requirements, design and tasks
in [the refinement protocol](batching-refinement-protocol.md), including its exclusions.
Historical 297-process pilot and no-selection result remain unchanged.

### Refinement delivery tasks

- [x] BR-T1: Record approval, preserve historical evidence and verify pinned inputs;
  build only the isolated Release harness with two workers.
- [x] BR-T2: Implement fixed 231 identities, unchanged lifecycle/cache/timing,
  validated optional telemetry deltas, mixed ratio suppression and forensic output.
- [x] BR-T3: Pass offline tests and run seven correctness plus 224 timing processes
  once; apply the fixed selection rule only after complete validation.
- [x] BR-T4: Finish deterministic reanalysis/completed resume, archive read-back,
  historical preservation verification and delivery checks.

## Approved prepared-state calibration — 2026-11-08

The owner’s supplied-plan implementation request explicitly approves PC1–PC5 and
PC-T1–PC-T5 in [the calibration protocol](prepared-calibration-protocol.md). This
grants one 483-process calibration at size 96 and evidence delivery only. Earlier
exclusions of later calibration remain historical; this approval is bounded to
this new run. Retention remains **not established** under every outcome.

- [x] PC-T1: Approval and historical input inspection.
- [x] PC-T2: Isolated frozen runner and strict recovery.
- [x] PC-T3: Analysis, forensic support and offline tests.
- [x] PC-T4: Fixed execution once.
- [x] PC-T5: Reanalysis, checks, archive read-back and delivery.

Subsequent owner approval (2026-11-08): “both”, in response to the explicit
choice between committing the completed calibration and designing the next
prepared-state path-isolation experiment, authorizes this calibration commit and
drafting the next protocol. It does not approve implementation or execution of
the next experiment, a push, merge or publication. Historical execution limits
and approval records remain unchanged.

## Prepared-state path-isolation proposal — draft, 2026-11-08

The owner approved designing the next experiment and committing the completed
calibration (“both” after those two explicit choices). Calibration commit is
`f7879b3`. The [new proposal](prepared-path-isolation-proposal.md) defines draft
PI1–PI6 requirements, source/build design and PI-T1–PI-T6 tasks, with a proposed
2,001-process budget and contemporaneous controls. Only protocol drafting is
authorized. Implementation/build/execution approval is pending; no next-experiment
sample has been collected. Retention remains **not established**.
