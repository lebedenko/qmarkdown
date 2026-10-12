# Design

Approved on 2026-10-13 through the explicit user reply "approved". Scope covers local implementation and verification of requirements, design and tasks; Commit, push and PR creation were subsequently authorized on 2026-10-13 through "commit, make a PR". Merge and publication remain separately authorized delivery actions.

## Parser and model

Retain privately bundled cmark 0.31.2. Add a narrow, default-off private parser option and task-state getter, with the existing symbol prefixing and hidden visibility. In `blocks.c`'s pre-inline processing, inspect only a paragraph that is its list-item parent's first child. Recognize the raw paragraph prefix before escapes, entities or reference links are resolved; store ordinary/unchecked/checked state on the item and remove only the recognized prefix before inline parsing. Do not preprocess whole Markdown strings or infer task state from projected text. Existing cmark block precedence and reference-map handling remain authoritative.

Update the bundled-source provenance to describe this local patch, private APIs and tests. Write the small recognition routine locally from the specification; copying upstream implementation would require retained attribution and license review. No new dependency, network access at build time, or parser replacement is proposed. The trade-off is maintaining one bounded private patch instead of migrating the CommonMark baseline for a single extension.

Extend private `Block` with a tri-state task field, retain it through image projection, and expose a private list-item model role. Extend `parse` with a default-disabled option so existing callers and corpus checks retain their current path. `ViewState` owns the public alias's boolean, reparses on changes and uses the existing resource-generation replacement lifecycle. Add the alias in `MarkdownView.qml`.

## Presentation and API

Keep existing bullet/ordered markers. Reserve an additional indicator gutter only on task rows; children continue through the existing recursive sequence. Draw the box and check using Qt Quick primitives rather than font-specific checkbox glyphs or a Qt Quick Controls dependency. Use body color and line metrics, center on the first body line, and clamp indentation to leave the existing one-pixel content minimum. Mark the indicator with Qt accessibility checkbox role, checked state and a read-only description; provide no handlers or focus behavior. No style properties or task-toggle signal are added in this slice.

The additive candidate is package 1.3.0 / QML 1.3, preserving 1.0/1.1/1.2 imports and Legacy defaults. Version updates, installed fixtures and docs are included in implementation. Example viewer adds an opt-in task-list toggle and a mixed/nested sample; it owns all host interaction.

## Delivery boundary

Before implementation, reconcile the actual integration baseline: iteration 034's local verification still lists hosted completion as pending. Preserve current user edits and historical evidence. Follow [development workflow](../../docs/development-workflow.md): one feature PR with its evidence, required minimum/development checks and squash merge by default. Commit, push, merge, tag and release actions require their own existing or explicit authorization. Publication is not implied by approval to implement.

Baseline reconciliation on 2026-10-13: local main is `8f1a5f3`, the merge of iteration 034's PR #13. The pending statement in the historical preparation record is not treated as current integration status. Existing user edits to AGENTS.md, README.md, specs/README.md and the new development-workflow document were preserved.
