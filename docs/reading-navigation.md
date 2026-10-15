# Reading and navigation

QMarkdown 1.9.0 / QML 1.9 provides reader state and geometry. Hosts own controls,
focus, keyboard shortcuts, scrolling and link policy. No public C++ API is added.

## Find

Set `searchActive: true` and assign `searchQuery`. The query is literal and searches
the decoded plain-text projection used for selection, including formatted text,
tables, code and image descriptions. Synthetic list prefixes and block separators
also belong to that projection. Matches are non-overlapping, in document order.
`searchCaseSensitive` defaults false and uses Qt Unicode case comparison. There is
no regex, replacement or locale-specific normalization.

`searchResultCount` is zero while inactive or the query is empty.
`currentSearchResult` is zero-based, or -1 when no result exists. Activating Find,
changing query/case, replacing source or changing effective parser options selects
the first result. Masked parser switches, layout changes and resource updates
preserve the active index. `searchNext()` and `searchPrevious()` return bool,
wrap through results and request navigation. Closing Find retains query/case.

`searchHighlightColor` (default `#70ffff00`) and
`currentSearchHighlightColor` (default `#b0ff9900`) paint beneath selection without
changing layout, selected text or clipboard requests. Image-description matches
highlight the native image rectangle when the image is loaded. Active Find
replaces custom fenced visuals with searchable source. Closing Find restores
eligible custom visuals when selection is disabled. Code actions keep their source
payloads. Find does not enable selection or own keyboard shortcuts.

## Outline and fragments

`headings` is a read-only snapshot array of `{level, title, fragmentId}` entries
for H1–H6 in document order, including headings nested in lists and quotes. Titles
are decoded text. Treat snapshot entries as read-only; changing a local JS copy
does not change the document. `navigateToHeading(index)` uses a zero-based index.

Generated IDs lowercase decoded titles, remove Unicode punctuation, collapse
whitespace runs to hyphens and trim whitespace-derived edge hyphens. Letters,
digits, combining marks and symbols remain. Empty results use `section`. IDs are
allocated in document order; collisions try `-1`, `-2`, etc., skipping all allocated
IDs. Thus `Foo`, `Foo`, `Foo 1` produce `foo`, `foo-1`, `foo-1-1`, while `Foo-1`
produces `foo1`. This is not a claim of GitHub slug compatibility.

`navigateToFragment(fragment)` requires a leading `#`, percent-decodes the suffix
once as UTF-8 and matches generated IDs exactly. Plus signs remain plus signs.
It returns false for empty, missing or nonlocal targets and emits no request.
`linkActivated(destination)` remains unchanged: the host decides whether to call
the resolver or handle other navigation.

## Host scrolling

Successful navigation methods return true for logical targets even before layout
is available. `navigationRequested(rect target)` emits the target bounding rectangle
in MarkdownView coordinates after geometry becomes available. Layout/resource
changes refresh the most recently requested target; duplicate rectangles are
coalesced. Hosts decide whether a refreshed request should scroll again. Source
or effective-option changes discard prior targets and active Find selects its
first result. Invalid requests discard pending targets. Closing Find cancels a
search target. Zero-width views defer requests until laid out. Matches spanning
separators union drawable fragments; projection-only matches use nearby block
geometry. No method scrolls the view itself.

```qml
Flickable {
    id: scroll
    contentWidth: width
    contentHeight: reader.contentHeight
    MarkdownView {
        id: reader
        width: scroll.width
        markdown: "# Installation\n\n[Jump](#installation)"
        onNavigationRequested: function(target) {
            const point = reader.mapToItem(scroll.contentItem, target.x, target.y)
            scroll.contentY = Math.max(0, Math.min(
                Math.max(0, scroll.contentHeight - scroll.height), point.y))
        }
        onLinkActivated: function(destination) {
            if (destination.startsWith("#")) reader.navigateToFragment(destination)
            // Apply host policy to other destinations here.
        }
    }
}
```

The playground demonstrates a Find bar, Ctrl+F, Enter/Shift+Enter, Escape, a
collapsible heading outline and host-clamped scrolling.
