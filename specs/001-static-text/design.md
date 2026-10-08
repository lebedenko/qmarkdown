# 001: static text design

**Status:** Implemented and verified for the bounded static-text slice. User approval recorded 2026-10-07 via the implementation plan; covers existing R1–R7 requirements, design, and T1–T7 tasks. Actual results and limitations are recorded in [verification](verification.md).

This record describes the historical Feature 001 slice. [Feature 003](../003-inline-formatting/requirements.md), separately approved on 2026-10-07, supersedes its literal-inline behavior for paragraphs/headings and adds inlineCodeFont. Feature 001 verification remains historical evidence.

## Parser selection and migration

Select a focused private C++ parser for feature 001, with no additional dependency. Its authored code uses the repository MIT license. It implements only the [requirements](requirements.md), not a general Markdown dialect. Qt ≥6.8, C++17, module packaging, and MIT licensing from approved feature 002 apply unchanged; minimum-version execution remains unverified.

Compared alternative: [cmark](https://github.com/commonmark/cmark/tree/0.31.2), the CommonMark C parser, at 0.31.2. Its [license](https://github.com/commonmark/cmark/blob/0.31.2/COPYING) has a BSD two-clause core and additional notices for bundled material. Adoption would require preserving applicable notices and reviewing the selected distribution. It offers a full semantic AST and reduces future core parsing work, but adds a C dependency, static/shared packaging work, and source-span reconstruction to keep unsupported punctuation literal in this slice. Neither dependency nor fixtures are added now.

The focused parser minimizes first-slice packaging and fallback complexity at the cost of temporary code. Do not grow it into an ad hoc full CommonMark parser. Before core expansion, reconsider cmark under separate approval; replace private parsing and enrich private block/inline data without exposing dependency types. Migration will require adapting fallback behavior and expanding semantic tests; public markdown/style names can remain, but unsupported rendering changes need revised specifications. This choice makes no full-conformance claim. Upstream fixture licensing is separate from implementation licensing; see [standards](../standards.md).

## Deterministic block algorithm

Normalize CRLF and CR to LF and U+0000 to U+FFFD, then process lines in source order. Retain whether each line ended in LF for fallback preservation. Blank ordinary lines flush a paragraph and produce no block. Outside a protected fence, check fence opening, then ATX heading, then accumulate ordinary paragraph text using R1. At end of input flush the pending paragraph. Unsupported containers never create nesting: for example `> # title` remains literal paragraph text, while a following standalone `# title` becomes a heading.

Heading indentation and fence indentation use four-column tab stops: a leading tab from column zero reaches column four and is ineligible. ATX recognition and closing markers follow [CommonMark ATX rules](https://spec.commonmark.org/0.31.2/#atx-headings) structurally; contents remain literal. After stripping the opening delimiter/separator, a remaining all-`#` run can close an empty heading because the opening separator preceded it. Embedded runs without a whitespace boundary remain text; backslashes remain visible and do not form closing delimiters.

A fence opener has zero to three indentation columns and at least three consecutive backticks or tildes. Any remainder is allowed for tilde fences; a backtick opener is invalid if its remainder contains a backtick. It can interrupt a paragraph. A closer has zero to three indentation columns, the same marker character with a run at least as long as the opener, and only ASCII spaces/tabs afterward. Shorter runs, other markers, or suffix text do not close it. Indentation need not match the opener. These recognition rules follow [CommonMark fences](https://spec.commonmark.org/0.31.2/#fenced-code-blocks); presentation differs deliberately.

Flush a paragraph before an opener. Collect opener, interior, and closer into one private LiteralFallback block, preserving normalized text exactly, including line endings and blank lines. Resume ordinary parsing on the next line after a closer. An unclosed fence extends through EOF. Four-column fence indentation is ordinary text and supplies no protection. No unsupported-block precedence is implemented beyond these rules.

## Private implementation and native layout

Extend `src/QMarkdown` with private parser/document files, a private QObject model adapter, `MarkdownStyle` C++ QObject registration, and a QML `MarkdownView` composed from native Item, Column/Repeater, and Text items. Use a private C++ view-state adapter behind QML property aliases to implement the style RESET handler and read-only geometry notifications. Register internal adapters anonymously or in a private implementation module; expose no constructible parser/document/delegate types or stable AST access. Keep files in the existing qt_add_qml_module target and preserve shared/static installation rules.

Private blocks carry kind (Paragraph, Heading, LiteralFallback), text, and heading level. Parse fully on markdown replacement and replace the model as one coherent update. Bind layout to style changes without reparsing. Each Text uses PlainText, left alignment, no elision, and Wrap, which falls back to character wrapping for long words ([Qt 6.8 Text](https://doc.qt.io/qt-6.8/qml-qtquick-text.html)). Preserve internal tabs through native Text rendering; do not promise a fixed tab pixel width. Native glyph shaping/font fallback is Qt's responsibility.

There is no padding, background, clipping, scrolling, selection, or link handler. Each block width is effective view width. Wrapping cannot subdivide a glyph/cluster wider than the entire view; native glyph overhang at such extreme widths is allowed, while the item width remains constrained. Paragraph/fallback height is native wrapped text height; empty heading height is one native line measured in its font. Gaps equal max(0, blockSpacing), occur only between blocks, and never accumulate at document edges. LiteralFallback uses body typography. `contentHeight` and `implicitHeight` reflect this layout even if explicit height is smaller; hosts own overflow and scrolling. Nonpositive width suppresses geometry until a positive width is supplied. `implicitWidth = 0` avoids a width/height binding cycle; hosts must bind width explicitly.

## Public QML contract

The public usage is:

```qml
import QtQuick
import QMarkdown

Item {
    width: 480
    MarkdownStyle {
        id: typography
        bodyFont.pixelSize: 16
        h1Font.pixelSize: 30
        bodyColor: "#202020"
        h1Color: "#303030"
        blockSpacing: 10
    }
    MarkdownView {
        width: parent.width
        markdown: "# Title\n\nBody"
        style: typography
    }
}
```

MarkdownView is an Item with writable `string markdown`, writable `MarkdownStyle style`, and read-only `real contentHeight`, in addition to inherited Item sizing properties. MarkdownStyle is a creatable QObject with writable `font bodyFont`, `color bodyColor`, `font h1Font` through `h6Font`, `color h1Color` through `h6Color`, and `real blockSpacing`. Each property has a change notification; whole-font assignment and QML font subproperty edits must both propagate. No heading-style array, inheritance chain, or additional public document API is introduced.

| Property | Default |
| --- | --- |
| bodyFont | Application default font family/style, with pixelSize 16 and normal weight |
| h1Font … h6Font | Same base family/style, bold weight, pixelSize 32, 28, 24, 20, 18, 16 respectively |
| bodyColor, h1Color … h6Color | Opaque #202020 |
| blockSpacing | 8 logical pixels |

Construct defaults from a snapshot of QGuiApplication's font when each style is created; clear point sizing when setting pixel size. Subsequent application-font changes require an explicit host update. Heading fonts/colors are independent, not inherited from body fields. Qt resolves host-assigned font values; the library adds no font scaling policy. Defaults favor legibility on light host surfaces; hosts supply suitable colors for other backgrounds. No platform-specific font family is required.

Each view owns one private default style. The public style property returns the currently effective non-null style. Omission, assignment of null, or QML property reset selects that default; resetting also restores its fields to construction defaults. Hosts can modify the returned default object. A supplied style remains host-owned; assignment never reparents or copies it and multiple views can share it. Replacing it disconnects the old notifications. Destruction of a supplied object safely restores the view's default style and notifies style change. Live supplied-field edits affect every attached view. Negative blockSpacing is clamped to zero for layout without rewriting the object; nonfinite values use default spacing 8. Colors may be transparent. Style changes settle by the next completed layout/polish cycle.

## Approval boundary

Approval recorded 2026-10-07 covers [requirements](requirements.md), this design, and [tasks](tasks.md). Public resource policies, streaming, extension registries, and full core semantics remain later work. Implementation is authorized within that scope; changes to scope require renewed approval.

Typography amendment: [Feature 008](../008-font-units/requirements.md) supersedes fixed pixel defaults and the pixel-only playground editor. The historical scope and approval above are preserved.
