# Opt-in task lists

Read-only task-list indicators were introduced in the 1.3.0 / QML 1.3 source candidate and remain available in the current 1.4.0 / QML 1.4 candidate. Published 1.2.0 packages do not include this feature.

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

The playground has a **GFM task lists** toggle and sample. Task-list syntax follows [GFM 0.29-gfm section 5.3](https://github.github.com/gfm/#task-list-items-extension-) on the existing CommonMark 0.31.2 block core. Tables, strikethrough, extended autolinks and GFM HTML filtering are not enabled. This selected extension is not full GFM conformance. The default CommonMark corpus and extension tests are separate evidence; see [iteration 035](../specs/035-gfm-task-lists/verification.md).
