# Evaluation design

Approval: owner's supplied implementation request, 2026-11-05; evaluation only.

Start with a standalone public-API placement gate. A QTextLayout owns shaped
positions; glyphRuns returns values. Mutating returned QGlyphRun positions must
not be mistaken for updating the source layout. Render the same layout before
and after moving copied glyphs, and compare exact captures. Independently paint
the original/moved runs to establish that the requested placement really differs.
This is a negative capability test for the existing prepared-run adapter, not a
proof that every possible Qt architecture is impossible.

Run independent distance-field/curve processes, retain actual screenshots,
identities and observed API/DPR. A timer moves the item through real rendered
frames; frameSwapped intervals are application swap notifications, not compositor
presentation timestamps or GPU work. Never label these as display qualification.
An external process deadline survives GUI/render-thread stalls. Stage records
flush before application creation, layout, exposure, capture and completion.

The demonstrator uses the actual tested QSGTextNode, with no proxy or shipping
library changes. Interactive mode remains open for inspection; it is a placement
control, not an authoritative full-document 2.x demonstrator. Expansion to padded
composition requires an adapter with explicit cluster/source mapping and bidi
semantics. Per-word layouts, transparent selection emission and synthetic spaces
are not accepted shortcuts. Inline objects must reserve width/ascent/descent in
the same authoritative layout used for selection and hit testing.

Skia must be a native independently built control with explicit font files and
shaping dependencies. Browser CanvasKit or Qt raster output cannot stand in for
native Skia GPU measurements. Missing dependencies are reported as missing
qualification. Production dependency changes remain outside scope.

Full qualification must use paired minimal/realistic fixtures, an actual-source
interactive viewer and browser references. Resource generations are keyed by
content/style/width/fonts/DPR; scroll changes transforms only. Record bounded
fallback work and invalidation, not just average frame times. Measure presentation
feedback separately from captures, CPU timings, GPU queries and resident/texture
memory. No backend is selected from a successful build or a replay-only test.

## Approved demonstrator continuation

The standalone demonstrator lives under demonstrator/. Its manifest indexes
source spans in UTF-16 code units; browser and Skia adapters must translate native
offsets explicitly. Normal blocks advance by measured height. The old overlapping
scene remains a historical probe.

Qt preparation is copied into an isolated build stage from a recorded Git commit;
only the staged copy receives inspection hooks. Prepared glyphs/cursor cells/code
rectangles and object recordings remain authoritative for fallback painting and
selection. Raster origins floor the transformed ink bounds in physical pixels;
painting includes the residual phase, and texture placement cancels that phase.
Native nodes retain their layout until width/style/selection changes. Fallback
work is counted separately.

Skia uses SDL windows, its own GPU context and SkParagraph shaping, with explicit
font files and mapped padding placeholders. Semantics around reservations require
verification before support is claimed. Official source is resolved once and
pinned, with dependency identities recorded by the external build procedure.

Every process flushes stage records. A supervisor retains partial/malformed output,
checks requested against observed identities, hashes inputs before and after runs,
and applies independent startup, operation and shutdown deadlines. GPU runs are
serial and failures do not skip another renderer. Captures are correctness evidence,
not presentation pacing. The inspection title stays outside the document capture.

## Bounded correctness continuation — approved design

Approval: owner explicitly approved C1–C6, this design and mapped tasks on
2026-11-05 ("approved").

### Qt source mapping (C1, C4)

Keep stage.py's original source commit
`43d6bba842bb5b7ab23889e49d5f138c6f22a7fb`. First retain failing geometry traces
on both exact SDKs, including source offsets, cursor-cell bounds, line membership,
visual direction, pointer positions and resolved source offsets. Use these to
separate a geometry defect from an ambiguous edge expectation; never change the
expected copied source to accommodate a failure.

Represent any preparation correction as a checked reproducible patch applied
only to the isolated stage. Fail staging on an unexpected base or patch mismatch;
record original commit, patch SHA-256 and patched output SHA-256 identities.
Interactive selection and offscreen tests consume the same prepared mapping.
No fixture-text branches or independent test-only hit-test correction are allowed.

### Skia fragment reservations (C2, C4)

Use the existing pinned SkParagraph and whole-paragraph shaping. After each
layout, derive code-fragment boundaries and the horizontal/vertical reservations
needed by those fragments; rebuild the mapped layout and repeat. Accept only a
stable boundary/reservation state within eight passes. Track prior states to
reject cycles explicitly, and report non-convergence at the pass limit.

Inserted reservations map to source boundaries without contributing copied text;
collapsed objects retain their complete UTF-16 source interval. Painting,
selection and hit testing all use the final accepted layout and its mapping.
Check horizontal padding on every fragment, vertical space, object bounds and
exact copied source in both directions. Check widths immediately below, at and
above observed wrap transitions, including adjacent spans, bidi, marks and
surrogate pairs. Reject reservations that disrupt bidi order, shaping, legal
wrapping or source mapping. Retain a minimal reproducer and blocker if this
bounded trial fails; do not add synthetic spaces, atomic code spans or another
layout engine.

### Vulkan lifecycle diagnostics (C3)

Start with the existing empty-window reproducer. Exercise default and basic
render loops separately, then explicit scene-graph release with the event loop
still running. Log callback/resource teardown order and collect externally
bounded crash backtraces on each exact SDK. Keep variants diagnostic until
observed Vulkan identity, normal shutdown and three process recreations pass.
Do not switch backend, Qt SDK or system driver to claim a lifecycle correction.
If no valid harness correction passes, keep minimum-Qt Vulkan failed and retain
an independently runnable empty-window reproducer.

### Evidence gates (C4–C6)

Run geometry before affected native fixtures. An unresolved geometry failure
blocks acceptance for that case; retain its failure rather than interpreting
replay as acceptance. Run GPU processes serially, with both Qt renderers/APIs
and each observed Qt DPR; record unavailable Skia scales explicitly. Exercise
stationary and selected captures, resize/reset, motion/replay and recreation.
Shared layout/lifecycle changes trigger the complete demonstrator matrix.

Append evidence separately from historical runs, retaining commands, source and
patch identities, binaries, fonts and before/after results. Update support and
review/migration records from actual outcomes only. Owner visual acceptance
remains separate from automated geometry, process and replay success.

## Fragment-aware upstream experiment — approved design

Owner approval received 2026-11-05 ("approved") for F1–F7 and the mapped tasks.
The following supersedes the
rejected reservation mechanism only for the proposed experiment; its historical
implementation and evidence remain intact.

### Isolation and reproducibility (F1, F6)

Use a separate staged checkout and separate build/output directory. Validate the
pinned base and clean inputs before patching; reject unexpected content and patch
application failures. Keep the original checkout, libraries and control binaries
available and hash them before/after comparison. Record patch SHA-256, patched
source hashes, DEPS revisions, system package/tool versions, GN/CMake options,
font/fixture hashes, library/executable hashes and exact commands. Build with two
workers. Keep experimental metadata/diagnostics confined to the adapter and
patched SkParagraph, with no shipping dependency or API edits.

### Source spans and candidate fit (F2, F3)

The adapter validates ordered, nonoverlapping code intervals and object intervals
against the original UTF-16 source and Unicode grapheme boundaries. Adjacent code
spans remain distinct. Map valid boundaries once to SkParagraph UTF-8 byte
indices, retaining reverse mappings and complete source intervals for collapsed
objects. Reject malformed surrogate sequences, out-of-range/reversed intervals,
partial object intersections and grapheme splits with explicit diagnostics.
Padding metadata must not introduce shaping/style boundaries into otherwise
identical text. A span boundary inside a shaped cluster that cannot receive an
edge without changing its shaping is a capability blocker, not permission to
split or reshape that cluster.

Extend the pinned `TextWrapper` candidate-fit path and `ParagraphImpl` /
`TextLine` formatting path. Reuse upstream clusters, legal break opportunities,
line bidi ordering, trailing-whitespace semantics and emergency-wrap decisions.
For each candidate endpoint, derive contiguous visual fragments by span identity
and visual adjacency after upstream bidi ordering. Charge both padding edges for
each fragment, including disjoint fragments of one logical span. A shared
fragment calculation drives fit and final formatting; font fallback alone does
not create a new fragment.

Scan a finite set of upstream candidate boundaries without assuming fragment
costs are monotonic under bidi reordering. Commit a fitting legal endpoint using
upstream break priority; if none fits, use upstream emergency boundaries while
preserving indivisible graphemes/objects. Each committed nonempty line advances
source; an oversized first indivisible unit advances once and records required
width, available width and unavoidable overflow. Handle hard breaks/empty lines
through upstream semantics. Do not rebuild whole paragraphs to converge, invent
new break opportunities or replace the upstream line-breaking engine. If these
extension points cannot express the behavior, stop and record the limitation.

### One authoritative geometry (F4)

Apply fragment translation and vertical reservations before final line geometry
is published. Use identical adjusted positions for paint, range rectangles,
carets/hit tests and placeholder/object placement. Preserve glyph IDs, fonts,
cluster membership and relative glyph advances within clusters; only fragment
translation and reserved line extents change. Invalidate layout/position caches
when span metadata changes. The empty/disabled-metadata path retains upstream
behavior and is compared with a matching unpatched control using identical
source, styles, object reservations, fonts and widths, without padding inserts.

For `objects@679`, trace unrounded/rounded baseline, ascent/descent, placeholder
rectangle, paragraph bounds and actual object paint bounds. Resolve reservation
and placement together in the enabled experiment; do not add a paint-only shift
or loosen the 0.05 logical-pixel tolerance. Keep any unchanged upstream behavior
in disabled-padding comparisons explicit rather than hiding a mismatch.

### Ordered gates and stop conditions (F5–F7)

1. Disabled-padding equivalence: compare line/source breaks, glyph IDs/fonts,
   positions, object rectangles, caret/source mappings and copied text against
   the pinned unpatched build. Require exact discrete values and record numeric
   comparison precision; do not introduce a new tolerance to mask differences.
2. Geometry: retain all 751 existing cases. Detect changed line-break signatures
   across the width scan, refine transition intervals, and add fractional widths
   below/at/above transitions with recorded probe precision. Cover adjacent spans,
   disjoint bidi fragments, combining marks, ligatures, surrogate pairs, fallback
   fonts, whitespace and objects, including oversized units and invalid ranges.
3. Consistency: assert both padding edges for every visual fragment, vertical
   reservation, object containment, both selection directions and exact copying.
   Compare shaping identities and intracluster advances with matching unpadded
   shaping, and verify hit tests against the same final geometry used for paint.
4. Native: only after the preceding gates pass, run patched Skia serially on
   OpenGL and Vulkan at actually observed DPR. Check stationary/selected captures,
   resize/reset, motion/replay and three process recreations. Preserve deadlines
   and identity checks. Explicitly list unavailable scales; do not rerun or change
   the Qt control to qualify this Skia-only experiment.
5. Review: retain representative matched before/after captures and launcher
   commands. Process/replay success cannot establish owner visual acceptance.

A failing gate retains a minimal fixture, exact command, diagnostics and identities
and blocks claims dependent on that gate. Infrastructure failures remain distinct
from missing layout capabilities. Final support records say feasible only after
all automated gates pass, otherwise blocked with the precise missing capability
or verification obstacle. Document touched upstream internals, patch size,
rebase/regression burden and dependency costs without performance qualification.
