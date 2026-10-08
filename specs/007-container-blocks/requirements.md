# 007: Native lists and block quotes

**Approval:** On 2026-10-08 the user explicitly requested implementation of the supplied Feature 007 plan. Approval covers these requirements, design, tasks, and package 0.5.0 / module 0.5.

- R1: Parse normalized Markdown once with bundled cmark 0.31.2; preserve private recursive blocks and CommonMark container semantics, without claiming full conformance.
- R2: Adapt inline formatting, entities, escapes, soft spaces and hard newlines. Links/images/autolinks expose inert labels; definitions disappear; HTML stays literal.
- R3: Native recursive lists/quotes reuse leaf delegates. Ordered starts/delimiters, widest-marker gutters, empty lines, tight/loose spacing and narrow-width clamping are required.
- R4: Add notifying listIndent=24, quoteIndent=16, quoteRuleColor=#808080, quoteRuleThickness=2; preserve style ownership/reset. Numeric rendering clamps finite negatives and defaults nonfinite values.
- R5: Playground mixed sample, palette colors, controls/presets/reset; deliver 0.5.0/0.5 and verify shared/static packaging, privacy and inert resources.

Deferred: resource APIs, navigation, images, task lists, selection, streaming and extensions.
