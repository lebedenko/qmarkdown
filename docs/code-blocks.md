# Code-block usability

`MarkdownView` owns ordinary code-source layout. Hosts compose headers, provide
icons, perform clipboard writes, and optionally style the internal scrollbars.
These settings work with syntax highlighting enabled or disabled and apply to
both fenced and indented source. Inline code retains its existing layout.

| View property | Default | Meaning |
|---|---|---|
| `codeBlockWrapEnabled` | `true` | Wrap source within the available width |
| `codeBlockLineNumbersVisible` | `false` | Decorative source-line numbers on the physical left |
| `codeBlockHeightLimitEnabled` | `false` | Bound the source area independently of the header |
| `codeBlockViewportHeight` | `0` | Host reader viewport height in logical pixels |
| `codeBlockHeightRatio` | `0.5` | Reader-height fraction used by the limit |
| `codeBlockMinimumLines` | `8` | Minimum limit in effective code-font lines |
| `codeBlockMaximumLines` | `30` | Maximum limit in effective code-font lines |
| `codeBlockScrollBar` | `null` | Optional host scrollbar component |

The limit is `clamp(viewportHeight * ratio, minimumLines * lineHeight,
maximumLines * lineHeight)`. A missing/nonpositive viewport uses the maximum-line
limit. Invalid/nonpositive ratios use 0.5; line counts are at least one, and the
maximum is no smaller than the minimum. Short and empty blocks retain their
natural height. Header height, padding and scrollbar gutters are additional.
The limit affects presentation only: source is fully laid out and remains
available for selection, Find and copy.

Numbers use the effective code font, right alignment, and eight logical pixels
of padding on either side. Wrapped continuation rows have no number. One terminal
newline is omitted for display; real blank lines remain, and empty source has
line 1. `style.codeBlock.lineNumberColor` defaults to the effective code foreground
with alpha multiplied by 0.6. Assign `undefined` to reset it. Numbers are excluded
from selection, Find, clipboard payloads and accessibility nodes.

The source viewport reserves scrollbar gutters only for overflowing axes. Wrap
mode disables horizontal scrolling. Wheel/trackpad input and touch panning move
the source; wheel events at boundaries pass outward. Desktop selection retains
priority and can scroll at source edges. Source replacement resets offsets;
layout changes retain the first visible source position and clamp offsets.
Find reveals results internally before asking the host to navigate the outer
reader. Manual internal scrolling does not repeat that reveal.

## Headers

`codeBlockActions` components retain their `codeText` and `requestCopy` contract.
They can additionally declare any of these properties (including as `required`):

| Property | Meaning |
|---|---|
| `language` | Original case-sensitive first info token |
| `languageId` | Canonical alias identifier, or the unchanged unknown token |
| `languageName` | Friendly alias name, or the unchanged unknown token |
| `infoString` | Parser-produced complete info string |
| `fenced` | Whether the source originated in a fence |

Only declared writable properties are supplied, before component completion.
Language metadata is empty for indented or unlabelled blocks. Aliases for C/C++,
JavaScript/JSX, TypeScript/TSX, Python, QML, Bash and JSON resolve independently
of the optional syntax backend. Headers remain stationary above source and
numbers. `requestCopy()` emits the complete parser-produced source, including
content outside the visible area. The host performs the actual clipboard write.
Null headers remain supported.

Custom fenced visuals retain precedence and manage their own layout. These source
settings and the gutter apply when Find or a rendering fallback exposes ordinary source.

## Host scrollbars

The host component must produce an `Item` with writable `orientation`, `position`,
`size`, `active`, and `requestPosition` properties. The library creates a separate
instance for each overflowing axis and sizes it to the reserved ten-pixel gutter.
`position` is the offset divided by content extent; `size` is viewport extent
divided by content extent. Call `requestPosition(value)` with a normalized position
between 0 and `1 - size`. Values are clamped. Invalid components use a minimal
Qt Quick fallback; the library does not depend on Qt Quick Controls.

```qml
MarkdownView {
    codeBlockWrapEnabled: false
    codeBlockLineNumbersVisible: true
    codeBlockHeightLimitEnabled: true
    codeBlockViewportHeight: reader.height
    codeBlockScrollBar: Item {
        required property int orientation
        required property real position
        required property real size
        required property bool active
        required property var requestPosition
        // Supply host visuals and call requestPosition() from host interaction.
    }
}
```

The playground opts into nowrap, line numbers, height limiting, 50% of the reader
viewport, and 8–30 line caps. Its badges and copy icon are original playground
assets and are not included in the library source distribution.

## Development: nested wheel handoff

Source viewports consume complete wheel events while an internal offset moves.
Further outward input at an edge, and input over a block without overflow, reaches
the host reader. Direction reversal immediately moves the source again. Phased handoff follows the
host reader's native gesture threshold; consumed source deltas do not preload it.

A host `WheelHandler` that only observes scrolling must set `blocking: false` in
addition to `event.accepted = false`. On Qt 6.12.0 a default-blocking observer can
prevent its own Flickable from receiving mouse-wheel input even without QMarkdown.
The provider cannot undo a host handler's acceptance after handing an event off.
