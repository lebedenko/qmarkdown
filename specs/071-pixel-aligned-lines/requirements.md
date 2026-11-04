# 071 — Pixel-aligned decorative lines

Approved on 2026-11-04 by the owner request to implement ("Approved, go with nearest
rounding"). Approval covers requirements, design, tasks, nearest-device-pixel
thickness rounding with a one-pixel minimum, implementation, verification and one
reviewable PR. Merge and publication require separate authorization.

Input: the [original report](../../docs/bug-line-thickness-fractional-scaling.md)
against QMarkdown 1.18.0. The report is design input, not a commitment.

## Reported versus established

Reported: equal configured table borders, heading separators and thematic breaks
rasterize at 1 or 2 physical pixels at 125%/150% scaling, and horizontal lines
change weight while scrolling; `Flickable.pixelAligned` does not help.
Established by the report: offscreen software rendering only. Native GPU and
compositor behavior is unverified. Vertical-line behavior during vertical
scrolling is not independently established.

## Requirements

R1. Axis-aligned decorative lines (table outer and internal borders, heading
separators, thematic breaks) with equal configured thickness render with equal
physical thickness at device pixel ratios 1, 1.25, 1.5 and 2, at integer and
fractional layout origins and scroll offsets, with `Flickable.pixelAligned` on or off.

R2. Horizontal and vertical lines are both covered. Adjacent lines and
intersections show no gaps, doubled coverage or inconsistent clipping.

R3. Zero or negative thickness still disables a line. A positive thickness never
disappears; intentionally different thicknesses stay distinguishable where the
device grid allows.

R4. Layout, text placement, selection, navigation and hit-test coordinates are
unchanged. Only painted geometry is snapped.

R5. Scrolling remains smooth and host-owned through public APIs. No host
cooperation, new public API, style property, dependency or version change is required.

R6. Continuous-scroll rendering cost remains acceptable and is measured.

R7. Preserve the report and probe as evidence. Native GPU/compositor acceptance is
reported separately from software/offscreen checks and stays pending until captured.

## Non-goals

One logical pixel need not equal one physical pixel. Rotated or non-uniformly
scaled hosts only need to remain correct, not snapped. Border styles, dashes,
radii and cell-background redesign are out of scope.
