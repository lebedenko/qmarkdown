# Requirements — Standalone display math

Approval: requirements, design and tasks approved by the user response “approved” on 2026-10-16. Dependency audit and verification remain implementation tasks; no success is implied. One cohesive 1.13.0 / QML 1.13 release PR is planned; merge, tagging and publication require separate authorization.

Release delivery approval — 2026-10-17: the user authorized the release plan for PR #36, including documentation completion, fresh final-head Qt 6.8.0/6.12.0 CI and retained full gate, squash merge through normal protections, `release.yml` dispatch with `release_pr=36`, tagging `v1.13.0`, stable/latest publication and downloaded hash verification of exactly four assets. Retain package 1.13.0 / QML 1.13 and existing APIs; no runtime changes. Stop on failed gates, changed inputs or differing published bytes. Historical approvals and results below remain unchanged; this approval supersedes earlier statements that delivery still requires authorization.

R056.1: Extend the existing private block parser behind mathEnabled, without regex preprocessing. Opening/closing delimiter lines contain only `$$` and optional whitespace, with up to three indentation spaces relative to the current container. Support top-level, list and quote containers.

R056.2: Require closure in the same container; unmatched delimiters remain ordinary Markdown. Do not interrupt an existing paragraph or recognize math in code/literal HTML. Preserve existing GFM interactions and all 652 default-disabled CommonMark semantic passes.

R056.3: Preserve the TeX body without inline processing. Single dollars, same-line `$$…$$` and backslash-bracket syntax remain ordinary Markdown. Container prefixes/delimiter lines are not part of the formula body; retain body whitespace after container handling and existing newline normalization.

R056.4: Use feature 055's native renderer/viewport without code decorations. Fitting formulas center; wide formulas scroll horizontally at natural size. Selection enabled, active Find, backend absence/errors and limits show source. Search/plain-text copy use the body without delimiter lines. Toggle changes reparse the document and restore normal Markdown when disabled.

R056.5: Deliver with feature 055 in the same 1.13.0 / QML 1.13 PR, including fixtures, examples, guidance and actual verification. No inline math or additional display delimiters.

## Development supersession — 2026-10-18

Feature [057](../057-atomic-math-selection/requirements.md) supersedes only the requirement to show source when selection is enabled: eligible native formulas remain rendered and select atomically. Find still shows source. Historical approval and verification records above are unchanged.
