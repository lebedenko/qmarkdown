# 063 — Mapped tasks

Status: explicitly approved by the user on 2026-10-20.

| Task | Requirements | Work and checks |
| --- | --- | --- |
| T1 | R1, R4–R5 | Add provider enum/request objects, strict JS span validation, terminal/cancelled state and guarded ownership; test invalid/empty/duplicate/retained-wrapper cases. |
| T2 | R2, R5–R6 | Add provider-local coalescing dispatcher, timeout, bounded success cache, invalidation and subscriber cancellation; test shared providers, reentrancy, stale results and identity/generation isolation. |
| T3 | R2–R3, R6 | Integrate provider-first SyntaxRequest dispatch with backend/plain fallback, source limits and palette-only recoloring; test ON/OFF, unknown languages, themes and precedence. |
| T4 | R1, R3, R7 | Expose QML 1.17 property/type, forward through recursive code surfaces, preserve availability semantics and older imports; test footnote code and view/provider destruction. |
| T5 | R7 | Add deterministic async playground provider/sample, API/syntax/lifecycle guidance, roadmap and compatibility metadata; verify installed shared/static consumers and archive payload. |
| T6 | R7 | Run focused then affected suites, lint/docs, backend ON/OFF and exact Qt 6.8.0/6.12.0 gates after toolchain sync; record actual results and native-input limitations. |
| T7 | R7 | Open one implementation/tests/docs/evidence PR, require final-head packaging checks and record results. Do not merge, tag or publish. |

Approved scope: requirements.md, design.md and T1–T7 for implementation and
verification. Changes to approved scope require renewed approval. Merge, tagging,
publication and the excluded renderer/streaming features remain outside approval.

Approval record: the user selected “Approve Feature 063” for requirements, design
and T1–T7 on 2026-10-20. Implementation and verification are authorized after
Feature 062's required CI checks pass. Merge, tagging and publication remain
excluded. No provider runtime implementation had started at approval time.
