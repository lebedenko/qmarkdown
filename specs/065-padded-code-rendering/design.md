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
