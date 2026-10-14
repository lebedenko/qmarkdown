# Opt-in GFM strikethrough

See the [public API reference](api.md) for the complete supported contract.

Native inline strikethrough was introduced in the 1.4.0 / QML 1.4 candidate and remains available in the current 1.5.0 / QML 1.5 candidate. It is not published; published 1.2.0 packages do not include this feature.

```qml
import QMarkdown 1.4

MarkdownView {
    strikethroughEnabled: true
    markdown: "~Single~ and ~~**bold** [link](custom:review)~~"
}
```

`strikethroughEnabled` is writable and defaults to `false`. It is independent of `taskListsEnabled` and `extendedAutolinksEnabled`; enable any combination. Changing it reparses the original source using the existing resource lifecycle, preserves `markdown` and does not emit `markdownChanged`. Equal assignments do nothing.

Following [GFM §6.5](https://github.github.com/gfm/#strikethrough-extension-), matching runs of one or two raw tildes delimit strikethrough. Existing emphasis whitespace/punctuation flanking rules apply. Longer, unmatched and unequal-length runs remain literal. Escapes and character references cannot create delimiters. Code and HTML token content stays literal, links retain their normal precedence and delimiters cannot cross paragraph boundaries. Soft and hard breaks within a paragraph are supported.

Native Qt font strikeout composes with bold, italic, inline code and link styles, including wrapped text, headings, list/task labels and image fallback descriptions. It uses the resolved font and existing colors; no custom strike styling is added. Link destinations, activation and host resource policies retain existing behavior. This implements selected GFM extensions, not full GFM conformance; tables and HTML filtering remain deferred; [extended autolinks](extended-autolinks.md) are available through an independent switch.

The playground includes an independent GFM strikethrough toggle and sample. See [iteration 036](../specs/036-gfm-strikethrough/verification.md) for actual verification evidence and limits.
