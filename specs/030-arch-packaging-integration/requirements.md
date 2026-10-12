# Requirements

Approved on 2026-10-12 by the explicit instruction to implement the supplied iteration 030 plan. Approval covers commits, pushes, one PR, manual workflow dispatches, protected ordinary merge and publication of the final evidence branch.

- R1: Integrate only reviewed iteration 029 tooling and evidence from an isolated origin/main worktree; preserve the original tree and iteration 028 evidence branch.
- R2: Require both configured packaging checks and the complete manual release workflow on the final PR revision and exact main merge commit; uploads must be skipped.
- R3: Independently validate downloaded Ubuntu/Arch checksum pairs, source provenance and hosted lifecycle evidence.
- R4: Retain final results separately on docs/arch-packaging-integration-evidence, based on verified main. Preserve historical hashes and failures.
- R5: Keep v1.1.0 tag, release body/assets, protections, package/QML versions, APIs, dependencies and renderer behavior unchanged. No publication, AUR or host package operations.
