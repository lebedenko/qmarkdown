# Requirements

Approved on 2026-10-12 through the explicit instruction to implement the supplied Iteration 032 plan. Approval covers commits, pushes, PRs, ordinary protected merges, manual workflow dispatches and a separate evidence branch. Publication, tags, release uploads, AUR submission and host package installation are excluded.

- R1: Merge pending Arch integration evidence through a documentation-only protected PR with both packaging checks and resolved review threads.
- R2: Preserve the original tree/branches; transfer reviewed copy actions from an updated-main isolated worktree to feat/code-block-copy-actions. Renumber their specification to 031, preserving historical approvals, logs, hashes and artifact paths. Keep main's Git provisioning.
- R3: Prepare unreleased 1.2 notes and reconcile README/changelog/index/roadmap; stable remains published 1.1.0. Preserve package 1.2.0/QML 1.2, codeBlockActions and codeBlockCopyRequested(string text), exact parser text, callback invalidation, default rendering, host clipboard ownership and 1.0/1.1 consumers. No additional features/dependencies.
- R4: Final feature head requires both hosted Qt 6.8.0/6.12.0 packaging passes and the complete manual release workflow before ordinary protected merge. Merge advancing main into the branch and repeat gates without rewriting history.
- R5: Exact main merge requires fresh packaging and complete manual release gates, same minimum-built Ubuntu archive on both runtimes and Arch install/ownership/consumers/temporary upgrade/removal; release upload must be skipped.
- R6: Independently verify downloaded Ubuntu/Arch checksum pairs, versions, source SHA and lifecycle evidence. Retain candidate/PR/run/job/image/distribution/checksum/success/failure records on docs/v1.2-integration-evidence based on verified main; leave candidate main unchanged.
