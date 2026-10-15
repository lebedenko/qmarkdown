# Tasks — 1.9 reading and navigation

Status: **approved on 2026-10-15** by the user response “approved”.
Approval covers requirements R1–R9, design and tasks T1–T6 implementation and
verification. Merge, tagging and publication require separate authorization.

T1–T6 implementation and local verification are complete. Results are recorded in
[verification](verification.md); required hosted PR checks remain a delivery gate.

| Task | Requirements | Planned checks |
| --- | --- | --- |
| T1: Add private literal search state, offsets, lifecycle and wrapping actions; expose QML search API | R1–R3, R7 | V1, V2 |
| T2: Implement independent search painting, searchable source dispatch and registered target geometry with refresh | R3, R7 | V2, V3 |
| T3: Collect recursive heading outline, allocate collision-safe IDs and implement explicit heading/fragment requests | R4–R7 | V4, V5 |
| T4: Add playground controls, shortcuts, outline and host scrolling/link examples | R6, R8 | V5, V6 |
| T5: Add installed-consumer coverage, public API docs and coordinated package/QML 1.9.0/1.9 release documentation | R1, R9 | V7, V8 |
| T6: Run focused tests, relevant configured Qt shared/static and installed-consumer gates and conformance regressions; record actual outcomes | R1–R9 | V1–V8 |

Approval recorded for R1–R9, this design and T1–T6 implementation/verification. Merge, tagging and publication remain excluded.

## Release authorization — 2026-10-15

The user instruction “release it” additionally authorizes the release PR, protected
squash merge, tagging and publication of 1.9.0 after required hosted checks and
distribution checks pass. This supersedes the earlier delivery exclusions above.
Publication completion requires downloaded-asset hash verification. Final hosted
verification and release outcomes are retained in GitHub Actions and the release.
