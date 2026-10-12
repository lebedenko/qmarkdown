# Design

Approved on 2026-10-12 through the explicit instruction to implement the Iteration 028 plan, including documentation reconciliation, evidence integration, protected PR merge and post-merge verification.

Create docs/post-release-upkeep from docs/v1.1.0-publication-evidence. Refresh origin and incorporate newer main commits with an ordinary merge. Update only current summaries; label earlier candidate/version/environment descriptions as historical without altering numbered feature records. Published binary compatibility covers the bounded Ubuntu 24.04 x86_64 Qt 6.8.0 archive on exact Qt 6.8.0/6.12.0 runtimes, not a universal ABI guarantee.

Review the complete diff against main, local Markdown links outside illustrative examples, JSON validity, whitespace and retained log hashes. Push and open “docs: reconcile published v1.1 status and integrate evidence.” Require packaging (minimum) and packaging (development) on the final revision, then merge normally without bypasses. Verify the merge SHA and both main checks; confirm the unchanged tag commit. Record actual results separately from planned checks, with final post-merge evidence on docs/post-release-upkeep-evidence. No new local runtime tests or release are needed.
