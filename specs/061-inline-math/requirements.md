Approval: explicitly approved by the user on 2026-10-19 for implementation and
verification of requirements, design and T1–T7. Merge/tag/publication excluded.

# 061 — Inline math requirements

Status: approved.
Prepared 2026-10-19 from the richer-rendering plan. That plan explicitly requires
approval before each feature's implementation. Runtime implementation is authorized by the approval above. Target package 1.15.0 / QML 1.15; published 1.14 is unchanged.

## Observable requirements

- **R1 — Independent opt-in.** Add `MarkdownView.inlineMathEnabled`, default false,
  independent of `mathEnabled` and aggregate GFM. Reuse `mathAvailable`,
  `style.math`, bundled MicroTeX and its restricted dialect. Qt >=6.8, C++17,
  private C++ implementation and native Qt Quick rendering remain required.
- **R2 — Syntax.** Recognize single-line `$…$` and `\(…\)` in paragraph, heading,
  list/quote, table-cell and link-label content. Dollar openers/closers must be
  unescaped single dollars outside runs of two or more dollars. An opener must
  precede non-whitespace; a closer must follow non-whitespace and must not precede
  a digit. Escape status uses odd/even immediately preceding backslashes.
  Parenthesis delimiters require unescaped delimiter backslashes; their nonempty
  body may contain whitespace. Empty, unmatched and multiline candidates retain
  ordinary Markdown behavior. The first eligible matching closer ends a formula;
  nested formulas are unsupported. Body contents undergo no Markdown emphasis,
  entity decoding or backslash processing. Parser newline normalization remains.
- **R3 — Protected syntax.** Code spans/blocks, literal HTML tokens/blocks, link
  destinations/titles and image descriptions do not recognize formulas. Inline
  HTML retains existing CommonMark token boundaries: text between separate HTML
  tokens remains ordinary inline content. A resolved image description is parsed
  without math, including when its bracket contents initially looked like text.
  Formula bodies can contain Markdown punctuation literally without changing the
  enclosing link or emphasis structure.
- **R4 — Inline geometry.** Render eligible formulas as indivisible baseline atoms,
  with ascent/descent included in line height. Wrap before/after complete atoms.
  Use the surrounding resolved font size multiplied by sanitized `style.math.scale`
  and its resolved color; retain bundled math fonts. If an atom exceeds the whole
  available content line width, show its wrappable body source. Pending, unavailable,
  invalid or failed work also shows body source. Reevaluate eligibility on resize.
- **R5 — Stable text.** Copy, Find, heading text and accessible descriptions use
  literal TeX bodies without delimiters. Active Find shows body source. Closing
  Find and asynchronous completion preserve projection text and selection offsets.
  Rendered atom hit testing selects its start/end; repeated clicks select the
  whole body. Any selection overlap paints the atom, while retained partial-source
  selections copy exactly their selected substring. Source fallbacks select normally.
- **R6 — Existing interactions.** Preserve enclosing links, keyboard traversal,
  accessible link labels, image/text splits, tables, recursive outline, resources,
  custom fenced visuals and existing block math/highlighting. Formula descriptions
  contribute once to accessible text; no additional keyboard focus stop is added.
  Hosts continue owning scrolling, navigation, clipboard and resources.
- **R7 — Bounded lifecycle.** Reuse existing engine budgets, serialized worker,
  cancellation and cache bounds. Rendering mode belongs in cache identity.
  Replacement, disablement and destruction discard stale work without waiting.
  Completion can relayout but cannot alter projection offsets.
- **R8 — Delivery evidence.** Include deterministic playground examples, API/math
  guidance, current roadmap corrections, older-import and installed shared/static
  consumer checks. Preserve the pinned CommonMark baseline with switches disabled.
  Run affected suites, lint/docs and required exact Qt 6.8.0/6.12.0 packaging gates,
  including math backend ON/OFF. Record actual results separately from plans.

## Scope boundaries

One implementation PR includes specifications, runtime, tests, documentation and
playground work. Merge, release publication and tagging require separate authorization.
Footnotes (062) and highlighting providers (063) follow sequentially with their own
approval documents and PRs. Mermaid, popup footnotes, public C++ provider APIs,
arbitrary provider typography and streaming are outside this change.
