# 003: inline formatting requirements

**Status:** Implemented and verified for the bounded Feature 003 slice. Approved by the user on 2026-10-07. Approval covers Feature 003 requirements R1–R7, the design, and tasks T1–T8; implementation and verification are authorized.

## Scope

Extend the bounded static-text slice with inline emphasis, strong emphasis, and code in paragraphs and ATX H1–H6. Target package version 0.2.0 and QML module version 0.2. This is not full CommonMark conformance.

| ID | Requirement and acceptance criterion |
| --- | --- |
| R1 | Apply CommonMark 0.31.2 emphasis, strong emphasis, escapes, character references, and code-span normalization to normalized paragraph and heading content. Nested formatting accumulates. Unmatched delimiters and invalid references retain ordinary text behavior. Existing trimming, soft-line joining, block recognition, order, replacement, and protected fence behavior remain authoritative. |
| R2 | Recognized links, images, autolinks, and raw HTML retain their original source spelling, without adapting their children. Unresolved reference forms remain ordinary inline text; reference definitions are not collected. No resource requests, navigation, or link activation occur. Invalid source spans or unexpected adapter tree shapes cause the entire normalized block to render literally. |
| R3 | Keep MarkdownView.markdown, style, and contentHeight unchanged. Add writable, notifying MarkdownStyle.inlineCodeFont supporting whole-font assignment, QML subproperty edits, shared styles, replacement/destruction, and default reset. Its construction default specifies only the system fixed-font family; unspecified font fields inherit from the containing block. Color always follows the containing block. |
| R4 | Emphasis enables italic; strong raises resolved weight to at least bold. Code applies its font overlay against the containing block font, then surrounding emphasis/strong flags accumulate. Code has no background or padding. Plain content and protected fences retain PlainText Text rendering. Formatted blocks use native QTextLayout ranges through a private Qt Quick item. |
| R5 | Preserve zero-width suppression, wrapping with character fallback, bidi/shaping, empty-heading lines, inter-block spacing, implicit sizing, host-owned scrolling, and existing layout/polish timing. Width and live style changes invalidate cached layout safely without reparsing markdown. Logical wrapping width remains independent of raster bounds so glyph overhang is not inadvertently clipped. |
| R6 | Bundle pinned cmark 0.31.2 privately with complete applicable notices, acquisition URL, and checksum. Builds and installed shared/plugin-only/static consumers require neither downloads nor an external cmark package. Public installed APIs expose no cmark headers/types/targets; bundled external symbols are prefixed and hidden in shared builds. Install third-party notices. Qt ≥6.8 and C++17 remain requirements. |
| R7 | Extend the editable viewer and installed-consumer fixtures with formatted content and live code-font edits. Verify parser boundaries, native font/layout behavior, lifecycle, resource inactivity, private type isolation, QML lint, and relocated packaging. Record actual results and inspect a viewer capture. |

## Superseding behavior and exclusions

R1–R4 intentionally supersede Feature 001's literal-inline requirements for paragraphs/headings and add one style property. Feature 001's historical verification remains intact. Unsupported block semantics, hard breaks, semantic fenced blocks, selection, streaming, resource policies, extension renderers, and official conformance-suite imports remain outside this slice. No compatibility switch preserves the old literal-inline behavior.

See [design](design.md), [tasks](tasks.md), and [verification](verification.md).
