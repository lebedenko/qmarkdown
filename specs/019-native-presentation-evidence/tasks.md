# Tasks

Approved on 2026-10-09 through the explicit user approval of Iteration 019. Approval covers requirements, design and tasks; scope is tests/documentation only. Production changes require revised approval.

- [x] T1 (R1–R3,R6): Audit existing native assertions and create coverage.md with named checks, observed contracts and explicit gaps. Identify production defects separately from missing tests.
- [x] T2 (R1,R2): Add bounded data-driven native checks for missing block/inline mapping and layout transition coverage in the existing view suite. Reuse existing assertions where sufficient.
- [x] T3 (R3): Close presentation gaps around resource fallback/arrival/revocation and navigation by extending existing view/resource fixtures only where needed. Retain deterministic decoder/network gates.
- [x] T4 (R4,R5): Run focused cases, strict 652-example semantic checks, shared/static CTest and both pinned local CI tasks. Record executed results against every coverage row and preserve unresolved failures.
- [x] T5 (R1,R5,R6): Update current-status README/standards/roadmap, finalize verification and coverage records, and list remaining release decisions without a version bump or v1.0 declaration.

Approval excludes production changes discovered during the audit and any v1.0 release decision.
