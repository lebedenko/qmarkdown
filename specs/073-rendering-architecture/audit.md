# Coordinate audit and placement blocker

## Current pipeline

| Stage | Coordinates and ownership | Risk / evidence |
| --- | --- | --- |
| Inline preparation | UTF-16 ranges remapped to layout text and object placeholders | Source, grapheme and glyph indices are distinct; preserve mappings. |
| QTextLayout | Logical units; shaped fallback glyph runs, per-line baselines | `formattedtext.cpp` builds formats before shaping; line widths reserve padding/object advances. |
| Padded layout | Visual-order cursor cells, per-cell shifts, extra ascent/descent | `cacheLine` applies shifts to copied QGlyphRuns. These shifts are absent from the QTextLayout consumed by public text nodes. |
| Decorations/interaction | Code rectangles, cursor cells, links and objects share translated geometry | Replacing only glyph drawing cannot change these independently. |
| Raster surface | Ink bounds expand one logical pixel, round outward in physical units | Retained atom-free path uses local origin. Other paths include `mapToScene()` and repaint on placement changes. |
| Allocation | Expand to a common integral logical/physical grid, up to 120 | Prevents painted-item allocation/extent quantization from changing the intended scale. |
| Sampling | Retained path `setSmooth(true)`; fallback nearest | Moving a retained raster between physical pixels changes sampled ink. Exact transport equality does not prove acceptable strokes. |
| Scene/display | Parent transforms, clip, window DPR, compositor scale | Requesting a Qt scale is not observing final DPR. Capture is not presentation feedback. |

Source: [formattedtext.cpp](../../src/QMarkdown/private/formattedtext.cpp).
The shipped scene-graph surface can consume the authoritative layout only when
its eligibility conditions hold. Its public `addTextLayout` call has no input
for the separate padded glyph positions.

## Public API gate

Both installed Qt headers expose `addTextLayout` and `addTextDocument`; neither
exposes an arbitrary QGlyphRun insertion method on QSGTextNode. QTextLayout offers
line placement and value-returning glyphRuns, with no public setter for its shaped
runs. The probe independently reserves two eight-unit code gutters and forty units
after an object placeholder in copied glyph positions. Original and requested
raster controls differ, while rebuilding the text node from the original layout
replays exactly. No transparent text or selection-emission technique is used.

This rejects **direct reuse of existing prepared padded runs through QSGTextNode**.
It does not reject curve or distance-field quality, and does not establish that
all possible public Qt layout compositions are impossible. Splitting layouts at
format boundaries requires its own bidi, contextual shaping, wrapping and cursor
proof. A native Skia glyph-run adapter is another unqualified candidate.

The public contract agrees with the [Qt QSGTextNode documentation](https://doc.qt.io/qt-6/qsgtextnode.html).
Skia offers positioned glyph drawing through its [public API](https://docs.skia.org/docs/user/api/),
but a native Qt/Skia integration and shaping policy still need implementation and
measurement; API availability alone is not selection evidence.

## Historical defects

[Feature 065's artifact report](../065-padded-code-rendering/blur-probe/artifact-verification.md)
retains the overlapping scene, actual-source/proxy comparisons and the stalled
Qt 6.8 Vulkan process. A 146-unit unclipped paragraph in a 120-unit row allocation
explains that scene's overlap, not the owner's candidate-only stroke complaint.
Those complaints remain unclassified. This new probe's successful initialization
does not resolve the historical executable's stall or qualify its replay.
