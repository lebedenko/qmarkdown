# Requirements

Approved on 2026-10-11 through the explicit instruction to implement the supplied Iteration 026 plan. Scope includes protected ordinary PR merges, documentation integration, and exact-commit hosted verification; tagging and publication are excluded.

- R1: Review and integrate CI PR #4 before styling PR #5 using ordinary merge commits; preserve ancestry and repository protections.
- R2: Retarget styling to main, confirm its diff contains only styling changes and supporting evidence, and wait for both required packaging checks on the updated base before merging.
- R3: Update the current roadmap/index and draft 1.1.0 release notes. Preserve package 1.1.0, module 1.1, Legacy defaults, existing 1.0 compatibility and host ownership. Keep the changelog unreleased; no new API/features.
- R4: Verify the exact final main candidate with hosted Qt 6.8.0/6.12.0 packaging: shared/static tests, strict 652-example CommonMark checks, QML lint and installed/relocated 1.0/1.1 consumers.
- R5: Manually dispatch the existing release workflow at that SHA. The same minimum-built archive must pass installation, consumption and removal on both runtimes. No release upload.
- R6: Retain candidate SHA, workflow/job URLs, exact SDKs, immutable image identities, archive metadata and SHA-256 separately from planned checks. Any corrective change requires fresh candidate checks.
