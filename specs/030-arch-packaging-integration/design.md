# Design

Approved on 2026-10-12 by the explicit instruction to implement the supplied iteration 030 plan. Approval covers commits, pushes, one PR, manual workflow dispatches, protected ordinary merge and publication of the final evidence branch.

Use build/arch-packaging-integration in /tmp/qmarkdown-030, based on refreshed origin/main. Copy the bounded uncommitted iteration 029 change set and historical evidence without including iteration 028 evidence-branch-only changes. Review packaging, uploader, tests, workflow and install documentation together. Current summaries describe integrated tooling separately from published assets.

Run local checks first, then push and open one PR. Require packaging (minimum) on Qt 6.8.0 and packaging (development) on Qt 6.12.0. Dispatch release.yml against the branch and confirm headSha. Require Ubuntu shared/static/CommonMark/lint/consumer/benchmark builds, minimum binary lifecycle on both runtimes, and exact-commit Arch metadata/namcap/pacman/four-consumer/upgrade/removal checks. Upload must be skipped. Any correction requires fresh revision-specific gates.

Merge through protections with an ordinary merge commit. Repeat main checks and manual release gates for that exact merge SHA. Download artifacts and evidence; verify hashes and provenance independently. Publish final evidence on a separate branch based on verified main. Compare final release/tag/protections with the retained baseline. Rolling Arch builds establish per-run provenance rather than reproducible bytes.
