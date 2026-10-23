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
- [ ] Update open PR and observe final-head hosted checks (D6; results in PR).
