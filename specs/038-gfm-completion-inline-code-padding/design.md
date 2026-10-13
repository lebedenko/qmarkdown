# Design — staged proposal

Status: staged design approved on 2026-10-13 by the user response "approved"; detailed inline renderer design approved on 2026-10-13 by the subsequent user response "approved". See [requirements](requirements.md), [investigation](investigation.md) and [tasks](tasks.md). Prototype evidence and the detailed design are approved; actual implementation verification is recorded separately.

## Parsing and lifecycle (R1–R3)

Replace the growing positional private parse booleans with one private parse-options value containing the five effective extension flags. Derive it in one ViewState helper from stored independent properties and gfmEnabled; route all source parsing through that helper. Setters return on equality, update only their stored property, reparse stored Markdown, reload resources/rebuild the model and emit that property's notification. Preserve the existing cancellation lifecycle even when a changed individual value is masked by the aggregate switch. Update private QML metadata and public aliases together.

Add private cmark table option and table/row/cell node kinds and alignment storage. Recognize header/delimiter pairs within the existing block parser, after container prefixes are handled; do not preprocess entire Markdown or replace the parser. Split escaped pipes with explicit grammar, including the special escaped-pipe handling inside table code spans; apply normal inline parsing to cell content. Require matching header/delimiter counts; normalize body rows to header width, retaining empty missing cells and ignoring excess cells. Respect blank-line and block-start termination. Extend private node ownership/freeing, traversal and test-only semantic serialization with the new kinds.

Retain table structure as owned blocks with row/cell children and column alignment. Append private kinds/roles without shifting existing numeric QML delegate values. Cell inline nodes/ranges/links/images retain existing ownership and projection rules. Extend recursive resource traversal through cells so cancellation, denied resources, replacement and decoder ownership use the existing controller.

Store original and filtered raw HTML separately for both inline and block nodes. A shared private scanner recognizes the nine names case-insensitively with valid tag-name boundaries and opening/closing forms, replacing only the qualifying leading angle bracket in the filtered representation. Do not alter visible text or source spans. Probe serialization selects filtered semantics only when enabled. Verify near-match names, attributes, case, closing tags and code exclusion independently.

## Native table presentation (R4–R5)

Extend BlockModel and the recursive BlockSequence delegate with table rows/cells. Equal widths derive from table width and column count; borders are contained within allocation. Each native cell surface wraps existing formatted-inline and image/segment presentation, with header bold and inherited foreground unless overridden. Row height is the maximum content height plus vertical padding. Add private text alignment to FormattedText so wrapping and hit geometry follow the same layout. Do not duplicate link handlers or bypass resource policies.

Use grouped TableStyle with existing MarkdownStyle value/override/reset machinery. Color defaults use inherited/transparent semantics. Clamp effective horizontal cell padding at extreme widths to leave usable content width, as existing code-block presentation does; border geometry must stay within allocation. Vertical padding and border width use nonnegative finite metrics. No scrolling or minimum table width larger than its container.

## Inline padding investigation gate (R6–R7)

Authorize a bounded prototype only after this staged proposal is approved. Compare QTextLayout-based layout-only spacing with visual-run placement using Qt-shaped glyphs. Keep source text/ranges immutable and explicit source-position mapping. Prototype must demonstrate padding for every wrapped visual code fragment, transparent-background spacing, increased row advance, grapheme-safe bidi placement and linked-padding hit testing. Repeated-layout convergence, finite work bounds and preparation cost must be measured rather than assumed.

Record selected algorithm, source/layout mapping, fragment identity, break selection, visual order, baseline calculation, paint bounds, cache invalidation and complexity in this document after successful gates. Obtain explicit approval of that detailed design before production integration. If no candidate passes, revise the design rather than implementing reduced paint-only behavior. See the comparison and acceptance checklist in investigation.md.

## Compatibility and delivery (R8–R10)

Change CMake project/module versions, package matrices and installed consumers together after implementation approval. Preserve older 1.x compatibility and reject incompatible future/exact requests. Tests should assert both interface availability and native behavior in shared-linked/plugin-only/static consumers. Keep strict CommonMark tests extension-disabled and supplement them with extension/mixed semantic expectations.

Keep actual evidence separate from planned gates. Update host controls and public guidance with default-off extensions and the new default code spacing. Release authorization is distinct from feature approval; final package inputs require exact-byte verification and checksums before publication, followed by independent download acceptance. Do not infer release authorization from iteration 037's 1.5.0 approval.

Trade-offs: a bounded bundled-parser patch preserves syntax/container precedence without adding dependencies. Equal-width tables sacrifice content-based allocation for predictable fitting. Layout-aware code padding increases renderer complexity; the prototype gate prevents accepting an unproven algorithm.

## Detailed inline renderer design — approved at A2

The [Qt 6.12 prototype](prototype/padding.cpp) passes 144 checks; [actual measurements](prototype/results-qt6.12.json) and investigation notes support the following approved design. The user explicitly approved this detailed design on 2026-10-13, authorizing production integration and verification.

Keep one cached QTextLayout over unchanged UTF-16 text, existing interval font/color/decoration formats and paragraph bidi shaping. Retain separate semantic code-span identities when projecting inline nodes, even if effective formatting intervals merge; image descriptions remain inert. Build a source-position-to-code-span map and source-position-to-link map once per preparation.

For each QTextLine, first lay out at containing width. Derive code fragments by sorting valid cursor cells into visual order and grouping contiguous cells with the same semantic code identity. Reserve twice horizontal padding per visual fragment through a reduced QTextLine width. Recompute fragments after wrapping; only increase the reservation budget, never oscillate. Stop when the actual fragment count fits the budget. The budget is bounded by the finite number of code-bearing grapheme cells; an indivisible grapheme may overflow at extreme widths as already documented. Conservative reservation can leave extra trailing space after a wrap reduces fragment count; accept this small fitting trade-off for deterministic termination.

Leave Qt in charge of word/grapheme break decisions. Add padding at both ends of each visual fragment, translating neighboring visual cells and shaped glyph positions by the cumulative insertion width. Use QTextLine glyph retrieval with glyph indexes, positions and string indexes, mapping cluster indexes back to valid source cursor positions. Never reshape bidi fragments independently or split grapheme clusters. Cells belonging to one ligature can overlap within one identity; union their geometry. Formatting boundaries separate code and noncode shaping. Retain paint segments with resolved foreground and decoration flags; preserve font fallback and Qt glyph-run underline/strike/overline behavior when drawing translated runs. Alignment is calculated against final padded width, including table-cell alignment.

Each code-bearing line advances by its native height plus twice vertical padding; baseline moves down by vertical padding. Code rectangles and linked-padding regions use those same fragment bounds. Link hit regions map each translated glyph cell and padding strip to original source/link identity; unrelated cells and blank tails remain inactive. If a visual fragment contains a link boundary, assign edge padding to the adjacent code cell's link, rather than assigning the entire fragment to an arbitrary link. Preserve source text and semantic UTF-16 ranges exactly.

Cache translated glyph runs, code rectangles and hit regions until text, font, interval/link/code spans, width, alignment, padding or DPI changes. Background/corner-radius changes repaint without reshaping. Include shifted glyph ink and padded backgrounds in raster bounds while parent geometry uses logical width and padded line advance. Zero horizontal and vertical padding use the original draw/geometry path for spacing compatibility.

Preparation uses source maps linear in UTF-16 length and sorting per visual line; Qt shaping/line and glyph retrieval costs remain governed by QTextLayout. Prototype observed two layout passes per code-bearing line on the measured corpus. Keep bounded monotone retries and measure production behavior; do not claim universal linear time. Advisory budget on comparable 15,000-character/1,000-span paragraphs: padded preparation within 2× the same implementation's zero-padding preparation; investigate regressions rather than introducing a machine-dependent timing test.

## Release delivery authorization — 2026-10-13

The supplied release plan explicitly authorizes D1–D3: commit the feature on a branch, open one PR, squash merge after required packaging checks and resolved review threads, run the release workflow against the exact merged commit, tag v1.6.0, publish stable/latest Linux x86_64 Ubuntu/Arch packages, independently accept downloaded packages and attach compact evidence. Preserve existing releases, assets and protections. No evidence-only PR or automatic post-release upkeep commit. Earlier pending-authorization statements retain their historical scope; actual delivery results will be attached to the release.
