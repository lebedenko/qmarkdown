# Design

Approved on 2026-10-12 through the explicit instruction to implement the supplied Iteration 032 plan. Approval covers commits, pushes, PRs, ordinary protected merges, manual workflow dispatches and a separate evidence branch. Publication, tags, release uploads, AUR submission and host package installation are excluded.

Integrate Arch evidence first. Preserve the original working tree and branch by exporting only reviewed feature changes to an isolated worktree based on updated main. Apply source/test/consumer changes and documentation deliberately; retain existing Arch integration and CI Git provisioning. Rename current copy-action references to 031 while retaining historical evidence bytes and original approval descriptions. Document the numbering correction explicitly.

Prepare candidate documentation and the approved iteration records on the feature branch. Gate its final revision on strict protected packaging checks and all manual release jobs before an ordinary merge. Any source correction or updated base invalidates earlier gates. Verify the exact resulting main SHA again, dispatching by branch after confirming identity and checking workflow headSha.

Download distribution artifacts and checksums plus runtime/Arch evidence; verify hashes and source provenance without installing packages on the development host. Commit actual records separately from planned checks on an evidence branch based on the candidate. Preserve existing protections and v1.1.0 tag/body/assets. No publication occurs.
