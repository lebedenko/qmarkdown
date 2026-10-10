# Design

Approved on 2026-10-10 through the supplied release plan.

Use the existing protected-main PR workflow and Stable release package workflow without tooling changes. Merge documentation only after both required Qt checks pass. Manually verify the exact final main commit and review its Qt 6.8 archive and both cross-runtime evidence artifacts. Create v1.0.0 at that SHA and publish stable notes from docs/release-1.0.0.md. The publication workflow builds and uploads verified assets. Download those assets and use the existing verifier inside a disposable Ubuntu toolchain container; never install into host /usr. Record post-publication evidence separately so the tagged commit stays fixed.
