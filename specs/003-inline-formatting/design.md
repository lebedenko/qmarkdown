# 003: inline formatting design

**Status:** Implemented and verified for the bounded Feature 003 slice. Approved by the user on 2026-10-07. Approval covers Feature 003 requirements R1–R7, the design, and tasks T1–T8; implementation and verification are authorized.

## Dependency boundary

Vendor the library sources for [cmark 0.31.2](https://github.com/commonmark/cmark/tree/0.31.2), retaining the complete applicable [COPYING notices](https://github.com/commonmark/cmark/blob/0.31.2/COPYING). Record the exact acquisition URL and SHA-256 of the acquired archive in the vendored provenance record. Generate required library configuration headers locally; do not fetch during configure/build. Enable C99 and embed a position-independent private object target into QMarkdown. Use a checked-in symbol-prefix mapping for all externally defined bundled symbols, including internal cross-translation-unit symbols, and hidden C visibility. Verify with symbol inspection. Do not export/install the object target or cmark headers. Install notices with the package licenses.

Using cmark avoids expanding the existing parser into an ad hoc CommonMark inline implementation. Keeping the block splitter preserves the accepted bounded structural behavior at the cost of a conservative inline adapter and source-span validation. No cmark internal API or HTML renderer is used.

## Inline adaptation

Keep normalization, paragraph assembly, ATX extraction, and fence protection in private/document.cpp. Pass only normalized paragraph/heading strings to a new private inline adapter. Prepend a fixed ordinary-text sentinel ending in a space before invoking cmark's public document parser, forcing a single paragraph even when content resembles a block construct. The sentinel must contain no Markdown punctuation. Require one document paragraph and no unexpected block siblings; empty heading content may bypass parsing. Remove exactly the sentinel from the first text contribution and fail conservatively if it cannot be identified.

Walk supported text, emphasis, strong, and code nodes into a private result containing display QString plus sorted, coalesced, nonoverlapping UTF-16 ranges with emphasis/strong/code flags. A flag stack accumulates nesting. Text literals are decoded by cmark; code literals use cmark normalization. Preserve recognized link/image/autolink/HTML nodes by copying their complete original source span and skipping children. Ordinary unresolved references remain text and may participate in normal inline formatting. Do not collect reference definitions: the sentinel prevents a definition from establishing a separate definition block.

Interpret public source positions against the prefixed UTF-8 input. Validate line/column bounds, monotonic spans, UTF-8 boundaries, and the sentinel offset before slicing. Convert validated byte spans to QString only after validation. Account for cmark's public position conventions explicitly in authored tests, including multibyte characters, escaped punctuation, code, and adjacent preserved nodes. If any position or tree invariant fails, discard partial output/ranges and return the complete normalized original block literally. Own the document root with deterministic cleanup and release it after adaptation; retain no cmark node in the model. Protected fences bypass this adapter entirely.

## Model and native rendering

Extend private Block and BlockModel roles with formatting ranges and a formatted-content discriminator. Register the rendering item only in QMarkdown.Private and extend its tooling metadata; no public constructible implementation types or document API are added. MarkdownView uses the Qt.labs.qmlmodels DelegateChooser available in Qt 6.8 to select the existing PlainText Text path for blocks without formatting and literal fences; a private QQuickPaintedItem handles formatted blocks. Keep block font/color selection and Column spacing in existing conventions.

The painted item receives display text, ranges, containing font/color, code font overlay, and logical layout width. Resolve the code overlay against the containing font; then apply italic and max(current weight, Bold) for accumulated flags. Use QTextCharFormat/QTextLayout::FormatRange directly, never rich text. Schedule layout through polish on relevant changes, cache QTextLayout, and paint the cache without parsing. Use WrapAtWordBoundaryOrAnywhere and native line heights. Decoded LF characters are mapped to Qt line separators only in the layout string, preserving display text and UTF-16 range offsets; this matches the PlainText delegate without adding Markdown hard-break semantics. Preserve empty-heading line metrics and suppress layout at nonpositive widths. Compute ink bounds separately from logical wrapping width, translating painting and extending raster bounds where needed without changing the parent block's wrapping width or block order. Do not clip glyph overhang to logical width.

Add inlineCodeFont to MarkdownStyle using its existing setter/notification/default-restoration pattern. Construct its default QFont with only QFontDatabase's system fixed-font family resolved; preserve the resolve mask so size, weight, italic, and other fields inherit. Font resolve-mask changes must invalidate layout even if resolved visual fields currently compare equal. Existing view-state style ownership, reset, replacement, and destruction behavior applies unchanged.

## Integration and verification

Bump project/module versions to 0.2.0/0.2 and update examples, tooling metadata, and installed consumers. Retain shared, plugin-only shared, and static packaging paths. Extend the viewer's editable examples with nested formatting, escapes/entities, code, literal resource syntax, and an alternate code font. Update current documentation only after implementation verification, clearly separating Feature 003 from Feature 001 history.

The [task/check mapping](tasks.md) defines the approval scope. Verification records actual commands, Qt/compiler versions, counts, failures, limitations, and viewer capture inspection. Qt 6.8 execution may be claimed only if available and tested; adding minimum-version infrastructure is excluded.
