# Design — document-wide selection and copy

Status: approved on 2026-10-14 by the user response "approved". Scope: [requirements](requirements.md).

## Projection and lifecycle (R1, R2, R6)

Add a private projection helper alongside `private/document.*`. Walk parsed source
blocks before `projectImages`, producing one immutable QString plus block text
ranges. Append separators by counting existing trailing line feeds. Containers
retain descendant ranges; list prefixes and structural separators belong to the
projection even when they have no text leaf. Continuation content retains native
block order; nested list indentation depends on list depth, not quote depth.
Tables serialize each row's normalized cells, including empty ones. Code and HTML
use their parser text; formatting and filtered HTML metadata do not rewrite it.

Carry ranges through resource splitting in `resourcecontroller.cpp`, including
text-segment local offsets and image alt ranges. Append roles in BlockModel without
renumbering existing roles; an enabled-rendering role selects native text leaves
while retaining the original disabled delegates. Reset the model on enablement
toggles so DelegateChooser recreates the appropriate delegates; propagate them through recursive cell models.

Keep a private per-view coordinator owned by ViewState: projection, anchor and
active UTF-16 endpoints, normalized public endpoints and registered selectable
leaves. Public QML properties/methods forward to this coordinator; Copy forwards
the exact selected substring through the public signal. Clear resets to offset
zero. Select All sets anchor zero and active projection length. Compare effective
ParseOptions before clearing for option changes, including aggregate GFM masking.
Resource model resets unregister leaves and cancel pending link taps, as delegate
destruction previously did, while retaining projection and selection.

## Native rendering and geometry (R5, R6)

Extend `FormattedText` with private selection endpoints/color, cursor hit testing
and cached visual cursor cells. Reuse [QTextLayout valid cursor boundaries](https://doc.qt.io/qt-6/qtextlayout.html#isValidCursorPosition) and the
existing padded glyph translations rather than reshaping fragments. Preserve
source offsets for bidi cells; use visual distances for hits and merge selected
cell rectangles per line where contiguous. Clamp hits to grapheme boundaries.
For an overflowing unpadded grapheme, query cursor cells at natural line width,
then restore the original width and translate cells by the alignment difference.
This retains original layout and accurate hits at one-pixel allocations.
Paint translucent selection over backgrounds and behind unchanged foregrounds;
selection/color changes repaint without relayout. Zero-padding and padded paths
share source-indexed geometry. Retain terminal-newline offsets although the
display removes its existing final line feed.

When enabled, route plain paragraph/heading/code/HTML leaves through FormattedText
with their existing fonts, alignment and minimum empty heights. Keep disabled
Text delegates. Register image leaves as atomic ranges and paint their highlight.
Add the style color using existing preset/override/reset machinery and tests.

## Pointer and focus routing (R3, R4)

Use mouse-only Qt Quick pointer handlers coordinated per view. Establish anchor
on press, retain it for Shift extension, and capture dragging after Qt's drag
threshold. Cancel pending link taps when selection captures or Shift is used.
A mouse-only exclusive tap handler reserves document presses after the passive
observer excludes actions; this prevents Qt 6.8 Flickable's item grab from filtering
selection moves. The drag handler takes over after the platform threshold and
retains its grab. No host viewport properties are changed. Gesture flags live in
an internal QtObject; document endpoints remain in ViewState.
A mouse-only PointHandler resets action-exclusion state on release, including
after a canceled tap, so the next document drag can start normally. Preserve the
distinction between absent links and valid empty destinations.
Hit-test registered leaves in scene coordinates; table containment selects the
actual cell before nearest-boundary fallback. Resolve gap ties by document order.
For atomic image ranges use the nearest edge; crossing the image selects its alt
range. Never consume gestures originating in host action delegate subtrees.

Acquire view focus for document mouse interaction. Handle standard platform key
sequences only with view active focus; do not clear on focus loss. Do not discover
viewports or scroll automatically. Mouse-only handlers leave touch scrolling and
wheel delivery to the host. Recreated leaves read retained coordinator offsets.

## Integration and trade-offs (R7, R8)

Update MarkdownView.qml, private QML metadata, MarkdownStyle, viewer controls and
API documentation together. Bump package/QML to 1.7.0/1.7 only after approval;
extend installed consumer and compatibility fixtures while retaining older 1.x.
Follow [delivery workflow](../../docs/development-workflow.md).

A private projection decouples copy text from resource/layout churn. Extending
the existing renderer avoids a second shaping engine but requires geometry tests
for both padded and unpadded paths. Synthetic markers/separators have offsets
without glyph cells; nearest leaf boundaries and Select All expose them without
adding a second interactive marker renderer.
