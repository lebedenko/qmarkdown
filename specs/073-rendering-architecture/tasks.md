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
