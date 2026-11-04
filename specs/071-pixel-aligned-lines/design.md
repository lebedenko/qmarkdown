# Design

Approved on 2026-11-04 by the owner request to implement ("Approved, go with nearest
rounding"). Approval covers requirements, design, tasks, nearest-device-pixel
thickness rounding with a one-pixel minimum, implementation, verification and one
reviewable PR. Merge and publication require separate authorization.

## Cause

Lines are `Rectangle` items with logical thickness and fractional positions
(content-derived row heights, `index * width / columns - width / 2`, text-height
based separator offsets). The final physical position also depends on the host
origin, `contentY` and DPR, which no QML binding can observe, because
`mapToItem` is not reactive to ancestor movement. The snap must therefore happen
at scene-graph update time.

## Private `PixelLineItem`

A private C++ `QQuickItem` (registered in `QMarkdown.Private` beside the existing
scene-graph items) draws one filled rectangle through the scene graph.

- Properties: `color`. Geometry comes from the ordinary `x/y/width/height`, so QML
  layout code, including the table's logical geometry, is unchanged.
- `updatePaintNode` uses `mapToScene()` and the window's
  `effectiveDevicePixelRatio`. When the transform is translation-plus-positive-scale,
  it maps the item rect to device space and snaps it, then converts back to item
  coordinates for a `QSGRectangleNode`.
- Snapping: on the thin axis (smaller device extent) the start edge is rounded and
  the thickness is `max(1, round(extent))` device pixels, so equal logical thickness
  is equal at every position (snapping both edges independently would yield 1 or 2
  pixels again). On the long axis both edges are rounded, keeping lines flush with
  cells. Private `startInset` and `endThickness` metrics make vertical starts follow
  the snapped top border and horizontal ends include the snapped right border.
  This shares intersection edges without overlapping translucent paint or changing
  logical item geometry. Position rounding uses the same small half-pixel tolerance
  as thickness rounding to absorb transform precision at shared boundaries.
  Extents of zero or less and transparent lines return the same empty rectangle
  during revalidation and painting (R3), so they do not schedule idle frames.
- GPU backends use `QSGRectangleNode`. Software rendering uses a bounded
  `QSGRenderNode` with the public painter resource, without claiming an opaque
  logical-pixel region. This avoids software occlusion artifacts when one snapped
  device pixel is thinner than one logical pixel.
- Under rotation, skew, or no window, it paints the unsnapped rect.
- Scrolling: Qt's item-change listener API is private, so a small per-window tracker
  (a child `QObject` of the `QQuickWindow`) holds the visible lines and, on
  `QQuickWindow::afterAnimating`, recomputes each line's snapped device rect and calls
  `update()` only when it changed. Verification must show this repaints on Flickable
  `contentY` changes within the same frame; any lag is a defect. Cost is measured (R6).
- No textures, no `QQuickPaintedItem`, no document-level HTML or `QTextDocument`.

## Thickness policy (approved: nearest)

Round device extent to the nearest pixel with a one-pixel minimum. Equal
thicknesses are therefore equal at every position. At DPR 1.25, 1 logical px is 1
device px (nominally thin); round-up would give 2 px, visibly heavy.

## Call sites

- `TableBlock.qml`: horizontal, bottom and vertical lines become `PixelLineItem`.
  Cell fills stay `Rectangle`. A row-layout Column and a shared border overlay
  keep every border above every cell background, so the next row cannot cover a
  snapped edge crossing the logical row boundary. Logical row geometry is preserved.
- `HeadingSeparator.qml`: base type changes from `Rectangle` to `PixelLineItem`,
  keeping `objectName`, `height`, `color` and decoration metrics.
- `BlockSequence.qml`: thematic break delegate.

Existing `objectName`s (`tableHorizontalLine`, `tableBottomLine`,
`tableVerticalLine`, `headingSeparator`) are retained so current tests and probes
keep working. Check whether existing tests read `color`, `height` or `width` on these
objects and keep those properties intact.

## Risks

Uniform scroll-triggered updates on many lines in long tables; measured in R6.
Window-transform notification coverage for nested transformed ancestors. Software
backend differs from GPU in sub-pixel coverage; hence native review stays separate.
