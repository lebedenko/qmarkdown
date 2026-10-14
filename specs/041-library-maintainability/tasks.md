# Tasks — library maintainability

Status: approved on 2026-10-14. T1–T7 are complete. Actual results are recorded in [verification](verification.md).

| Task | Requirements | Verification |
| --- | --- | --- |
| T1: Introduce named model/render/selection contracts and private QML metadata | R1 | V1, V3 |
| T2: Separate semantic tree, text projection and presentation tree; adapt resource projection/model consumers | R3 | V1, V2, V4 |
| T3: Extract shared code surface and table components; internalize helpers | R2, R6 | V3, V4, V5 |
| T4: Replace string style state/dispatch with typed values and explicit notifications | R4 | V3, V4 |
| T5: Harden parser ownership, manager transfer and policy reference; reduce header coupling | R7 | V1, V2, V5 |
| T6: Write complete public API reference and reconcile current guides | R5 | V6 |
| T7: Update packaging/archive inventory; run relevant final checks and record results | R8 | V1–V6 |

Approval record: the user approved requirements, design and T1–T7 on 2026-10-14.
Local implementation and verification are authorized.
