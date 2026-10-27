# Design

Approved 2026-10-27 through the implementation request and supplied plan.

Start from c2895f9 on a new branch; retain diagnostic commits on their original branch. Keep accepted source and batching timer in private ViewState. Full parse each published snapshot and regenerate projections and metadata. Guard notification reentrancy and lifetime with QPointer and publication state. Keep ordinary replacement reset semantics.

Recursively reconcile presentation models, comparing all content and metadata. Preserve equal prefix/suffix rows, update compatible rows, and remove/insert incompatible rendering kinds. Resource reconciliation uses stable append-only request slots until a full restart; removed slots become cancelled tombstones, preventing callback index reuse. Retain existing admission deadlines, failures, images and limits.

Map old semantic block ranges to retained blocks and rebase selection and Find positions. Reconcile logical link targets by surviving occurrence, label and destination. Block-local action identity replaces whole-document identity. Hosts retain scrolling ownership. Full parsing, rather than incremental parsing, is an explicit scaling tradeoff.

## Subsequent release delivery approval — 2026-10-27 UTC

The user explicitly approved the supplied “Release QMarkdown 1.18.0 / QML 1.18” plan and requested implementation. This extends the original local-only authorization to commit, push, open one release PR, protected squash merge and stable/latest publication. Historical local scope and evidence above remain unchanged. Host-installed package replacement remains excluded.

Retain the implemented streaming feature, tests, playground and documentation together, plus changes merged since 1.17.1. Add no features; permit contract fixes. Keep package 1.18.0, QML 1.18, SOVERSION 1 and dependencies. Finalize dated release notes, changelog and all active release/install/compatibility documentation using 2026-10-27 UTC. Describe full snapshot parsing, retained state, binding ownership and host scrolling, with bounded measurement claims.

Run task check, git diff --check and scripts/check-docs.py --release. Existing local Full evidence supports review; the latest final-head hosted Full matrix on exact Qt 6.8.0/6.12.0 and retained qmarkdown-ci-v2 gate authorize publication. Open “Release QMarkdown 1.18.0”; after passing protections squash merge, keep main fixed, dispatch release.yml with the PR number, and require verified-tree, package and Qt identity gates. Verify extracted source builds with syntax OFF/ON on both targets and focused Arch checks. Publish source/Arch archives and their SHA-256 companions under v1.18.0 stable/latest; completion requires all four downloaded assets matching checked bytes. Stop on failure, moved main or differing assets; never move tags or overwrite differing bytes. Keep delivery evidence in the PR/Actions without an evidence-only follow-up PR or upkeep commit.
