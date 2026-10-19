# 060 — Table presentation requirements

Approved 2026-10-19 by the user's instruction to implement the supplied “Table presentation and pending playground work — one 1.14 development PR” plan. Approval covers requirements, design, tasks, implementation, verification and opening one combined PR containing pending commit d11635a. No merge, tag or publication is authorized.

- R1: Resettable, notifying QML 1.14 table borderMode and alternateRowBackground; Grid default; alternate color inherits bodyBackground, preserving explicit transparency and overrides.
- R2: None, Grid, Rows and HeaderOnly native decoration, each shared boundary painted once. Alternate from second body row, per table.
- R3: Preserve layout, input, selection, Find, links, accessibility and resource behavior. Visible modes share existing Grid insets; None uses padding only. Sanitize invalid metrics and narrow widths.
- R4: Preserve feature 059 and add bounded playground table controls and expanded samples, retaining startup and persistence behavior.
- R5: Development package 1.14.0 / QML 1.14, compatibility and installed-consumer coverage; retain published 1.13 boundaries and deliver one unmerged PR with both packaging gates.

Follow-up approval on 2026-10-19: preserve the grouped API and document Qt’s lack of runtime revision enforcement for grouped members. Verify 1.14 metadata and retain an explicit expected-failure probe for older-import rejection. Strict runtime rejection is excluded from the approved compatibility contract.

## Release delivery approval — 2026-10-19

The user's supplied “Release QMarkdown 1.14.0” plan authorizes one release PR
from unchanged main after PR #39, current-release documentation and dated notes,
host Qt synchronization, documentation/release/whitespace validation, final-head
full Qt 6.8.0/6.12.0 shared/static packaging gates, squash merge and release.yml
dispatch with the release PR number. Publication includes stable/latest v1.14.0
with exactly source/Arch archives and their SHA-256 companions, followed by
successful downloaded hash verification. No runtime changes or new dependencies.
Stop on failed checks, moved main, conflicting tags or differing assets; preserve
historical approvals and evidence and create no post-release evidence commit.
This approval extends delivery scope without altering the historical exclusions above.
