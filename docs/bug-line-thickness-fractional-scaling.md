# Inconsistent table-border and separator thickness at fractional scaling and scroll offsets

> Storage cleanup: generated artifacts linked below were removed from Git history.
> See the [retention policy](artifact-storage.md); historical results remain, raw-data revalidation requires regenerated inputs.

**Type:** Rendering bug / request for consistent device-pixel alignment\
**Affected version:** QMarkdown 1.18.0\
**Observed in:** md-viewer and an independent public-API QMarkdown probe\
**Status:** Reproduced with offscreen software rendering; native GPU/compositor verification pending

## Summary

Table borders, heading separators and thematic breaks can appear to have different thicknesses despite sharing the same configured logical thickness. Horizontal lines can change apparent thickness as the document scrolls.

A standalone probe reproduces this with QMarkdown’s public API. At fractional display scaling, nominally one-logical-pixel lines rasterize to different physical-pixel widths depending on their positions.

Please provide consistent rendering of axis-aligned decorative lines under fractional scaling, fractional layout coordinates and scrolling.

## User-visible behavior

- Some horizontal table borders look heavier than adjacent borders.
- Vertical borders can differ in thickness between columns.
- Heading separators and thematic breaks can exhibit similar variation.
- Moving the document changes which horizontal lines appear heavier.
- The document’s content and configured style values remain unchanged.

The supplied md-viewer screenshot shows differing coverage and brightness across table borders, consistent with this mechanism. The screenshot alone does not establish its original display scale or compositor behavior.

Vertical scrolling should primarily affect horizontal-line placement. Changes to vertical lines during vertical scrolling were not independently established; intersections, clipping and display resampling should be checked separately.

## Environment and evidence

The controlled reproduction used:

- Linux.
- Installed QMarkdown package reporting version **1.18.0**.
- Qt **6.12.0** environment.
- `QT_QPA_PLATFORM=offscreen`.
- `QT_QUICK_BACKEND=software`.
- `QT_SCALE_FACTOR=1`, `1.25` and `1.5`.
- `selectionEnabled: true`, matching the consuming reader.
- One-logical-pixel red lines against white backgrounds for unambiguous pixel measurements.

| Configuration | Observed result |
| --- | --- |
| 100% scaling | Measured lines remained one physical pixel thick in this fixture |
| 125% scaling | Equal configured thicknesses produced one- and two-physical-pixel lines |
| 150% scaling | Equal configured thicknesses produced one- and two-physical-pixel lines |
| Fractional scrolling | Changed which horizontal lines rasterized thicker |
| `Flickable.pixelAligned: true` | Did not eliminate thickness variation at fractional scaling |

Example measurements at 125%, with `pixelAligned: false`:

- At `contentY: 0`, successive sampled table borders occupied **1, 1, 1, 2** physical scanlines.
- At `contentY: 0.5`, the corresponding borders occupied **1, 1, 2, 1** physical scanlines.
- Vertical borders in one sampled row occupied **1, 2, 1, 1** physical columns.

These are raster measurements, not changes to the configured border width.

## Minimal reproduction

Use a Qt Quick host with the installed QMarkdown module:

```qml
import QtQuick
import QMarkdown 1.18

Rectangle {
    width: 900
    height: 600
    color: "white"

    property alias shift: viewport.contentY
    property alias aligned: viewport.pixelAligned

    Flickable {
        id: viewport
        anchors.fill: parent
        contentHeight: document.height + 800
        clip: true

        MarkdownView {
            id: document
            x: 24.5
            y: 24
            width: 841
            selectionEnabled: true
            gfmEnabled: true

            markdown: "## Heading\n\n---\n\n"
                + "| A | B | C |\n|---|---|---|\n"
                + "| One | Two | Three |\n".repeat(20)

            style: MarkdownStyle {
                preset: MarkdownStyle.PolishedLight
                bodyFont.pointSize: 11

                table.borderWidth: 1
                table.borderColor: "red"
                table.headerBackground: "white"
                table.bodyBackground: "white"
                table.alternateRowBackground: "white"

                headings.h2.separatorColor: "red"
                thematicBreakColor: "red"
            }
        }
    }
}
```

Run independently at each scale factor:

```sh
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
QT_SCALE_FACTOR=1.25 qml reproduction.qml
```

For each run:

1. Capture at `shift` values `0`, `0.25`, `0.5`, `0.75` and `1`.
2. Compare horizontal borders at an X coordinate away from text and vertical borders.
3. Compare vertical borders inside a row, away from horizontal intersections.
4. Repeat with `aligned: true`.
5. Repeat at scale factors `1` and `1.5`.

For automated measurement, use a Qt Quick host and capture the window after each layout/render update. The investigation used `QQuickView::grabWindow()`.

## Relevant implementation

Read-only inspection of the installed provider found:

- **`TableBlock.qml`:** borders are filled `Rectangle` items. Internal vertical positions use column division and half-line-width offsets. Row heights depend on content and font metrics.
- **`HeadingSeparator.qml`:** separator height comes from the configured logical thickness.
- **`TextBlock.qml`:** heading separators are positioned using text height plus separator spacing.
- **`BlockSequence.qml`:** thematic breaks are filled rectangles with the configured logical height.

These paths do not explicitly align the resulting decorative geometry to the final physical pixel grid.

The consuming viewer also permits fractional viewport offsets and centers the document using potentially fractional coordinates. Those are ordinary host-layout conditions that expose the issue.

## Root-cause assessment

The evidence supports a device-pixel alignment problem.

For an untransformed host, a horizontal line’s screen position depends on the document position, scroll offset and device-pixel ratio:

```text
physicalY ≈ (viewportOriginY + documentY + localLineY − contentY) × DPR
physicalThickness = logicalThickness × DPR
```

At 125% scaling, one logical pixel becomes 1.25 physical pixels. Without a consistent alignment and thickness policy, raster coverage varies by position.

This is not evidence of duplicated table borders. The inspected table implementation draws shared horizontal boundaries explicitly.

The exact contribution of native rendering and compositor resampling remains unverified.

## Requested behavior

Provide a consistent rendering policy for table borders, heading separators and thematic breaks:

- Equal configured thicknesses should produce consistent apparent weight.
- Scrolling should not make a line alternate between thinner and thicker appearances.
- Account for effective device-pixel ratio and final viewport-relative placement.
- Cover both horizontal and vertical lines.
- Preserve intentional thickness differences and zero-width disabling.
- Avoid introducing table gaps, doubled intersections or inconsistent clipping.
- Preserve text layout, selection, navigation and hit-test coordinates.
- Support ordinary host-owned scrolling through public APIs.

An upstream solution should ideally preserve smooth fractional scrolling without requiring the host to enable `Flickable.pixelAligned`.

If host cooperation is unavoidable, document the limitation and expose the minimum necessary public contract. Do not require consumers to inspect renderer internals.

## Why viewport alignment alone is insufficient

Qt documents `Flickable.pixelAligned` as aligning content offsets to pixels and recommends it for thin-line content. Its default is `false`.

However, enabling it did not solve the reproduction at fractional scaling. Integer logical offsets can still correspond to fractional physical offsets, while line widths and local layout positions remain fractional.

Likewise, snapping only document-local geometry may be insufficient if a later viewport translation moves that geometry off the physical grid.

The exact upstream implementation is open; the requirement is consistent final rendering, not a prescribed snapping algorithm.

Reference: [Qt Flickable pixelAligned documentation](https://doc.qt.io/qt-6/qml-qtquick-flickable.html#pixelAligned-prop).

## Acceptance and regression coverage

- Exercise DPRs **1, 1.25, 1.5 and 2**.
- Exercise integer and fractional scroll offsets, with viewport alignment both enabled and disabled.
- Cover odd document widths, centered fractional origins and three-column tables.
- Include wrapped cells and mixed row heights.
- Cover outer borders, internal grid lines, heading separators and thematic breaks.
- Check disabled lines and intentional fractional/non-default thicknesses.
- Verify stable thickness, continuity and intersections using image measurements.
- Check selection and navigation geometry remains correct.
- Run native scrolling and scaling review separately from software/offscreen checks.
- Confirm acceptable rendering performance during continuous scrolling.

Do not require one logical pixel to equal one physical pixel at every scale. Define a consistent rendering policy and verify that equivalent lines retain consistent weight.

## Investigation artifacts and limitations

Temporary artifacts prepared during investigation:

- `/tmp/mdv-lines-probe.qml`
- `/tmp/mdv-lines-probe.cpp`
- `/tmp/mdv-lines-probe`
- `/tmp/mdv-lines-*.png`
- Original screenshot: `/tmp/codex-clipboard-61fwEA.png`

These temporary paths are local evidence, not durable report attachments. Preserve the QML/C++ probe, original screenshot and representative 125%/150% captures alongside the report when saving it.

No application or provider source files were changed. No native GPU/compositor acceptance, upstream patch, hosted CI or performance verification was performed.

## Preserved attachments

The original investigation artifacts are preserved in [../specs/071-pixel-aligned-lines/evidence/](../specs/071-pixel-aligned-lines/evidence/):

- [QML probe](../specs/071-pixel-aligned-lines/evidence/mdv-lines-probe.qml) and [C++ capture host](../specs/071-pixel-aligned-lines/evidence/mdv-lines-probe.cpp).
- Original md-viewer screenshot (removed artifact: `../specs/071-pixel-aligned-lines/evidence/codex-clipboard-61fwEA.png`).
- Representative 125% captures: offset 0 (removed artifact: `../specs/071-pixel-aligned-lines/evidence/mdv-lines-1.25-0-0.png`), offset 0.5 (removed artifact: `../specs/071-pixel-aligned-lines/evidence/mdv-lines-1.25-0-0.5.png`), pixel-aligned offset 0.5 (removed artifact: `../specs/071-pixel-aligned-lines/evidence/mdv-lines-1.25-1-0.5.png`).
- Representative 150% captures: offset 0 (removed artifact: `../specs/071-pixel-aligned-lines/evidence/mdv-lines-1.5-0-0.png`), offset 0.5 (removed artifact: `../specs/071-pixel-aligned-lines/evidence/mdv-lines-1.5-0-0.5.png`), pixel-aligned offset 0.5 (removed artifact: `../specs/071-pixel-aligned-lines/evidence/mdv-lines-1.5-1-0.5.png`).

The repository keeps the probe sources, the original screenshot and the representative captures above. The full set of 30 captures and the environment-specific probe executable stay with the original report outside the repository. Capture filenames encode `DPR-pixelAligned-scrollOffset`; `0`/`1` denotes disabled/enabled pixel alignment. The C++ host retains its original absolute `/tmp` input/output paths; restore the QML probe there or adjust those paths before rerunning it.
