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

## Smoothing-off motion diagnostic — approved 2026-11-04

The owner-approved supplied plan extends feature 065 only (B1–B5). Export HEAD
into two fresh source trees. Add identical private counter access in both; change
only `RasterTextSurface::place()`'s `setSmooth(retained)` to `setSmooth(false)` in
the candidate. The constructor and atom/unsupported-scale behavior already use
false; do not modify placement, painter, layout, interaction or shipping files.
Record the commit and renderer hashes. Preserve the existing stationary harness.

A separate executable owns a real QML Flickable and links one staged library.
Stationary offsets are physical fractions divided by observed DPR. A separately
painted retained image inherits exactly the surface position, size and DPR, with
nearest sampling. A second clipped viewport begins exactly 800 logical pixels away (an integer
physical phase at every qualified DPR), permitting simultaneous framebuffer
comparison during live motion. It never calls `update()` on the measured surface. Freeze paint counters
only after constructing/warming this independent control. Movement checks retain
layout signals, all character cells, surface pointer, position and allocation.
Capture timing is explicitly not display frame rate. Mutation gates run afterward.

Native preview uses the same source pixels through switchable nearest/linear
sampling controls within one viewport; qualify this proxy against the isolated
baseline/candidate stationary outputs before accepting it as a source comparison.
Labels stay outside the clipped document. Actual source variants remain isolated
for all correctness/performance measurements. No perceptual acceptance is inferred
from automated equality or contrast metrics. Results and gaps are recorded in
`blur-probe/motion-verification.md` separately from this design.

## Native-only diagnostic design — approved 2026-11-04

The supplied plan authorizes N1–N5. Keep previous rejection evidence immutable.
The stager pins `43d6bba842bb5b7ab23889e49d5f138c6f22a7fb`, records its own
hash and policy identity, and adds identical opt-in first-paint instrumentation to
both isolated renderers. Candidate policy consults the exposed window's observed
graphics API. Construct measured text after exposure/backend initialization.

Calibration precedes qualification: independently paint a transparent image with
the real raster device's format/size/DPR and painter transform, clip and hints;
retain actual and independent images. The authoritative control is a separate
QQuickPaintedItem holding those independent pixels, matching source geometry,
antialiasing and sampling. Opacity switching holds viewport coordinates fixed.
Raw/control mismatches are calibration failures, never geometry regressions.
Instrumentation is disabled outside calibration, especially performance runs.

A failed calibration stops the later matrix, pose replay, performance and native
review delivery. Record those stages as not reached, retain failing captures and
never qualify the old unconditional candidate or narrow this candidate to OpenGL.

The revised motion harness captures the source alone during live motion. After
settling, a static container under the same clipped viewport replaces the
Flickable content parent, using each captured actual content-item position.
Every replay compares source/live first, then opacity-switches the independent
painted control at that same pose. Counter, surface and interaction assertions
follow each pose. Replay failures and control-calibration failures have separate
record types/guards. The translated direct-node control remains a stationary
non-authoritative diagnostic observation.

`qualify_native.py` sequences fresh focused calibration on both exact SDKs,
then full Vulkan/OpenGL/software correctness and finally five new alternating
Release pairs for every target/backend. It stops at the first failing stage.
Interrupted/incomplete comparisons are missing evidence, not geometry mismatch.
Software preservation compares matching stationary source captures from both
variants. Calibration/raw capture instrumentation is disabled in performance mode.

Post-gate diagnostic refinements keep the existing timing boundaries: performance
asserts expected smoothing before warming and after every timed frame; assertions
sit outside the timers. Raw capture directories are unique per fixture. The
separate review executable validates native comparison evidence and proxy pixels,
labels a performance rejection outside the document, and supports both exact SDKs.
Visual review cannot replace the performance gate. See the
[actual native-only outcome](blur-probe/native-only-verification.md) for the frozen
attempt, missing raw artifact, post-gate smoke scope and stages not reached.

## Actual-source diagnosis design — approved 2026-11-05

Use shared fixture definitions/configuration in motion and review. Build review
separately against each immutable native-only-v2 staged source. Label the linked
variant actual source; proxy remains explicitly unqualified. Preserve initialization
at y=40 before moving each paragraph to 40+i*120, clipping and child order.
Record timestamps, requested/actual content positions, paragraph transforms,
visibility, draw order and clip bounds. Freeze within the same Flickable hierarchy;
stop attribution on replay failure. Save source/proxy/difference viewport images.
After reproduction, replay both exact SDKs, OpenGL/Vulkan and both alignment modes.
Require exact whole viewport parity at failing poses, all sixteen Cartesian phases,
entry/exit, origin and complete trace; zero warmed paints/layout rebuilds and stable
surfaces/interaction geometry. Keep binaries/library/renderer/harness hashes.
Missing screenshot or replay evidence is an explicit limitation, not a pass.
