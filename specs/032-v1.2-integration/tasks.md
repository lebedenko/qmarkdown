# Tasks

Approved on 2026-10-12 through the explicit instruction to implement the supplied Iteration 032 plan. Approval covers commits, pushes, PRs, ordinary protected merges, manual workflow dispatches and a separate evidence branch. Publication, tags, release uploads, AUR submission and host package installation are excluded.

| Task | Requirements | Checks |
| --- | --- | --- |
| Record approval; inspect source, pending evidence and protections | R1–R6 | Review branch diffs and live protection/release baseline |
| Open and merge Arch evidence PR | R1 | Documentation-only diff, both packaging checks, review threads |
| Transfer reviewed feature into isolated updated-main branch; renumber | R2, R3 | Focused view/import checks, preserved historical hashes/provisioning |
| Reconcile unreleased release documents | R3 | Contract/version/status/link review |
| Final feature gates and protected merge | R4 | Both configured Qt suites, hosted packaging and complete manual release |
| Exact-main gates and independent artifacts/evidence retention | R5, R6 | Source/checksum/version/lifecycle/upload assertions, candidate unchanged |

All tasks completed on 2026-10-12. Final feature and exact-main hosted gates passed; independently validated artifacts and actual records are retained on docs/v1.2-integration-evidence. Published stable remains 1.1.0; publication is separate. See [actual verification](verification.md).
