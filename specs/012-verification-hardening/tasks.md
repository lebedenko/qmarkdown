# Tasks

Approved on 2026-10-08 by the explicit user instruction to implement the supplied Verification-first improvements plan. Approval covers requirements, design and T1–T4 below; package/module remain 0.7.0/0.7.

- [x] T1 → R1: CI matrix, explicit packaging SDK/artifacts, library-only configuration; packaging and workflow inspection.
- [x] T2 → R2: 43 authored expectations, schema/validation/fault tests, individual ledger review; focused oracle suite, full corpus and strict failure.
- [x] T3 → R3: private decoder seam and gated lifecycle scenarios; focused repeated lifecycle runs and full CTest.
- [x] T4 → R4: opt-in executable/Task, JSON validation/smoke and full Release measurements; disabled builds and install exclusion.
- [x] Documentation: README, specification index, roadmap and conformance inventory; record actual evidence separately.

Implementation and initial local verification are complete; full Release benchmark output passed validation. The first remote Qt matrix run failed in viewer tests; the diagnosis and follow-up verification are recorded in verification.md. A remote run of the fixes remains pending.
