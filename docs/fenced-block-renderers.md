# Custom fenced-block renderers

QMarkdown 1.8.0 / QML 1.8 lets hosts replace fenced code content with native QML
Items. The surrounding code surface, padding, style and codeBlockActions header
remain library-owned. No renderer engines or extra dependencies are bundled.

```qml
import QtQuick
import QMarkdown 1.8

MarkdownView {
    width: 320
    markdown: "```preview details\nHello from source\n```"
    fencedBlockRenderers: ({ preview: previewComponent })
    Component {
        id: previewComponent
        Text {
            required property string codeText
            required property string infoString
            required property string language
            required property var markdownStyle
            text: "Preview: " + codeText
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            color: markdownStyle.codeBlockColor
            font: markdownStyle.codeBlockFont
        }
    }
}
```

The default map is empty. Matching uses the first whitespace-delimited token of
the parser-decoded info string, case-sensitively. `preview details` matches
`preview`; `Preview` does not. Empty/unmatched tokens and all indented blocks use
ordinary code. Only own map entries count. Assign a new complete map to update it:
`view.fencedBlockRenderers = { preview: anotherComponent }`. Nested map mutations
are unsupported.

The renderer must create an Item with the four required properties shown above.
`codeText` preserves parser-produced spaces, tabs and trailing newlines; `infoString`
is the complete decoded info; `language` is the matched token. The library supplies
these values during construction, assigns available content width and uses finite
nonnegative `implicitHeight` as reserved height (negative/nonfinite values reserve
zero). Bind internal layout to width and implicitHeight. `markdownStyle` receives
the view's effective style and follows replacement/live property changes.

Null entries, invalid/unready Components, incompatible root/property types and
creation failures produce diagnostics and ordinary code fallback. The library
validates an Item root, writable string properties and a writable markdownStyle
property. Declare properties `required` so Qt enforces construction initialization;
Qt's public metaobject API does not expose QML required declaration flags reliably,
so the library does not inspect those flags. No private Qt API is used.

Instances are library-owned and destroyed on document/component replacement,
selection-path replacement and teardown. Width/style updates retain the instance.
Keep the Component's creation context alive. Nested lists and quotes use the same
contract. Custom Items are trusted application code with the application's existing
capabilities; the library adds no engine, network access or resource permission.

When `selectionEnabled` is true, the view shows ordinary selectable source for all
code blocks, including when no text is selected. Disable selection to show custom
visuals again. Code actions continue to copy exact source, and document copying
uses the ordinary source projection. The view never writes the clipboard.

The playground's Custom fenced renderers sample uses native QtQuick Items only.
Enable Custom fences and disable Document selection to view it; re-enable selection
to inspect source. Bundled highlighting, Mermaid, math, inline extensions and public
C++ APIs remain deferred. See the [API reference](api.md) and
[Feature 042](../specs/042-custom-fenced-block-renderers/requirements.md).
