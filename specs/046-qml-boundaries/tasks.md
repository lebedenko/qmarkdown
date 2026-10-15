# Tasks — QML responsibilities and private contracts

Status: **approved on 2026-10-15** by the user response “approved”.
Approval covers requirements R1–R5, design and tasks T1–T7 implementation and
verification. Merge and publication require separate authorization.

| Task | Requirements | Planned checks |
| --- | --- | --- |
| T1: Capture pre-change advisory search baseline and add bounded C++ range query/revision; migrate QML queries and remove full-list accessor | R1, R5 | V1, V6 |
| T2: Add typed SelectionRegistration lifecycle and metadata; migrate selection/navigation consumers and private tooling metadata | R4 | V2, V3 |
| T3: Extract BlockSequence, text/shared selectable text, image and list presentations; wire explicit inputs and relayed signals | R2, R4 | V3, V4 |
| T4: Remove code whole-view inputs, retain lifecycle guards and project fence language into a model role | R3 | V4, V5 |
| T5: Type styles, validate table sequence casts and update all resource/install/archive inventories and package checks | R4, R5 | V3, V7 |
| T6: Run focused tests/lint, relevant suites and exact configured Qt shared/static packaging matrices; compare advisory benchmarks | R1–R5 | V1–V7 |
| T7: Review public API/behavior compatibility, record actual evidence and changed files, prepare one focused PR | R1–R5 | V7, V8 |

T1–T6 implementation and local verification are complete. T7 compatibility review
and evidence recording are complete; the change is prepared as one focused PR.
See [actual results and changed files](verification.md). Approval is recorded
above. Merge and publication remain outside this authorization.
