# Requirements — document-wide selection and copy

Status: approved on 2026-10-14 by the user response "approved".
Approval covers implementation, verification, one focused PR and squash merge after required checks; publication remains excluded.
Target: package 1.7.0 / QML 1.7. Publication is not authorized.

- R1: Add opt-in `MarkdownView.selectionEnabled` (default false), read-only
  `selectedText`, `selectionStart` and `selectionEnd`, `selectAll()`,
  `clearSelection()`, `requestCopy()` and `copyRequested(string text)`.
  Positions are normalized UTF-16 offsets in the document plain-text projection.
  Empty selection has equal endpoints and empty text. Selection methods are inert
  when disabled except clearing; Copy emits only for enabled, nonempty selection.
- R2: Produce decoded rendered paragraph/heading text, link labels and image alt
  text without formatting delimiters. Ordinary sibling boundaries require two
  line feeds; list-item boundaries require one. Preserve existing ordered markers
  and bullet `•` markers, followed by a space; task indicators are `[ ]`/`[x]`
  followed by a space. Indent nested lists two spaces per level. Quote rules add
  no characters. Preserve code/literal HTML whitespace and parser terminal line
  feeds, adding only enough line feeds to satisfy boundaries. Tables use tabs
  between every cell and line feeds between rows, retaining empty cells. Thematic
  breaks contribute boundaries without visible characters. Resources, wrapping
  and action headers never alter copy text.
- R3: Mouse dragging selects across blocks in either direction; Shift-click
  extends the existing anchor (or establishes one if absent). Ordinary clicks
  establish a new anchor; ordinary link clicks still activate links. Drags and
  Shift-clicks suppress link activation. Resolve gaps to nearest text boundaries,
  table hits to their actual cell and images to atomic alt-text ranges.
- R4: With view active focus, standard Select All/Copy shortcuts and Escape call
  the corresponding methods. Preserve selection when focus moves to host controls.
  Hosts own clipboard writes and scrolling. Exclude action delegates from document
  gestures; preserve wheel and touch scrolling.
- R5: Add `MarkdownStyle.selectionBackgroundColor` through existing preset,
  override and reset conventions. Use a translucent highlight preserving text
  colors. Selection is grapheme safe and uses the rendered geometry, including
  bidi, wrapping, alignment and padded inline code; highlight image selection.
- R6: Clear before source replacement, effective parser-option changes or disabling.
  Preserve offsets through resource completion/policy changes, font/style changes,
  resizing and delegate recreation. Preserve disabled rendering and enabled empty
  block sizing and code/HTML terminal-newline display conventions.
- R7: Enable selection in the playground, wire Copy to its existing clipboard
  helper, add Select All/Copy controls and feedback, and document API/copy rules.
  Retain private C++ boundaries and dependencies; no document HTML, QTextDocument
  or WebEngine rendering.
- R8: Deliver implementation, tests, documentation and actual evidence in one PR;
  update version/compatibility fixtures together, synchronize development Qt before
  verification, squash merge after required packaging checks and resolved threads.
  Do not tag or publish.

Excluded: touch selection handles, edge autoscrolling, double/triple clicks, full
caret navigation, rich-text/Markdown copying and editing.
