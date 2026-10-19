# 060 — Table presentation requirements

Approved 2026-10-19 by the user's instruction to implement the supplied “Table presentation and pending playground work — one 1.14 development PR” plan. Approval covers requirements, design, tasks, implementation, verification and opening one combined PR containing pending commit d11635a. No merge, tag or publication is authorized.

- R1: Resettable, notifying QML 1.14 table borderMode and alternateRowBackground; Grid default; alternate color inherits bodyBackground, preserving explicit transparency and overrides.
- R2: None, Grid, Rows and HeaderOnly native decoration, each shared boundary painted once. Alternate from second body row, per table.
- R3: Preserve layout, input, selection, Find, links, accessibility and resource behavior. Visible modes share existing Grid insets; None uses padding only. Sanitize invalid metrics and narrow widths.
- R4: Preserve feature 059 and add bounded playground table controls and expanded samples, retaining startup and persistence behavior.
- R5: Development package 1.14.0 / QML 1.14, compatibility and installed-consumer coverage; retain published 1.13 boundaries and deliver one unmerged PR with both packaging gates.

Follow-up approval on 2026-10-19: preserve the grouped API and document Qt’s lack of runtime revision enforcement for grouped members. Verify 1.14 metadata and retain an explicit expected-failure probe for older-import rejection. Strict runtime rejection is excluded from the approved compatibility contract.
