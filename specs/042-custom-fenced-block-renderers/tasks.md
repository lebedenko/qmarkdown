# Tasks — custom fenced-block renderers

Status: **approved on 2026-10-14** by the user response “approved”. Requirements
R1–R8, design and tasks T1–T5 are authorized for implementation and verification.
Package 1.7.0 / QML 1.7 is retained; no version bump, tag or publication is authorized.
See [requirements](requirements.md), [design](design.md), [tasks](tasks.md) and
[verification](verification.md).

| Task | Requirements | Checks |
| --- | --- | --- |
| T1: Preserve fence origin/full decoded info through parser, semantic/presentation trees, roles and nested delegates | R1, R2, R3 | V1, V2, V6 |
| T2: Add default-empty map, exact dispatch, contract validation, diagnostics, fallback and instance ownership | R1, R3, R6, R7 | V1, V3, V4 |
| T3: Integrate custom content width/height/style with surfaces, actions and selection source path | R3, R4, R5, R6 | V2, V3, V4, V5 |
| T4: Add native playground example and eventual public guide/API documentation | R1, R3, R7, R8 | V5, V7 |
| T5: Extend installed consumers; verify packaging, Qt targets and semantic regressions; record actual results and complete protected delivery | R2, R7, R8 | V6, V7 |

Approval record: requirements R1–R8, design and T1–T5 were explicitly approved
on 2026-10-14 by the user response “approved”. Implementation retains 1.7.0 / 1.7.
Version decisions must be recorded before changing package/module metadata. Publication
requires separate authorization; delivery does not imply it.

T1–T4 and T5 local verification/evidence are complete; actual results are in
[verification](verification.md). T5 protected delivery follows the required final-head
PR checks and squash merge. No publication is included.
