# Tasks — Bounded code-block scrolling

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Publish QMarkdown 1.11, then implement 1.12 code-block usability” plan. Scope includes Features 050–053 and one package 1.12.0 / QML 1.12 release PR. Merge, tagging and publication of 1.12 require separate authorization.

- [x] Implement requirements using the design.
- [x] Test height formula, short/empty/tiny/unavailable viewports, both axes, fallback, input handoff, anchor preservation, clipped selection, edge scrolling and Find.
- [x] Update public API documentation and installed metadata; inspect the source allowlist (no new library files).
- [ ] Record focused verification, full Qt 6.8.0/6.12.0 shared/static backend OFF/ON matrix, lint, imports, relocated consumers, extracted source and Arch checks.
- [ ] Finalize 1.12 release documentation together; open one release PR and pass final-head packaging and release gate. Preserve published 1.11 summaries during development.
