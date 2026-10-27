# Reading and navigation

QMarkdown 1.10.0 / QML 1.10 provides reader state and geometry. Hosts own controls,
application shortcuts, scrolling and link policy. No public C++ API is added.

## Keyboard links and accessibility

`keyboardNavigationEnabled` defaults false. Enable it to let plain Tab and
Shift+Tab enter and traverse drawable native links in document order. Traversal
does not wrap: boundaries continue through the host Qt focus chain. Host action
controls and custom renderer controls keep their own focus behavior. Enter/Return
activates the focused link once per nonrepeated press through
`linkActivated(destination)`. Hosts decide navigation and fragment resolution.
Traversal does not change selection or activate links; selection shortcuts and
Find remain available.

`keyboardFocusColor` defaults to a live binding to `style.linkColor`. Each visible
fragment of the active link receives a two-logical-pixel outline without layout
padding. Focus acquisition and changed focused geometry request visibility through
`navigationRequested(rect target)`; the host owns scrolling and clipping.

One logical link spanning text and loaded image rows remains one stop. Separate
links sharing a destination remain separate. Native headings, nested lists and
quotes, table cells and autolinks participate. Links inside image descriptions are
inert; descriptions contribute to the enclosing link's decoded name. Links with
no drawable text or image geometry do not participate.

Each drawable logical link exposes an accessible Link with decoded name,
destination, screen geometry, focused state and a press action, even when keyboard
navigation is disabled. Empty names fall back to the destination. Accessible
activation uses the same host signal. Disabling keyboard navigation clears link
focus and removes its Tab stops while preserving accessible activation.

Resizing, style changes and image loading preserve logical identities. Source or
effective parser-option replacement invalidates old targets and activation
callbacks. In [1.18 streaming](streaming.md), surviving link occurrences retain focus without automatic navigation. Caret navigation remains deferred. The playground enables
keyboard links and includes a Keyboard links sample.

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
        keyboardNavigationEnabled: true
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

## Development footnote targets (1.16)

With `footnotesEnabled`, note references and occurrence backlinks emit local
fragments through `linkActivated`. Pass them to `navigateToFragment()` and use
`navigationRequested` with the host's existing scroll controls. IDs are reserved
after headings to preserve heading destinations; note-body headings join the
outline. Decorative backlinks remain accessible/keyboard links while contributing
no copy/Find text. See [footnotes](footnotes.md).
