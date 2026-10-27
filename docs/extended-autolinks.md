# Opt-in GFM extended autolinks

See the [public API reference](api.md) for the complete supported contract.

The unpublished 1.5.0 / QML 1.5 source candidate adds selected extended-autolink support. Published 1.2.0 packages do not include this feature.

```qml
import QMarkdown 1.5

MarkdownView {
    extendedAutolinksEnabled: true
    markdown: "Visit www.commonmark.org or email foo@example.org"
    onLinkActivated: destination => console.log(destination)
}
```

`extendedAutolinksEnabled` is writable and defaults to `false`. It is independent of `taskListsEnabled` and `strikethroughEnabled`. Changing it reparses original Markdown through the existing resource lifecycle, preserves `markdown`, and emits its own notification without `markdownChanged`. Equal assignments do nothing.

Recognition follows [GFM 0.29-gfm section 6.9](https://github.github.com/gfm/#autolinks-extension-) on the CommonMark 0.31.2 core. Lowercase `http://`, `https://` and `www.` forms require a valid dotted domain and a beginning/whitespace/delimiter boundary. Web domains use ASCII alphanumeric characters, hyphens and underscores, with no underscores in the final two segments. Paths trim the specified trailing punctuation, excess closing parentheses and entity-like tails. `www.` destinations receive `http://`.

Bare email, lowercase `mailto:` and `xmpp:` forms are recognized in decoded inline text, including escapes/entities. Addresses use the section's narrower ASCII local-part/domain grammar and dotted domains; invalid trailing hyphens/underscores remain literal. Bare addresses receive `mailto:`. XMPP supports one resource after `/`, containing ASCII alphanumeric characters, `@` and periods; a second slash ends the link.

Explicit and reference links retain precedence, and links cannot nest. Code and HTML token contents stay literal. Image descriptions retain semantic links but present them inertly through the existing image fallback behavior. Generated links use native wrapping, inherited emphasis/strikethrough, existing link colors/underline and `linkActivated(destination)`. Hosts own navigation; enabling recognition grants no resource permissions and makes no network requests.

The playground has an independent toggle and representative sample. This is selected GFM extension support, not full GFM conformance. Tables and HTML filtering are available through independent switches. See [iteration 037 verification](../specs/037-gfm-extended-autolinks/verification.md) for planned and actual evidence.
