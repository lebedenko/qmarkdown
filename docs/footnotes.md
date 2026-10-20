# Markdown footnotes (QML 1.16)

`MarkdownView.footnotesEnabled` defaults to false, independently of aggregate GFM.
Qt >=6.8 and C++17 remain sufficient. Footnotes use native Qt Quick blocks and
host-owned visual, navigation, scrolling, resource and clipboard policies.

```qml
import QMarkdown 1.16
MarkdownView {
    footnotesEnabled: true
    markdown: "A note[^label] and again[^LABEL].\n\n[^label]: Note **body**."
    onLinkActivated: function (destination) {
        if (destination.startsWith("#")) navigateToFragment(destination)
        // The host handles other destinations and navigationRequested targets.
    }
}
```

References use `[^label]`; definitions use `[^label]: body`. Definitions may appear
before or after references, including in lists and quotes. They cannot interrupt
an existing paragraph. Body continuation blocks require four spaces relative to
the definition's container; ordinary quote/list prefixes remain outside that indent.
Blank lines can separate continued body blocks. Labels use existing reference-link
Unicode case folding and trimmed/collapsed whitespace. First definition wins;
later duplicate markers remain visible literal syntax. Undefined references remain
literal. Unused winning definitions and their bodies are hidden.

Code, literal HTML tokens/blocks and formula bodies protect their contents.
References inside resolved links or image descriptions, and nested footnote syntax
inside note bodies, remain literal. With footnotes disabled, ordinary CommonMark
reference-link parsing applies, including reference definitions whose labels begin
with a caret.

Notes number by first reference in main-document traversal order. Superscript
references share one numbered endnote per definition, appended in number order.
Bodies use existing body/link/list/spacing styles and support ordinary Markdown,
enabled math and highlighting, custom fenced visuals and authorized images.
Note-body headings join the recursive heading outline. There are no popup notes.

Every occurrence and note has a local fragment reserved after all heading IDs.
Generated destinations avoid heading collisions, preserving main heading IDs.
References and per-occurrence backlinks emit `linkActivated(destination)`;
`navigateToFragment()` resolves them and emits the existing host navigation request.
Hosts choose scrolling and destination behavior. Use emitted destinations rather
than constructing generated IDs. Backlinks join the existing keyboard focus chain,
activation and accessible Link model, with names identifying the occurrence.

Copy and literal Find use `[n]` for references and `n. ` followed by each note body
in display order. Existing paragraph/list/code separators are retained. Decorative
backlinks contribute no copy/search text. References select as whole markers;
retained partial selections copy their exact substring. Find highlights the same
stable projection, and asynchronous body math/images retain those offsets.
