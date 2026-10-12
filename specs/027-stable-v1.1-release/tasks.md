# Tasks

Approved on 2026-10-11 through the explicit instruction to implement the Iteration 027 plan, including documentation integration, exact-commit verification, annotated tagging, stable GitHub publication and published-asset checks on both runtimes.

| Task | Requirements | Checks |
| --- | --- | --- |
| Integrate candidate evidence; record approval and finalize current release documentation | R1, R5 | Diff scope, links, date, retained compatibility/limits |
| Submit and merge release-preparation PR | R1, R2 | Both required packaging checks; ordinary protected merge |
| Verify final main and review artifacts | R2 | Exact SHA, both SDK packaging/build/runtime checks, checksum/metadata/images |
| Create annotated tag and publish stable/latest release | R3 | Exact tag commit, release flags/notes, publication workflow/upload success |
| Verify downloaded published assets on both runtimes | R4 | SHA-256/source metadata, disposable installer/consumer/removal checks |
| Record and push actual evidence separately | R5 | Complete provenance, fixed tag/main commit, clean evidence branch |

All six tasks completed on 2026-10-11 UTC. See [actual verification](verification.md) and [machine-readable evidence](run-evidence.json). Stable/latest v1.1.0 is published; both downloaded-asset runtime checks pass. Publication records remain on a separate evidence branch.
