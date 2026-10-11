# Requirements

Approved on 2026-10-11 through the explicit instruction to implement the Iteration 027 plan, including documentation integration, exact-commit verification, annotated tagging, stable GitHub publication and published-asset checks on both runtimes.

- R1: Integrate iteration 026 evidence and finalize 1.1.0 release notes, installation instructions, changelog and current documentation through a protected ordinary PR merge. Preserve historical records.
- R2: Verify the exact resulting main SHA with both hosted packaging checks and manual release builds/compatibility checks; review minimum archive/checksum/metadata and both runtime artifacts before tagging.
- R3: Create annotated v1.1.0 at the verified SHA, publish stable QMarkdown 1.1.0 as latest using finalized notes, and require the existing publication workflow and upload to succeed.
- R4: Independently verify downloaded published bytes and source metadata, then run archive/installer/1.0/1.1 consumer/removal checks in disposable Ubuntu 24.04 containers with exact Qt 6.8.0 and 6.12.0.
- R5: Retain workflow/release/tag/asset/image/checksum evidence separately from the tagged commit. Preserve package 1.1.0/module 1.1, Legacy defaults and host ownership; no production/API/dependency/workflow/protection changes.
