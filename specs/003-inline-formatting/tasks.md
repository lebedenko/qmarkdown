# 003: inline formatting tasks

**Status:** Implemented and verified for the bounded Feature 003 slice. Approved by the user on 2026-10-07. Approval covers Feature 003 requirements R1–R7, the design, and tasks T1–T8; implementation and verification are authorized.

- [x] T0 — Inspect existing architecture and prepare requirements, design, tasks, and verification records.
- [x] T1 — Obtain and record explicit user approval of these requirements, design, and tasks before source/build/dependency changes.
- [x] T2 — Vendor cmark 0.31.2, provenance/checksum, applicable notices, private C99/PIC object integration, complete symbol prefixing and hidden visibility; install notices. Covers R6; check V1/V6.
- [x] T3 — Implement private inline adaptation, sentinel removal, validated source preservation, flags/ranges, deterministic cleanup, and conservative whole-block fallback. Keep block/fence behavior. Covers R1/R2; check V2/V3.
- [x] T4 — Add inlineCodeFont overlay/default/notification/reset behavior and model formatting roles. Covers R3/R4; check V4/V5.
- [x] T5 — Implement private polished/cached QTextLayout rendering and QML delegate selection, wrapping/ink bounds, geometry and lifecycle behavior. Covers R4/R5; check V4/V5.
- [x] T6 — Update package/module versions, viewer examples/alternate style, and installed-consumer fixtures. Covers R6/R7; check V5/V6/V7.
- [x] T7 — Add authored parser/native/consumer regression fixtures, run focused tests then shared/static CTest, lint and relocated consumers, inspect symbols and viewer capture, and record actual evidence. Covers R1–R7; check V1–V7.
- [x] T8 — Update current documentation with verified behavior and limitations while retaining Feature 001 historical results. Covers R1–R7; check V7.

| Requirement | Tasks | Checks |
| --- | --- | --- |
| R1 | T3, T7 | V2, V3 |
| R2 | T3, T7 | V3, V5 |
| R3 | T4, T5, T7 | V4, V5 |
| R4 | T4, T5, T7 | V4, V5 |
| R5 | T5, T7 | V4, V5 |
| R6 | T2, T6, T7 | V1, V6 |
| R7 | T6, T7, T8 | V5, V6, V7 |
