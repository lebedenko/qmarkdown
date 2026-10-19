# Opt-in task lists

See the [public API reference](api.md) for the complete supported contract.

Read-only task lists are available since QML 1.3. Package 1.10.0 / QML 1.10 adds replaceable indicators.

```qml
import QMarkdown 1.3

MarkdownView {
    width: 480
    taskListsEnabled: true
    markdown: "- [x] Completed\n- [ ] Pending\n  - [X] Nested task"
}
```

`taskListsEnabled` defaults to false. Changing it reparses the current document and restarts resource handling through the existing resource policy. Reassigning its current value does nothing. The original `markdown` string is preserved; the option is independent for each view.

With the option enabled, the first paragraph of a list item can begin with a bracketed space or other ASCII whitespace character for an unchecked task, or `x`/`X` for a checked task. The closing bracket must be followed by whitespace before further content. A marker-only item is accepted, including at end of input, and reserves a body line. Ordered and unordered lists, mixed ordinary/task rows, nested lists, block quotes, tight lists and loose lists are supported. The recognized prefix and its separating whitespace disappear from the displayed paragraph; remaining formatting, links and image descriptions retain their usual behavior.

Escaped brackets, brackets produced by character references, markers in code/HTML, standalone paragraphs and markers in later paragraphs remain ordinary content. `[x]text` is not a task marker. A reference definition remains a definition under the CommonMark core rules; reference links in task labels resolve normally.

Indicators appear alongside existing bullets or numbers, so ordered numbering is preserved. Only task rows reserve an additional gutter. Body font metrics control indicator size and body color controls the box/check; live style edits update them. Indentation clamps to leave one content pixel at positive widths. At extremely narrow widths, the indicator is clipped. Nonpositive widths suppress layout as usual.

The native indicator exposes checkbox role and checked state through Qt accessibility, with a read-only description. It accepts no pointer, keyboard or accessibility toggle action and does not edit the source. Hosts can update a task by replacing `markdown`. There is no task-toggle signal or clipboard operation.

The playground has a **GFM task lists** toggle and sample. **Host-styled task indicators** is initially enabled and uses the active Qt Quick Controls checkbox appearance; turn it off to compare the library fallback. The viewer keeps its backing checkbox hidden, enabled and unfocusable and presents its public indicator separately. Its palette follows the viewer.

## Host presentation (1.10)

`MarkdownView.taskIndicator` defaults to null, selecting the library artwork: a transparent rounded outline and continuous antialiased checkmark in `style.bodyColor`. Its square preferred size is `max(16, round(body line height × 0.85))` logical pixels.

```qml
import QtQuick
import QMarkdown 1.10

MarkdownView {
    width: 480
    taskListsEnabled: true
    markdown: "- [x] Completed"
    taskIndicator: Component {
        Rectangle {
            required property bool checked
            required property MarkdownStyle markdownStyle
            implicitWidth: 20
            implicitHeight: 20
            color: checked ? markdownStyle.bodyColor : "transparent"
            border.color: markdownStyle.bodyColor
            radius: 4
            Accessible.ignored: true
        }
    }
}
```

The component must create an Item with the two required properties above. The library supplies live bindings, including style replacement. Supply `implicitWidth` and `implicitHeight`; the library owns actual geometry. Each invalid, nonfinite or nonpositive preferred dimension falls back to the default size. Artwork keeps its natural dimensions, with an eight-pixel content gap; the indicator and first text line are centered within their taller height. Taller indicators increase row height. Narrow views clip artwork while reserving at least one content pixel.

Custom components are trusted application code and **must be decorative**: no focus, pointer handlers, toggle actions or accessibility nodes, including in descendants. The containing library item owns the single read-only checkbox accessibility node. A Controls checkbox itself is therefore not a suitable component root; see the viewer’s [decorative adapter](../examples/viewer/TaskIndicator.qml).

Replacement destroys old instances. Creation failures or non-Item roots produce a diagnostic and show the fallback. Changing appearance does not reparse Markdown, change task state or reset document selection. Qt Quick Controls is a viewer/host dependency only.

## Replacement markers (1.14)

Set `MarkdownView.taskIndicatorsReplaceBullets: true` to place unordered task
indicators in the marker lane, without bullets or a second task gutter. The default
is false. Ordinary rows retain bullets and ordered tasks retain numbers with adjacent
indicators. The option applies recursively and does not change task state, source,
selection endpoints or copied text.

The marker lane accommodates custom implicit dimensions and retains an eight-pixel
text gap. Indicators center against the first text line; the first block reserves
the complete indicator band before subsequent blocks or nested lists. Task rows
reserve at least two logical pixels between consecutive indicator bounds, including
tight lists. Artwork keeps its natural dimensions when space permits. At widths
too small for the lane and text gap, artwork is clipped while at least one content
pixel remains. The indicator remains one read-only accessible checkbox.

The playground's “Style controls” panel includes “Task indicators replace bullets”
to compare the layouts.


In the playground, open Style controls and enable GFM task lists. Large host task indicators doubles the existing Controls artwork and defaults off. Disable Host-styled task indicators for fallback comparison. The sample includes empty, wrapped, tight/loose, mixed, nested and ordered cases. Sample selection preserves settings.
