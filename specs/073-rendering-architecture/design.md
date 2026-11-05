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
