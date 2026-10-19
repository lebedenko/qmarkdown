# QMarkdown 1.14.0

<!-- current-release:start -->
QMarkdown 1.14.0 / QML 1.14, released 2026-10-19, keeps eligible native math
rendered during selection. Formulas select atomically; copying preserves TeX body
source. Find shows literal source and preserves exact endpoints when closed.
Host math renderers retain their existing source fallback. See the [math guide](math.md).

Nested code viewports hand wheel input to the host at boundaries and resume
internal scrolling immediately on reversal. Horizontal and Shift-wheel scrolling
remain supported. Host observing WheelHandlers need `blocking: false` as well as
`event.accepted = false`; see [code scrolling](code-blocks.md#nested-wheel-handoff-114).

`taskIndicatorsReplaceBullets` defaults false. When enabled, unordered task
indicators occupy the marker lane; ordered tasks keep numbers and adjacent
indicators. Task state, selection and copy stay unchanged. See [task lists](task-lists.md#replacement-markers-114).

`style.table.borderMode` defaults to `MarkdownStyle.TableBorderGrid`, with Rows,
HeaderOnly and None alternatives. Grid interior and outer lines have uniform
thickness. Borders remain transparent and zero-width by default.
`alternateRowBackground` inherits `bodyBackground` and shades every second body
row, excluding the header. Explicit transparency remains an override; `undefined`
restores inheritance. Individual property resets and `resetOverrides()` restore
preset defaults. Qt does not enforce revisions on grouped members; import 1.14
when relying on these controls. See [table styling](styling.md#table-presentation-qml-114).

The playground adds richer table/task samples, nested scrolling instructions and
style controls, preserving settings between samples. Demo defaults enable selection,
math, GFM, replacement markers and Polished dark presentation; table decorations
remain off at startup. Normal-sized host task artwork remains the default.

Existing 1.x consumers remain compatible. Package 1.14.0 accepts compatible
1.0–1.14 requests and EXACT 1.14.0. Linux amd64 minimums remain Qt 6.8, C++17 and
CMake 3.21; configured verification targets are Qt 6.8.0 and 6.12.0.

Assets: [1.14.0 downloads](https://github.com/lebedenko/qmarkdown/releases/tag/v1.14.0),
`qmarkdown-1.14.0.tar.gz` and `qmarkdown-1.14.0-1-x86_64.pkg.tar.zst`, each with a
`.sha256` companion. See [installation](release-install.md), [Arch installation](arch-install.md)
and [source instructions](source-install.md).
<!-- current-release:end -->

Actual checks and release delivery evidence are recorded in
[feature 060](../specs/060-table-presentation/verification.md).
