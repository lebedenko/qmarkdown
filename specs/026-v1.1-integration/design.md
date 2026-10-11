# Design

Approved on 2026-10-11 through the explicit instruction to implement the supplied Iteration 026 plan. Scope includes protected ordinary PR merges, documentation integration, and exact-commit hosted verification; tagging and publication are excluded.

Integrate in dependency order: merge CI preparation, retarget/review styling against main, wait for protected checks, then merge styling. Ordinary merge commits preserve the stacked branch ancestry. Do not modify protections or rewrite history.

Keep iteration approval and planned checks in repository-owned documents before integration. Prepare release-readiness documentation in a separate protected documentation PR after the source merges. Its final main merge commit is the candidate; the main push workflow verifies packaging and the existing release workflow is dispatched with the full SHA. Manual dispatch skips publication.

Record subsequent actual evidence in this iteration directory on a separate evidence branch, leaving candidate main unchanged. Download the minimum-built archive/checksum and both runtime evidence artifacts, independently verify the checksum and source metadata, and retain image identities from hosted evidence. Historical verification documents remain historical.
