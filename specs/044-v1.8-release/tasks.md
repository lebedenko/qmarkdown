# Tasks — 1.8 desktop reading improvements

Status: **approved on 2026-10-14** by the user’s explicit “approved” response.
Approval covers requirements, design and T1–T5 implementation/verification.
T1–T5 implementation and local verification are complete. Final-head hosted
verification is recorded in the associated feature PR.
Protected merge, tagging and publication remain separately authorized.

| Task | Requirements | Checks |
| --- | --- | --- |
| T1: Preserve semantic unit ranges before resource splitting; implement Unicode word, whitespace, grapheme, cell and logical-line selection and whole-unit extension | R2, R3 | V1, V2 |
| T2: Add platform repeated-click handling and cancellable delayed desktop link activation; preserve excluded actions and existing touch/disabled behavior | R3 | V3 |
| T3: Add viewport/request interface, elapsed-time edge scrolling and cancellation; integrate playground host clamping | R4, R5 | V4 |
| T4: Update public documentation, package/QML metadata and installed-consumer interface/compatibility coverage | R1, R6 | V5 |
| T5: Finalize release documentation and run focused checks followed by relevant shared/static Qt matrix and final-head PR gates; record actual evidence | R6, R7 | V5, V6 |

Approval covers T1–T5 implementation and verification only. Protected merge,
release tagging and publication remain separately authorized delivery actions.
