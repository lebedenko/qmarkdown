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
