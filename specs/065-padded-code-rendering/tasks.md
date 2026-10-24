# Mapped tasks

Approval: owner requested the supplied implementation plan on 2026-10-22.

- [x] Inspect existing renderer/workflow; record approved scope (R1–R7).
- [x] Build public-API minimal software foreground-emission probe (R3–R4).
- [x] Retain failing reproducer, captures and actual results; stop integration (R4).
- [ ] Revise design and obtain renewed approval after prototype failure.

The following approved tasks are conditional on a successful prototype gate and
have not been executed:

- [ ] Extend gate to bidi, ligatures, combining marks, fallback, overhang, unique
  emission, shared layouts and retained node/layout identities on both exact Qt versions (R2–R4).
- [ ] Integrate persistent padded glyphs and rounded image backgrounds (R1, R5–R6).
- [ ] Add sparse/dense, wrap/tiny-width, alignment, links/decorations fixtures;
  test selection/copy, Find, focus, pointer geometry, style/padding and atom transitions (R2, R5).
- [ ] Qualify native DPR/lifecycle, fixed fresh baseline replay, diagnostic zero-work
  counters, preparation/resources/memory/replacement/teardown (R1, R7).
- [ ] Run exact-Qt shared/static/optional-feature/installed-consumer packaging
  matrices, then final-head hosted checks (R4, R6).
- [ ] Open one non-draft PR; leave unmerged and unpublished.

## Renewed retained-texture tasks (2026-10-22 approval)

- [x] Revise requirements/design/tasks before implementation (T1–T7).
- [x] Stage diagnostic patch and implement prepared-run image correctness gate (T1–T2).
- [x] Run italic fX first; retain reproducer and stop expansion on failure (T5).
- [ ] If correct, implement retained image nodes, rounded backgrounds and decorations (T3–T4).
- [ ] If correct, expand shaping/image/backend/DPR and lifecycle gates (T5–T6).
- [ ] If correct, compare preserved fixed traces with fresh baseline and report costs (T6).
- [ ] Record actual evidence, docs/whitespace and final-head hosted checks; open
  one non-draft PR, unmerged with auto-merge disabled (T7).

The retained preparation gate failed on both exact SDKs at image DPR 1.25.
Conditional expansion tasks remain intentionally unexecuted; see
[actual retained verification](retained-verification.md).

## Diagnostic tasks — approved 2026-10-23

The owner requested implementation of the supplied diagnostic plan.

- [x] Record requirements/design/tasks and approval before implementation (D1–D6).
- [x] Implement separate selectable diagnostic controls and structured logs (D1–D3).
- [x] Run historical reproducer and full phase matrix on fresh exact SDK builds (D4–D5).
- [x] Inspect representative captures; record actual results and recommendation (D5–D6).
- [x] Synchronize Qt and validate docs/whitespace (D6).
- [x] Submit continuation [PR 49](https://github.com/lebedenko/qmarkdown/pull/49)
  for final-head hosted checks (D6; actual outcomes recorded in PR). PR 48 was
  already merged before that session. PR #49 subsequently closed without merge;
  the current continuation recovers its work without reopening it.

## Whole-run tasks — owner approval 2026-10-23

- [x] Record approved bounded revision before implementation (W1–W7).
- [x] Recover closed PR #49 harness/evidence and correct delivery status (W6).
- [x] Implement separate whole-run tile/crop/direct gate and explicit fonts (W1–W4).
- [x] Fresh exact-SDK builds; historical fresh/reverse controls, complete basic
  matrix, then conditional extended matrix; retain identities/metrics/captures (W3–W5).
- [x] Lightweight verification (W6).
- [ ] Open one continuation PR, complete final-head hosted checks and leave
  unmerged with auto-merge disabled (W6; final status lives in PR checks/body).

Software parity alone permits a later design review, never production integration
or speedup claims (W7).

## Retained run-texture scene-graph tasks — approved 2026-10-23

- [x] Record the supplied approved requirements/design/tasks revision (S1–S9).
- [x] Inspect native environments; implement isolated immutable preparation,
  render-thread-owned image nodes and exact three-path controls (S1–S4).
- [x] Fresh SDK builds and preparation gates on both targets (S4).
- [x] Complete static software/OpenGL/Vulkan matrices and both orders; retain
  identities, counts, metrics and captures; stop on failure (S3–S5).
- [x] Conditional: scroll/clip/retention matrix, then three recreation cycles
  with retained CPU images and balanced resources (S6–S7).
- [x] Record actual experiment results and evidence separately (S8).
- [x] Prepare the complete stacked-PR payload and pass local repository checks (S8).

Final-head hosted outcomes and open/unmerged/auto-merge status are recorded in the
continuation PR body/checks; no evidence-only follow-up commit is created (S8).

## Shaping and decorated-content tasks — approved 2026-10-24

The owner explicitly requested implementation of the supplied plan on
2026-10-24; this approves the requirements, design and mapped tasks in this
revision before implementation.

- [x] Record C1–C7 requirements, design and tasks before implementation.
- [x] Implement separate gate, fixture records, explicit font validation and CPU controls (C1–C5).
- [x] Build fresh exact Qt 6.8.0/6.12.0 and run whole-run prerequisite (C5).
- [x] Finish CPU matrix on both targets; retain failures without threshold changes (C4–C5).
- [ ] Conditional: complete transport, traces and three recreation cycles (C6).
- [x] Retain actual evidence and prepare the complete continuation payload (C7).
- [x] Pass final lightweight repository checks, whitespace and evidence identities (C7).
- [x] Prepare one continuation PR payload against main, unmerged with auto-merge
  disabled (C7; actual publication and final-head outcomes live in PR body/checks).

The final CPU gate failed on both SDKs; conditional transport, motion and
recreation qualification remains unexecuted. See
[actual decorated verification](decorated-verification.md). PR #51 and #52 are
merged; their earlier delivery instructions remain historical records.
