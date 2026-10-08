# 008: Design

**Approval:** On 2026-10-08 the user explicitly requested implementation of the supplied “Support both font units and application-based defaults” plan. This approves the complete requirements, design, and mapped tasks below before implementation. Package 0.5.0 and QML module 0.5 remain unchanged.

Capture the application font once, normalize body weight, and derive role snapshots from it. Scale with setPointSizeF for points or max(1, round(size × ratio)) for pixels. Existing setters and restoreDefaults keep resolve-mask-aware notifications and role independence.

The playground uses a small example-only FontEditor QML module to inspect native QFont units and SizeResolved masks and resize copies. QML getters synthesize both units and setters prefer explicit pixels, so direct QML inspection/conversion cannot satisfy preservation. Effective inline size uses body when the overlay has no SizeResolved bit; merely reading it is passive. A unit ComboBox converts only on user activation. A scaled integer SpinBox represents points at hundredth precision and pixels as integers. Its presentation reads the active role unit. Use the preview window Screen attached logical density × 25.4 for DPI; do not use devicePixelRatio.

Compare native Text geometry to QTextLayout at fractional points. Native Text and QTextLayout use Qt’s default font DPI; retain that shared behavior rather than introducing a different screen conversion. Match native Text’s half-point layout resolution in private font copies, retaining public point fonts and resolved code overlays. Native Text skips rounding when its initial application font compares equal to the assigned font; private layout keeps that initial precision and rounds subsequent different assignments, including a later return to the application font. Inline code without an explicit size inherits the effective layout font. Reconnect screen/DPI notifications on window changes and schedule polish. Preserve existing pixel rendering and native Qt Quick architecture.

Conversion is approximate because pixel fonts are integral and font metrics are quantized. Inline code has no unique containing block in the editor; display its body-inherited size, while actual heading code inherits the heading. No dynamic style cascade or application-font subscription is added.

Reference: [Qt logical DPI](https://doc.qt.io/qt-6/qscreen.html#logicalDotsPerInch-prop). Logical pixels are device independent; Qt handles device scaling.

Implementation evidence: [native Text font assignment](https://github.com/qt/qtdeclarative/blob/6.8/src/quick/items/qquicktext.cpp) defines the equality guard and half-point resolution. These are private rendering details, not new public size constraints.
