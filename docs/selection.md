# Document selection and copy

See the [public API reference](api.md) for the complete supported contract.

QMarkdown 1.8.0 / QML 1.8 supports opt-in desktop selection across native blocks.

```qml
import QMarkdown 1.8

MarkdownView {
    id: document
    selectionEnabled: true
    markdown: "First paragraph\n\nSecond paragraph"
    style: MarkdownStyle { selectionBackgroundColor: "#603b82f6" }
    onCopyRequested: function(text) { hostClipboard.copyText(text) }
}
```

Assign a `MarkdownStyle` object to configure selection colors declaratively.
The host provides `hostClipboard`. The library emits text and never accesses the
clipboard. Hosts also own scrolling, clipping and action controls.

| API | Behavior |
| --- | --- |
| `selectionEnabled` | Defaults false; disabling clears selection. |
| `selectedText` | Read-only plain-text substring; empty for no selection. |
| `selectionStart`, `selectionEnd` | Read-only normalized UTF-16 projection offsets, start ≤ end. |
| `selectAll()` | Selects the complete projection when enabled. |
| `clearSelection()` | Resets both endpoints to zero. |
| `requestCopy()` | Emits `copyRequested(string text)` only when enabled and nonempty. |
| `style.selectionBackgroundColor` | Translucent selection background; follows preset/override/reset conventions. |

Drag with the left mouse button across blocks in either direction. Ordinary
clicks establish a new anchor; Shift-click extends it. A desktop mouse link click activates
its destination after the platform double-click interval when selection is enabled.
Repeated clicks, dragging and Shift-click suppress activation. With view
active focus, platform Select All and Copy shortcuts call the corresponding
methods; Escape clears selection. Moving focus to a host Copy button preserves
selection so it can call `requestCopy()`.

## Copy rules

Offsets address decoded rendered text, rather than Markdown source. Inline
formatting contributes no delimiters, links contribute labels and images contribute
alt text. Loaded image alt ranges are atomic. Wrapping, fonts, table alignment,
resource permissions/completion and code action headers do not change copy text.

Ordinary sibling blocks require two line feeds and list items require one. Existing
terminal line feeds count toward these boundaries; no whitespace is trimmed from
code or literal HTML. Lists include displayed ordered markers or `•`, spaces,
`[ ]`/`[x]` task indicators, and two-space indentation for nested lists. Quote rules
add no characters. Tables use tabs between cells and line feeds between rows,
including empty cells. Thematic breaks contribute boundaries without visible text.
Synthetic prefixes and separators have offsets without selectable glyph cells;
Select All includes them and cross-block selections include crossed boundaries.

## Lifecycle and interaction limits

Replacing Markdown, changing effective parser options or disabling selection clears
it. Changes masked by aggregate GFM enablement retain it. Resource updates, policy
changes, fonts/styles, resizing and delegate recreation retain offsets. Selection
uses Qt grapheme boundaries and the renderer's bidi, wrapping and inline-code
padding geometry. Highlights preserve existing text colors and layout.

Code action delegates are excluded from document gestures and copy text. Wheel and
touch scrolling remain host-controlled. Touch links retain their ordinary activation
behavior. Touch selection handles, full caret navigation,
rich-text/Markdown copying and editing remain deferred.

When selection is enabled, [custom fenced-block renderers](fenced-block-renderers.md)
show ordinary selectable source. Copy payloads retain source text; disabling selection
recreates eligible custom visuals.

## Repeated clicks (1.8)

Double-click selects a Unicode word using Qt word boundaries. Whitespace selects
its contiguous run; punctuation selects one grapheme. Loaded images select their
complete alt-text range. Triple-click selects a complete prose paragraph or
heading, a table cell, or one logical code/literal-HTML line including its newline
when present. Paragraph ranges survive image loading and include image alt text;
synthetic list prefixes and sibling separators are excluded. Dragging after a
repeated click extends by whole words or triple-click units in either direction.
Timing and distance use the platform's repeated-click settings.

## Host-owned edge scrolling (1.8)

Set `selectionViewport` to the visible scrolling Item and handle
`selectionScrollRequested(real deltaY)`. A null viewport disables requests. The
library never changes a Flickable or searches for an ancestor. For example:

```qml
Flickable {
    id: scroll
    width: 400; height: 300; clip: true
    contentHeight: document.contentHeight
    MarkdownView {
        id: document
        width: scroll.width
        selectionEnabled: true
        selectionViewport: scroll
        onSelectionScrollRequested: function(deltaY) {
            scroll.contentY = Math.max(0, Math.min(
                Math.max(0, scroll.contentHeight - scroll.height), scroll.contentY + deltaY))
        }
    }
}
```

During desktop selection dragging, the 32 logical pixels nearest either vertical
edge request linearly increasing speed up to 600 logical pixels/second, retaining
maximum speed outside the edge. Requests use a 16 ms timer with elapsed-time deltas
capped at 50 ms per tick. Positive deltas scroll downward. The stationary
window-space pointer updates selection as the host reveals content. Hosts clamp
their scroll position. Inside a code source viewport, desktop selection scrolls
internally on both axes, clamped to the source bounds. Internal vertical movement
has priority over outer scrolling. Once it cannot move, the outer reader receives
requests only when the pointer is within its own vertical edge zone; reaching a
code edge away from those outer edges never scrolls the page. Wheel and touch
boundary handoff are unchanged.

Release, cancellation, window deactivation, selection
disablement, viewport removal/destruction and source replacement stop requests.

## Rendered math (1.14)

Native [math formulas](math.md) select atomically while rendered, including their retained terminal newline on double/triple-click. Find shows TeX source; closing it preserves exact endpoints. A partial source selection highlights the full formula but copies only its selected substring. Host fenced renderers continue showing source during selection or Find.
