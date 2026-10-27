# Native math

QML 1.13 adds library-owned formula rendering through a private, bundled MicroTeX
Qt painter backend. The backend is built by default (`QMARKDOWN_WITH_MATH=ON`);
set it OFF for a source-only build. No runtime downloads or system TeX installation
are needed. Qt >=6.8 and C++17 remain sufficient.

```qml
import QMarkdown 1.13
MarkdownView {
    mathEnabled: true
    markdown: "$$\n\\frac{1}{2}+\\sqrt{x^2+y^2}\n$$"
    style: MarkdownStyle {
        math.scale: 1.25
        // math.color inherits bodyColor unless explicitly assigned.
    }
}
```

`mathEnabled` defaults to false per view; `mathAvailable` reports compiled engine
availability, independently of opt-in and individual render failures. The anonymous
group `style.math` exposes resettable `color` and `scale`. Invalid scale values use
1.0. Formulas use bundled mathematical fonts at the resolved body text size times
scale; the host body family does not replace mathematical glyphs.

Fenced blocks render natively when their first info token is exactly `math`
(case-sensitive). Registered host fenced renderers retain precedence and their
existing contract, including ordinary source fallback when a host renderer fails.
Native fenced formulas retain code surfaces and source-copy headers. Standalone
formulas have no code decorations. Both center when they fit and scroll
horizontally at natural size when wider than the available width. They use
`codeBlockScrollBar` and its existing normalized host contract. Vertical wheel
movement and unused horizontal movement pass to the host at viewport boundaries.

Standalone opening and closing lines must contain only `$$` and optional
whitespace, with at most three spaces of indentation relative to the current list
or quote container. Both delimiters must belong to the same container. An opening
cannot interrupt a paragraph. Code and literal HTML protect their contents;
unmatched delimiters stay ordinary Markdown. TeX body whitespace is preserved
after container prefixes and the parser's newline normalization. Single dollars,
same-line `$$…$$`, and backslash-bracket delimiters remain ordinary Markdown.

Since 1.14, native formulas remain rendered with selection enabled. Each
formula selects as one object: boundary clicks and Shift-click use its start/end;
double- and triple-click select its complete TeX body, including the retained
terminal newline. Dragging extends into surrounding text. Any nonempty overlap
highlights the formula rectangle, clipped to its viewport.

Active Find shows selectable literal body source. Closing Find restores eligible
formulas without changing selection endpoints. A retained partial-source selection
highlights the whole formula while copying only the exact selected substring.
Plain text copying and search omit fences/dollar delimiter lines and preserve body
whitespace and document separators. Missing resources, backend OFF, pending work,
invalid syntax and rendering failures retain selectable source fallback.

## Tested dialect

This is a restricted MicroTeX dialect, not full LaTeX. Tested families include
`\frac`, `\sqrt`, superscripts/subscripts, Greek commands (`\alpha`, `\Gamma`),
`\sum`, `\int`, scalable `\left`/`\right` delimiters, matrix environments
(`matrix`, `pmatrix`, `bmatrix`, `Bmatrix`, `vmatrix`, `Vmatrix`) and `aligned`.
Use `&` for columns and `\\` for rows. Common arithmetic/relation/arrow commands,
standard named functions, math font commands and accents are permitted. Literal
input is ASCII; use named Greek commands. Unknown commands reject the whole
formula. Custom command definitions, dynamic evaluation, external fonts/resources,
colors, dimensions and document commands are excluded. Mermaid remains deferred; host highlighting providers are available in 1.17. Footnotes
are described in the [footnote guide](footnotes.md).

## Work and resource policy

Engine access is serialized on one application-owned worker. Fonts register on the
GUI thread; layout produces immutable painter recordings. Generation/cancellation
checks discard replaced or destroyed items' results. Each request uses a fresh
formula and clears mutable predefined formula caches between inputs. Resource
reads are restricted to embedded assets; document input cannot choose a path.

Limits are 16 KiB UTF-8 input, 64 parser/layout nesting levels, 4096 engine shared
allocations and 100000 instrumented operations, 16384 logical pixels per axis,
4 megapixels of logical painted area, 512 px effective text size, 64 queued/running subscriptions, 1 MiB per recording,
and 64 cached results within a 16 MiB byte budget. Queue saturation falls back to
source; changing input or presentation can retry. Cache keys include source,
resolved size, color and display/text mode. No failed or partial result is cached.

Cancellation does not forcibly interrupt Qt font shaping, painter recording,
standard-library operations or trusted-resource initialization. Input and operation
budgets bound surrounding engine work; these individual backend calls have no
hard wall-clock deadline. Application shutdown waits for the active worker call;
view/document destruction does not wait. This is an in-process renderer, not a
hard real-time or process-isolated TeX service.

See [MicroTeX provenance](../third_party/microtex/PROVENANCE.md) and
[TinyXML-2 provenance](../third_party/tinyxml2/PROVENANCE.md) for pinned revisions,
private changes and licenses. The installed license payload accompanies the engine.

## Inline math (QML 1.15)

`MarkdownView.inlineMathEnabled` defaults to false, independently of
`mathEnabled`. With it enabled, single-line `$x^2$` and `\(x^2\)` use the same
bundled engine, `mathAvailable` and restricted dialect. Code, literal HTML tokens,
link destinations/titles and image descriptions protect their contents. Formulas
work in headings, paragraphs, links, lists, quotes and enabled table cells.

Dollar delimiters must be unescaped single dollars outside dollar runs. Openers
precede non-whitespace; closers follow non-whitespace and cannot precede a digit.
Empty, unmatched and multiline candidates retain ordinary Markdown behavior.
TeX bodies are literal, including Markdown punctuation, entities and backslashes.

Formulas use text-style math at the surrounding font size times `style.math.scale`
and the resolved math color. They align to the surrounding baseline and wrap as
complete objects. A formula wider than the complete content line shows wrappable
body source. Pending, unavailable, invalid and failed formulas also show body source.

Copy, Find, outline labels and accessible link descriptions use bodies without
delimiters. Rendered formulas select atomically; retained partial selections copy
exact substrings. Active Find shows source. Closing Find or completing rendering
preserves projection offsets. Inline math shares the block engine's budgets and
cancellation policy; cache identity also includes display/text rendering mode.

```qml
import QMarkdown 1.15
MarkdownView {
    inlineMathEnabled: true
    markdown: "Energy $E=mc^2$ and \\(x^2\\)."
}
```
