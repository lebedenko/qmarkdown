# 004: native fenced code block tasks

**Status:** Implemented and verified for the bounded Feature 004 slice. Approved by the user on 2026-10-07. Approval covers requirements R1–R6, design, and tasks T1–T7 for this bounded slice; implementation and verification are authorized.

- [x] T0 — Inspect architecture and prepare requirements, design, tasks and verification records.
- [x] T1 — Obtain explicit approval and record its date/scope in the feature documents before code changes.
- [x] T2 — Add CodeBlock/infoString and implement full-length fence extraction, indentation, physical-line handling and private info decoding. Covers R1–R3; V1/V2.
- [x] T3 — Add independent notifying code font/color defaults and lifecycle behavior. Covers R4; V4.
- [x] T4 — Add private model role and native code delegate with display-only LF removal and preserved layout behavior. Covers R3/R5; V3/V4.
- [x] T5 — Update versions, metadata, viewer and installed-consumer fixtures. Covers R6; V5/V6.
- [x] T6 — Add parser/native/lifecycle regression coverage; run focused tests, shared/static checks, lint, symbols, relocated packaging and viewer inspection. Record actual evidence. Covers R1–R6; V1–V6.
- [x] T7 — Update current documentation with verified behavior and limits, retaining historical verification. Covers R1–R6; V6.

| Requirement | Tasks | Checks |
| --- | --- | --- |
| R1 | T2, T6 | V1 |
| R2 | T2, T6 | V2 |
| R3 | T2, T4, T6 | V2, V4 |
| R4 | T3, T6 | V4 |
| R5 | T4, T6 | V3, V4 |
| R6 | T5, T6, T7 | V5, V6 |
