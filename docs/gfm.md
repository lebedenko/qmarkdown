# GFM extensions in QMarkdown 1.10

<!-- current-release:start -->
See the [public API reference](api.md) for the complete supported contract.

Package 1.10.0 / QML 1.10 supports all five GFM extensions on the existing CommonMark 0.31.2 core. Inline code reserves 4/2 logical pixels of horizontal/vertical padding in every preset. This is selected extension support, not a claim of exact GFM 0.29-gfm conformance or GitHub's additional sanitization behavior.

```qml
import QtQuick
import QMarkdown 1.10

MarkdownView {
    width: parent.width
    gfmEnabled: true
    markdown: "| Name | Status |\n| :--- | ---: |\n| **Native** | `ready` |"
}
```

All six switches default to false. `taskListsEnabled`, `strikethroughEnabled`, `extendedAutolinksEnabled`, `tablesEnabled` and `tagFilterEnabled` remain independent. Each effective extension flag is `gfmEnabled || individualEnabled`. The aggregate switch does not change individual values; disabling it restores their configured behavior. Changing a switch reparses the stored source and cancels stale image work. Equal assignments do nothing. The Markdown source and its change notification are unaffected by extension toggles.

Tables retain header/body rows, inline formatting, escaped pipes (including code), column alignment and empty missing cells. Excess body cells are ignored; mismatched header/delimiter column counts leave ordinary Markdown. Tables terminate at blank lines or another block start and work inside lists and block quotes. Native cells receive equal shares of the containing width, wrap content and share the tallest cell's row height. Left, center and right alignment also applies to formatted text and authorized image rows. Headers use bold text. Very narrow widths can still overflow an indivisible glyph; tables introduce no internal scrolling.

Use `style.table` with QML 1.10:

| Property | Default in every preset |
| --- | --- |
| `headerForeground` | Inherits body foreground; an invalid/reset color restores inheritance |
| `headerBackground`, `bodyBackground`, `borderColor` | Transparent |
| `borderWidth` | 0 logical pixels |
| `horizontalPadding`, `verticalPadding` | 4 and 2 logical pixels |

Each property follows the existing override/notification/reset contract. Assign `undefined` to reset one property, or call `style.resetOverrides()` to reset all overrides to the selected preset. Effective layout metrics clamp negative/nonfinite values to zero. Horizontal padding and borders fit within each column at narrow widths. Hosts own visual identity, scrolling and link navigation. Images in cells use the same host resource policy, decoding limits and replacement/cancellation handling as other images; denied images retain their literal descriptions.

Tag filtering follows [GFM §6.11](https://github.github.com/gfm/#disallowed-raw-html-extension-). It retains a separate private filtered representation of qualifying raw HTML for semantic verification. Original HTML spelling remains visible literally in the native view. Enabling it does not hide tags, execute HTML, fetch HTML resources or introduce an HTML renderer. Code spans/blocks retain their literal content. See [feature 038 verification](../specs/038-gfm-completion-inline-code-padding/verification.md) for evidence and remaining gates.
<!-- current-release:end -->
