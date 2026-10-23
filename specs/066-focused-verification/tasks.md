# Tasks

Approved by the user implementation request on 2026-10-23.

1. Implement profile classification and lightweight validation; test mixed/renamed/deleted/unknown and optional inputs.
2. Group current-version consumers, select retained build combinations, introduce Ninja/ccache and separate deadlines.
3. Update hosted workflow and Full-only release evidence; test publication rejection boundaries.
4. Implement staged/pre-push snapshot hooks, exact host Qt checks, locking and task commands; test isolated Git fixtures.
5. Update workflow documentation, run narrow tests then exact-Qt Core checks, and submit one open PR with auto-merge disabled. Hosted final-head Full checks and measured cache/timing results remain part of delivery, without evidence-only commits.

## Traceability

| Task | Requirements | Checks |
| --- | --- | --- |
| 1 | R1, R2, R9 | Profile, rename/deletion, evidence-syntax and complete tooling tests |
| 2 | R3, R4, R5, R6 | Both exact-Qt Core runs, hosted Full configurations and separate consumer processes; cache/phase artifacts |
| 3 | R2, R6, R7 | Release eligibility/publication fixtures, timeout/cancellation fixtures and hosted required checks |
| 4 | R8 | Git snapshot/ref/ancestry/locking/prerequisite fixtures and actual staged hook |
| 5 | R9, R10 | Documentation links, final diff review, open PR final-head Full results and PR evidence comment |
