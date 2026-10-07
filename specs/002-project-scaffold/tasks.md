# 002: project scaffold tasks

**Status:** Approved for implementation. User approval recorded on 2026-10-07: requirements, design, and tasks for feature 002 only; feature 001 was draft at that approval and has since been separately approved (see its records).

- [x] T0 — Inspect repository conventions and briefs; prepare requirements, design, tasks, and planned verification. Covers R1–R7.
- [x] T1 — Obtain explicit approval of these requirements, design, and tasks; record approval scope before implementation. Covers R1–R7.
- [x] T2 — Add root build configuration, private module anchor, and Qt-generated QML module/plugin; support shared/static and standalone/subdirectory options. Covers R1–R3, R5.
- [x] T3 — Add install/export rules and relocatable CMake package metadata. Covers R2–R4.
- [x] T4 — Add import-only example, headless Qt Test smoke test, and independent installed-consumer fixture with static registration. Covers R5, R7.
- [x] T5 — Add MIT license, ignores, build/install/consumer instructions, and update project/specification status. Covers R6.
- [x] T6 — Run shared/static build, smoke, install/consumer, lint, relocation, and dependency checks; record actual commands and results separately from planned checks. Covers R1–R7.

T1 gates T2–T5. T3 and T4 depend on T2. T6 follows implementation. Scope changes require updated documents and renewed approval; routine fixes within approved scope may proceed.
