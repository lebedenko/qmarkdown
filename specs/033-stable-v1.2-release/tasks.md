# Tasks

Approved on 2026-10-12 through the explicit instruction to implement the supplied Iteration 033 plan. Approval covers protected ordinary merges, commits/pushes, workflow dispatches, annotated tagging, stable/latest publication, release uploads and independent downloaded-asset checks. AUR submission, host installation, production/dependency/workflow/protection changes and additional features are excluded.

| Task | Requirements | Checks |
| --- | --- | --- |
| Record approval; isolate release branch and integrate evidence | R1, R5 | Original checkout preserved, historical evidence hashes |
| Finalize release documentation and submit preparation PR | R1, R2 | Link/status/date/scope checks, git diff --check |
| Protected ordinary preparation merge | R2 | Both configured packaging contexts, resolved review threads |
| Exact-main hosted gates and downloaded candidate validation | R2, R3 | Source SHAs, Ubuntu runtimes, Arch lifecycle, skipped upload, checksums/provenance |
| Annotated tag and stable/latest publication | R3, R5 | Verified SHA, notes/date/flags, publication builds and upload |
| Independent downloaded-asset acceptance | R4 | Four assets/checksums/artifact equality, disposable Ubuntu/Arch lifecycle |
| Retain actual publication evidence separately | R5 | URLs/IDs/images/inventories/hashes, fixed tag/release bytes/protections |

All seven tasks completed on 2026-10-12 UTC. Stable/latest v1.2.0 is published with Ubuntu and Arch assets; independent downloaded-asset checks pass on both exact Ubuntu runtimes and fresh Arch. See [actual verification](verification.md) and [machine-readable evidence](run-evidence.json). Main, tag and published bytes remain fixed; evidence is retained separately.
