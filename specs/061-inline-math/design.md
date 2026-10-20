Approval: explicitly approved by the user on 2026-10-19 for implementation and
verification of requirements, design and T1–T7. Merge/tag/publication excluded.

# 061 — Inline math design

Status: approved. Implements [R1–R8](requirements.md).

## Parser and owned model (R1–R3)

Add a default-off private cmark inline-math option and leaf node alongside the
existing private extensions. Recognize at raw inline tokenization in
`third_party/cmark/src/inlines.c`, before backslash handling, emphasis and entity
processing. Include dollars in special-character scanning only when enabled.
Scan each candidate within its physical line and preserve exact body bytes and
delimiter/body source boundaries. Failed candidates advance as ordinary Markdown;
avoid repeated rescans with per-line closer bookkeeping. Code and HTML consumers
retain precedence over recognition inside their consumed spans.

Link destination/title scanners consume their raw input without math. When a
bracket pair resolves to an image, reparse its original description with math
disabled before adaptation. This avoids treating `![…]` that fails image resolution
as a protected image, and prevents formula punctuation from leaking into alt text.
Preserve existing reference resolution and extended-autolink reparsing options.
Table splitting must skip recognized formula bodies so literal pipes in TeX do not
create cells; code-span and escaped-pipe precedence remain unchanged.

Extend `ParseOptions`, `InlineKind`, `InlineNode`, `InlineContent` and `Block` with
owned formula nodes/spans. Source spans refer to normalized parser input; projection
spans are separate UTF-16 start/length pairs. Convert UTF-8 positions explicitly,
including multibyte text before formulas. Project each formula body literally once
in `inlineadapter`; do not rewrite Markdown or scan finished projection strings.
Forward spans through model/presentation and text/image slicing with rebased offsets.
Document bundled parser patches and keep private symbols hidden.

## Shared rendering requests (R4, R7)

Extract the asynchronous service behind `MathItem` into a private request abstraction
usable by block items and `FormattedText`. Requests carry source, effective size,
color, display/text rendering mode, cancellation and generation. Block items retain
display mode; inline requests use text mode. Immutable recordings add finite,
nonnegative ascent/descent metrics from engine baseline measurements, with their sum
covering recorded ink. Cache source, size, color and mode; retain current queue,
recording, operation, nesting and allocation limits and fresh-engine-input isolation.
No public C++ ABI, QML worker calls, resource downloads or extra engine are introduced.

Each text item owns its subscriptions. Input/style changes cancel superseded work;
Find source mode releases subscriptions. Unavailable/failed/saturated requests remain
source. Completion checks generation and lifetime before scheduling polish. Style
changes can rerender; no changes to document projection or selection are permitted.

## Text layout and mappings (R4–R6)

Extend the existing `FormattedText` painted-glyph pipeline rather than adding one
QML Text per fragment. Keep `m_text` as the stable source projection. A separate
layout string replaces each ready, fitting formula body with one neutral object
placeholder. Maintain explicit piecewise UTF-16 layout-to-projection mappings and
inverse boundary mappings; ordinary text maps directly and an atom maps to its
whole body interval. Source mode uses identity mappings. Never expose placeholders
through copy, Find, accessibility, link labels or heading identities.

Use public `QTextLayout`/`QTextLine` shaping and cursor APIs, with a line-composition
pass that reserves measured atom advances and adjusts glyph positions as existing
inline-code padding does. Do not assume a placeholder glyph has the desired width.
Measure actual placeholder advance, replace it with atom advance, and break only
at text break opportunities or atom boundaries. Force candidate line endpoints
using public line column controls and retry shorter endpoints when reservation
exceeds width. Use a progress-guaranteed single-grapheme/atom case for narrow lines.
Maintain directional visual cells from shaped bidi runs; an atom uses a neutral
object's bidi position. Account for atom ascent/descent when positioning baselines,
underline decoration, following lines and ink bounds. Preserve code padding and
horizontal alignment in the same width calculation.

Paint immutable recordings at atom baselines and omit placeholder glyphs. Project
cursor cells, link regions, selection rectangles and Find highlights through the
mapping. Atom cells expose only body start/end for new hits. Selection geometry
highlights the entire atom for any overlap without rounding stored source offsets.
Links retain their logical identity and activation destination across atom/text
splits. Existing accessible text receives body descriptions; formula atoms add no
independent interactive child. A formula wider than the content width switches back
to ordinary source layout, including when asynchronous metrics arrive. Recompute
on width/font/style/result changes without mutating the document model.

The trade-off is additional line composition/mapping complexity inside the existing
private layout item, preserving shaping, native drawing and document-wide selection.
Using separate QML fragments would fragment bidi, baseline and selection behavior;
a document-level rich-text renderer would violate the architecture.

## API, compatibility and delivery (R1, R8)

Expose the opt-in at QML 1.15 using existing module revision conventions, forward it
through the private parser/model, and keep older imports/default behavior covered.
Prepare package 1.15.0 compatibility metadata without altering published release
records or claiming publication. Add playground samples covering both delimiters,
links, tables, fallback and independent block/inline toggles. Update math/API guides
and current roadmap wording to distinguish delivered block math from draft inline
math, footnotes and provider work. Feature 062 specifications begin after 061's
approved implementation/verification; 063 follows 062. Each has a separate PR.

## Implementation notes

Table splitting tokenizes a temporary inline tree so formula protection reuses
existing code/HTML/link/image grammar rather than introducing a second scanner.
Owned formula source coordinates retain cmark line/byte-column conventions;
projection spans and all text layout mappings use UTF-16. Table cells now receive
source coordinates before inline parsing. Line composition reserves atom advances
and uses explicit column endpoints when adjacent zero-advance placeholders cannot
be separated by width reduction alone. These are implementation details within the
approved design; the observable scope is unchanged.
