# Document selection and copy

See the [public API reference](api.md) for the complete supported contract.

QMarkdown 1.7.0 / QML 1.7 supports opt-in desktop selection across native blocks.

```qml
import QMarkdown 1.7

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
clicks establish a new anchor; Shift-click extends it. A plain link click activates
its destination, while dragging or Shift-click suppresses activation. With view
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
behavior. This desktop scope provides no touch selection handles, automatic edge
scrolling, double/triple-click selection, full caret navigation, rich-text/Markdown
copying or editing.
