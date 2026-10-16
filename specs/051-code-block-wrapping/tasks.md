# Tasks — Code-block wrapping

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Publish QMarkdown 1.11, then implement 1.12 code-block usability” plan. Scope includes Features 050–053 and one package 1.12.0 / QML 1.12 release PR. Merge, tagging and publication of 1.12 require separate authorization.

- [x] Implement requirements using the design.
- [x] Test plain/native wrap and nowrap, long lines, tabs, Unicode, alignment and live toggles.
- [x] Update public API documentation and installed metadata; inspect the source allowlist (no new library files).
- [x] Record focused checks, candidate full Qt 6.8.0/6.12.0 shared/static backend OFF/ON matrix, lint/import/relocated consumers, and final-input extracted-source/Arch checks.
- [ ] Confirm both packaging jobs and the retained release gate on the final PR head.
- [x] Finalize 1.12 versions and release documentation together, preserving historical 1.11 records.
- [ ] Deliver the single release PR with its final-head checks; merge/tag/publication require separate authorization.

## Requirement-to-check mapping

| Requirement | Implementation/check |
|---|---|
| R051.1 | defaultsAndLegacyHeader; plainSourceScrolling; wrappingHeightGutterAndFind |
| R051.2 | metadataAndCopy; existing syntax and native layout tests |
| R051.3 | existing inline-code, renderer and styling tests |
