# 001: static text tasks

**Status:** Implemented and verified for the bounded static-text slice. User approval recorded 2026-10-07 via the implementation plan; covers existing R1–R7 requirements, design, and T1–T7 tasks. Actual results and limitations are recorded in [verification](verification.md).

- [x] T0 — Resolve bounded CommonMark structural rules, parser comparison/licensing/migration, exact style/default/ownership/reset API, sizing/update contracts, implementation layout, and requirement/check traceability. Documentation-only resolution recorded on 2026-10-07. Covers R1–R7; see [design](design.md).
- [x] T1 — Obtain and record explicit approval of the resolved requirements, design, and task scope before any feature source/build/dependency work. Covers R1–R7.
- [x] T2 — Implement private ordered document/parser/model adapter with normalized lines, paragraphs, ATX headings, and protected literal fences. Covers R1–R3, R7; V1–V3, V7.
- [x] T3 — Add MarkdownView and private native text delegates with wrapping, gaps, contentHeight/implicit sizing, and empty-heading geometry. Covers R2–R4, R7; V2–V4, V7.
- [x] T4 — Add MarkdownStyle defaults, font/color fields, ownership and shared-object handling, notification/reconnection, null/reset/destruction behavior, and runtime layout propagation. Covers R4–R5, R7; V4–V5, V7.
- [x] T5 — Implement coherent full replacement, identical-input behavior, and empty/nonpositive-width layout. Covers R4, R6; V4, V6.
- [x] T6 — Add Qt Test semantic and QQmlEngine/native Qt Quick behavior tests, a minimal standalone viewer, and extend existing shared/static installed-consumer checks to both public types. Covers R1–R7; V1–V8.
- [x] T7 — Run narrow semantic/native checks first, then shared/static tests, lint, installed/relocated consumer checks, and inspect viewer behavior. Record commands, versions, actual counts, limitations, and coverage separately from plans. Covers R1–R7; V1–V8.

T1 gates every implementation task. T2 precedes T3/T5; T3 precedes T4 layout integration. T6 accompanies the corresponding implementation after approval; T7 follows implementation. Use feature 002's Qt Test/CMake conventions and packaging script; no additional testing framework is needed. Scope changes require document updates and renewed approval.

| Requirement | Delivery tasks | Planned checks |
| --- | --- | --- |
| R1 | T2, T6, T7 | V1, V6 |
| R2 | T2, T3, T6, T7 | V2, V3 |
| R3 | T2, T3, T6, T7 | V3, V7 |
| R4 | T3, T4, T5, T6, T7 | V4, V5, V6 |
| R5 | T4, T6, T7 | V5, V8 |
| R6 | T5, T6, T7 | V6, V8 |
| R7 | T2, T3, T4, T6, T7 | V7, V8 |

See [requirements](requirements.md) and [verification](verification.md). User approval of this package is recorded above; actual verification is recorded separately.
