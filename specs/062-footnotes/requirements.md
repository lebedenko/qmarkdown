# 062 — Markdown footnotes requirements

Status: explicitly approved by the user on 2026-10-19.
Prepared from the richer-rendering plan on 2026-10-19. Target package 1.16.0 /
QML 1.16. Merge, tagging and publication are outside implementation approval.

- **R1 — Independent opt-in.** `MarkdownView.footnotesEnabled` defaults to false,
  independently of aggregate GFM. Disabled parsing preserves the pinned CommonMark
  baseline, including ordinary reference-link syntax that uses caret labels.
- **R2 — Definitions and references.** Support `[^label]` and `[^label]:` with
  document-wide forward/repeated references and multiline Markdown bodies. Definitions
  can occur in ordinary document containers. Continuation blocks require four spaces
  relative to the definition's container; blank lines belong to a note only when
  followed by valid continuation. A definition cannot interrupt an existing paragraph.
  Use existing reference-label normalization (Unicode case folding, trimmed/collapsed
  whitespace); empty labels are invalid. First definition wins; later duplicate
  definition markers remain visible literal syntax. Undefined references remain
  literal; unused winning definitions and their bodies are hidden.
- **R3 — Protection.** Code and literal HTML protect their contents. Do not resolve
  nested definitions/references in note bodies, references within existing links,
  or references inside image descriptions. Preserve ordinary Markdown parsing in
  note bodies while keeping excluded footnote markers literal. Formula bodies are
  already protected inline leaves. A failed image/link follows ordinary inline
  parsing rather than creating a new protected semantic container.
- **R4 — Numbering and body rendering.** Number notes by first resolved reference
  in main-document traversal order. Render superscript reference numbers and append
  exactly one numbered native endnote body per referenced definition, in number order.
  Repeated references share the note. Bodies support ordinary inline/block syntax,
  enabled math, enabled highlighting, resources and custom fenced renderers through
  existing policies/styles. Body headings participate in the recursive outline.
- **R5 — Host navigation.** Every reference occurrence and note has a collision-free
  local fragment, allocated after heading IDs. Preserve main-document heading
  destinations. References and per-occurrence backlinks emit `linkActivated()`;
  `navigateToFragment()` resolves generated targets through existing host-owned
  navigation requests. Hosts retain scrolling and destination policy. Backlinks
  appear in occurrence order and have meaningful accessible names. References and
  backlinks use existing keyboard link traversal and focus presentation.
- **R6 — Stable projection.** Copy and literal Find use `[n]` for each reference,
  followed by numbered endnote bodies in display order. Endnote numbering contributes
  `n. ` before its body, with existing list/body separators. Decorative backlink
  controls contribute no projection text. Keep generated navigation identities
  separate from source positions and display labels; asynchronous body rendering
  preserves offsets and existing selection/Find behavior.
- **R7 — Delivery.** One PR includes parser/model/native presentation, tests,
  deterministic playground sample, API/reader guidance, specifications and actual
  verification. Verify forward/repeated/undefined/duplicate/unused cases, container
  indentation, rich bodies, fragment collisions, backlinks, keyboard/accessibility,
  resource policy, exact copy/Find and disabled conformance. Cover old imports,
  installed shared/static consumers, backend ON/OFF interactions, lint/docs and the
  exact Qt 6.8.0/6.12.0 packaging gates; identify unavailable native-input acceptance.

Qt >=6.8, C++17, private C++ implementation and native Qt Quick rendering remain.
Popup footnotes, nested note resolution, Mermaid, streaming, host highlighting
providers (063) and public C++ interfaces are outside this feature.

Approval record: the user selected “Approve Feature 062” for requirements, design
and T1–T7 on 2026-10-19. Implementation and verification are authorized after
Feature 061's required CI checks pass. Merge, tagging and publication remain
excluded. No footnote runtime implementation had started at approval time.
