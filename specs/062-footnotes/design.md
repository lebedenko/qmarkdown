# 062 — Footnotes design

Status: explicitly approved by the user on 2026-10-19. Implements [R1–R7](requirements.md).

## Parse and resolve (R1–R4)

Extend the bundled private cmark parser with a default-off footnote option, a
container-aware definition block and a reference inline node. Recognize definition
markers before ordinary reference-definition removal; retain raw normalized source
and container-relative indentation for recovery. The note container consumes its
first-line body and four-space continuations; ordinary block parsing handles its
children. Existing list/quote prefixes are removed by their own containers before
note indentation is measured. Blank lines are buffered until continuation is known.

Reuse the existing reference-label normalizer through a private parser helper,
retaining original labels for literal fallback and normalized labels for resolution.
Do not invent a second case-folding convention. Definitions in a note body stay
literal, and its inline parse disables footnote resolution without disabling other
extensions. Code, HTML and formula consumers keep their established precedence.
After links/images resolve, suppress reference nodes in their labels/descriptions;
retain raw markers as text there so their contents do not become navigation links.

Adapt into owned private definition/reference nodes with separate source spans.
A document-wide pass collects definitions in source order, selects the first for
each normalized label, and walks main-document references to assign numbers. Remove
unused winning definitions. Duplicate definition markers project literally, with
ordinary Markdown parsing of their body text and continuation blocks; ensure they
cannot disappear through ordinary reference-link-definition recognition. Undefined
reference nodes revert to their exact original marker. Do not scan a completed
plain-text projection or rewrite input Markdown.

Append one endnote section built from the selected owned bodies, preserving recursive
block semantics. Move/copy each chosen body once into its endnote entry; repeated
references add backlinks rather than duplicate note content. Main-document reference
numbering excludes all note-body nodes. Image resources and custom fenced visuals
flow through the existing recursive presentation/resource pipeline.

## Presentation and projection (R4, R6)

Add private endnote entry presentation data containing number, body blocks and an
occurrence catalog. Reuse block sequences, list-style number gutters and existing
body/link/spacing styles; introduce no popup or host visual-identity dependency.
Endnote entries append after the main sequence with its normal section spacing.

Reference nodes contribute `[n]` to the stable UTF-16 projection but display only a
superscript number. Extend the explicit layout/projection mapping from feature 061
with a measured text atom for the number: shape the superscript label natively,
reserve its full width, and map it to the complete `[n]` interval. Keep the enclosing
logical link identity on its cell, including focus/accessibility geometry. New hits
select the complete marker; retained partial source selections copy exact substrings.
Do not substitute numbers by rescanning ordinary link labels. Find uses the stable
`[n]` projection and highlights that reference atom.

Endnote projection uses `n. ` followed by the body and existing recursive separators;
entries are separated by a blank line. Backlink controls are decorative for copy and
search and have zero projection length. Their explicit link geometry and accessible
labels are registered separately from ordinary text spans so they remain keyboard
reachable without adding invisible copy characters.

## Identities, outline and navigation (R5)

Build the recursive heading outline first, traversing main content before appended
note bodies. This retains existing main heading IDs and allows note-body headings
into the same outline. Then reserve generated note/reference fragments against the
complete heading-ID set. Prefer `fn-N` and `fnref-N-K`, adding deterministic numeric
suffixes on collisions. Publish no new promise about fragment spelling beyond local
uniqueness and stability for an unchanged document.

Store fragment targets independently of source coordinates, reference labels and
logical link IDs. References emit the selected note fragment; each backlink emits
its own occurrence fragment. Extend `navigateToFragment()` to check heading and
note/reference targets, using existing navigation target/rectangle delivery rather
than scrolling directly. Register geometry from the reference atom and endnote
entry; refresh after layout, resource loading and body math completion.

Extend the existing link controller with explicit zero-length decorative link
registrations for backlinks. Keep one logical accessible Link per occurrence and
backlink, with labels such as “Footnote 2” and “Back to reference 2, occurrence 3”.
Tab/Backtab and Enter retain existing host focus-chain and activation behavior.

## Compatibility and delivery (R1, R7)

Expose `footnotesEnabled` in QML 1.16 using established API/module conventions,
prepare package 1.16.0 metadata and retain earlier imports. Leave published release
records unchanged. Add a deterministic playground sample and independent toggle;
update API/reader guides and the current roadmap. Keep one implementation PR with
planned checks separated from actual results. Feature 063 receives its own concrete
specification review and explicit approval after this feature's implementation and
verification. Merge, tagging and publication require separate authorization.

Approval record: the user selected “Approve Feature 062” for requirements, design
and T1–T7 on 2026-10-19. Implementation and verification are authorized after
Feature 061's required CI checks pass. Merge, tagging and publication remain
excluded. No footnote runtime implementation had started at approval time.

## Implementation notes

Endnotes reuse native ordered-list/list-item presentation instead of introducing
a parallel numbered-body component. A zero-projection Backlinks presentation leaf
registers explicit decorative link geometry. Superscript numbers share the measured
atom/projection mapping machinery from Feature 061, with native text recordings
rather than math requests. Owned block/reference nodes retain parser source
coordinates independently of generated destination IDs. Parser teardown now frees
all retained private math/note leaf payloads. Observable approved scope is unchanged.
