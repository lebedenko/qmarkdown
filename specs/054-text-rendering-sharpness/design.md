# Design — Text rendering sharpness

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Fix blurry text without changing layout or selection” plan. Scope covers the bounded renderer correction, regression coverage, representative captures, both Qt packaging targets and native Wayland candidate acceptance. System-package replacement and release publication are excluded.

Call `setSmooth(false)` beside `setAntialiasing(true)` in the private FormattedText constructor. Filtering an already rasterized text surface introduces extra softness; retain glyph antialiasing while removing the additional texture filtering. This single construction policy reaches all existing consumers without changing layout or replacing the scene-graph renderer.

[Qt documents smooth](https://doc.qt.io/qt-6/qquickitem.html#smooth-prop) as the texture sampling choice. No position snapping, font changes, texture-resolution changes or host scroll adjustments are part of this fix.

Use QQuickWindow::grabWindow in the styling suite to compare the same plain document with selection disabled/enabled, with matching resolved font, width, colors and placement. Sum squared horizontal and vertical luminance differences and require selectable/reference contrast >= 0.8. The existing fractional CTest invocation supplies 125% scaling. Establish failure before and success after the correction on both supported SDKs.

Capture headings, emphasis, inline code, links, tables and highlighted code at 100%, 125%, 150% and 200%, including fractional placement and scroll offsets. Inspect edges, decorations and clipping. Software captures do not replace native Wayland acceptance: inspect a viewer explicitly loading the isolated candidate at 125%, exercise selection, Find and scrolling, and revise the design if objectionable jaggedness or shimmer appears.
