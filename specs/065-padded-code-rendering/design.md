# Design

Approved scope: the owner-supplied Iteration 065 plan, explicitly requested on
2026-10-22; see [requirements](requirements.md).

The existing FormattedText layout remains authoritative. Prototype a single
additional draw-only layout with identical fonts, text and breaks, coalescing
source intervals by padding translation and foreground. Emit intervals using
public QSGTextNode selection foreground, making surrounding text transparent.
Preserve prepared underline, strikeout and gutter connectors. Retain the layer
order: code backgrounds, Find, selection, glyphs, focus.

Only after every prototype gate passes, integrate persistent glyph nodes and
small retained rounded-background textures using public image nodes. Invalidate
only affected layers; geometry/style/DPR govern texture replacement. Math and
footnote atoms keep the existing raster path, while already eligible native text
keeps its current path.

The standalone prototype first tests transparent surrounding text at a distinct
source-format boundary, using italic DejaVu Sans `fX`. Compare normal foreground
emission of `f` against selection foreground emission of `f` with both source
formats transparent. Both use the same text/font/breaks and source properties.
Format changes require re-preparing the layout between captures; this minimal
correctness probe makes no persistence or performance claim.

A failure of any required case stops integration. Do not replace the proposed
method with private glyph APIs or bitmap paragraph caching. A new implementation
strategy requires revised documents and renewed approval.

## Retained texture experiment (supersedes selection strategy)

Renewed approval: owner's 2026-10-22 implementation request. A diagnostic-only
patch exposes a snapshot of the prepared runs and invokes preparation; it is
applied only to a fresh staged source tree. No substring shaping is permitted.
Each occurrence uses its original raw font, index and translated position.
Raster tiles use outward-rounded physical ink bounds plus transparent margins;
the original position relative to the tile preserves subpixel phase. Cache
identity includes QRawFont equality, glyph, foreground, DPR and phase.

First compare single-glyph image composition against the prepared glyph-run
painter for italic fX at identical phase. Count occurrences and inspect lost,
extra and changed ink. This preparation-level gate precedes GPU/lifecycle/cost
expansion; failure makes further architecture work unnecessary. Passing requires
render-thread-owned public image nodes/textures, explicit decorations and layer
ordering, resource invalidation and the full requirements matrix. Linear
filtering is fixed for later fractional-transform captures; no shimmer claim
follows from a same-phase comparison. Historical probe files remain unchanged.

## Diagnostic revision (2026-10-23 approval)

A separate executable in retained-prototype shares only the staged snapshot patch.
Each case prepares text once. The reference paints whole runs; direct controls
reconstruct a glyph or copy its run before replacing indexes/positions. Tile controls
compare the historical zero-position formula with unchanged positions plus an integer
physical translation in the painter transform. A full-canvas single-glyph image is
cropped at integer physical bounds and copied without scaling using a DPR-1 compositor.
All paths preserve emission order and turn off decorations. JSON Lines retain every
case and glyph's metadata; PNGs retain historical examples and first differences.
Comparison selection controls reporting only, so failures never interrupt the matrix.
Exact premultiplied pixels remain the gate. Classification depends on paired controls,
not visual impressions. No GPU, scrolling, lifecycle, shaping breadth or performance
claim follows from this diagnostic.

## Whole-run revision — approved 2026-10-23

The owner's supplied implementation request approves W1–W7 and the tasks below.
Reuse PR #49's diagnostics without reopening it; its delivery is closed/unmerged,
not completed integration. Historical results remain evidence of earlier probes.

A separate standalone executable uses the same exported-tree snapshot patch.
Union raw-font ink rectangles at original glyph positions plus run/canvas origin,
scale to physical space, round outward and add four physical pixels. Translate
the painter by the negative integer tile origin divided by DPR and draw the
unchanged complete run. Reset image DPR to one for integer physical composition.
Do not rebuild or split runs. A full-canvas image per run supplies the crop
control and detects support outside bounds. All three paths preserve order.
Log each run and glyph sequence, fonts and geometry; count emissions explicitly.
Individual-glyph painting remains a non-accepting diagnostic. Compare exact
premultiplied pixels, keep transparent differences and representative captures.

Load explicit font files before layout and hash their bytes in each environment.
Historical binaries retain environment font resolution. Run historical, basic
phase and extended prepared-run stages in order, finishing each matrix before
stopping on failure. Both SDKs must pass the basic stage before either extends.
No later rendering architecture is selected by this experiment.

## Retained run-texture scene-graph revision — approved 2026-10-23

The owner's implementation request approves S1–S9 and mapped tasks. Add a separate
standalone executable; retain earlier probes unchanged. GUI-thread preparation
uses actual prepared runs and the exact-font/four-pixel whole-run tile algorithm.
An immutable generation contains DPR, canvas size, run ordinals/physical bounds
and candidate/crop/full images; no raw fonts or layout objects reach the render
thread. A custom QQuickItem synchronizes snapshots in updatePaintNode; each
generation creates public QSGImageNodes with owned QSGTextures on that thread.
Images carry DPR 1; node rectangles carry physical size/position divided by DPR.
Fix linear/clamp sampling without atlasing/mipmaps. Nodes survive parent movement.

Capture candidate, crop and full-per-run paths separately in the same window,
comparing exact premultiplied capture pixels. The full-canvas reference retains
individual run images and order, not a precomposited paragraph image. Record
fresh QPainter current-phase differences only as diagnostics. Stable frame
synchronization must precede grabs; failures retain counts and captures and never
relax thresholds. Native backend/device/DPR must be observed, not inferred from
requested environment settings.

Static fixtures/matrices are the existing 1,218 cases split by observed window
DPR, over white and #202020. Forward/reverse controls run in fresh processes.
Scroll fixtures: plain fXfX/left, single code/left, adjacent code/left, colour
boundary/left, wrapping/centre and tiny width/right. Physical translation trace
is row-major Cartesian {0,.25,.5,.75} squared, then (1,1),(-1,-1),(0,0), repeated
twice; convert offsets to logical units with actual DPR. Test full viewport and
logical clip (64,32,96,96). Resource counters/identity snapshots bracket warm-up,
each trace, and three window recreation cycles; they describe application work
and ownership only. Scene graph nodes own textures; teardown occurs on their
render thread. No broader lifecycle claim follows.

Progression: preparation on both SDKs → complete static target matrix → complete
scroll matrix → recreation smoke. Missing native hardware is unverified, not a
passing fallback. Finish all cases/orders of a started stage on mismatch, then
stop before the next stage. Publication is one new stacked continuation based
on PR #51, keeping implementation/specs/evidence in the same change.

## Production retained paragraph design — approved 2026-10-24

The owner's supplied implementation request approves P1–P6. Keep Qt ownership of
one existing painted surface per paragraph. For padded atom-free text and the
existing supported common logical/physical DPR grid, round local ink bounds
outward without scene translation. Stable position/size means ancestor observers
schedule no paints. Set smooth filtering only on this retained path. Existing
update() calls still repaint content, decorations, selection, Find and focus;
window/DPR lifecycle continues through existing handling. Native eligibility,
atom rendering and unsupported-grid fallback remain unchanged. Fractional motion
transforms cached pixels; fresh-painter phase equality is no longer required.
