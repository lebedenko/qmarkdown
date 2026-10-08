# 008: Tasks

**Approval:** On 2026-10-08 the user explicitly requested implementation of the supplied “Support both font units and application-based defaults” plan. This approves the complete requirements, design, and mapped tasks below before implementation. Package 0.5.0 and QML module 0.5 remain unchanged.

- [x] T1 (R1–R3): implement construction snapshots and derived fonts; test pixel and fractional-point defaults, reset, independence, notifications and inline inheritance.
- [x] T2 (R4): implement playground units, conversion and fractional editing; test passive inheritance, fields/masks, presets and reset.
- [x] T3 (R5): compare native/private point layout, correct private DPI handling and test invalidation, wrapping and content heights with shared/replaced styles.
- [x] T4 (R6): update README/examples and specification index; mark historical fixed defaults superseded.
- [x] T5 (R1–R6): extend installed consumer points; run focused tests, full suite, lint and shared/static packaging; record actual results and display/mixed-screen limits separately.
