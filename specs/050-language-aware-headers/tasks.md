# Tasks — Language-aware headers

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Publish QMarkdown 1.11, then implement 1.12 code-block usability” plan. Scope includes Features 050–053 and one package 1.12.0 / QML 1.12 release PR. Merge, tagging and publication of 1.12 require separate authorization.

- [x] Implement requirements using the design.
- [x] Test legacy and optional header properties, required initialization, aliases, unknown/empty metadata, lifetime and exact copy. Update playground header and resources.
- [x] Update public API documentation and installed metadata; inspect the source allowlist (no new library files).
- [x] Record focused checks, candidate full Qt 6.8.0/6.12.0 shared/static backend OFF/ON matrix, lint/import/relocated consumers, and final-input extracted-source/Arch checks.
- [x] Require both packaging jobs and the retained release gate on the final PR head; actual final-head results are tracked in PR #35.
- [x] Finalize 1.12 versions and release documentation together, preserving historical 1.11 records.
- [x] Open the single release PR, [#35](https://github.com/lebedenko/qmarkdown/pull/35), with final-head checks enforced; merge/tag/publication require separate authorization.

## Requirement-to-check mapping

| Requirement | Implementation/check |
|---|---|
| R050.1 | metadataAndCopy; backend OFF/ON consumers |
| R050.2 | defaultsAndLegacyHeader; existing action/renderer creation and replacement tests |
| R050.3 | viewerCopy; metadataAndCopy; existing exact action-copy tests |
