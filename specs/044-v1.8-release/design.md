# Design — 1.8 desktop reading improvements

Status: **approved on 2026-10-14** by the user’s explicit “approved” response.
Approval covers requirements, design and T1–T5 implementation/verification.
Protected merge, tagging and publication remain separately authorized.

## Semantic selection

Extend the existing private presentation/ViewState selection machinery. Record
semantic unit offsets and lengths during source text projection, before
`projectImages` splits a paragraph into delegates. Keep these ranges stable when
resources load; never reconstruct paragraphs from delegate geometry. Retain the
current projection as the selected-text source, excluding list markers and sibling
separators from paragraph units. Table cells include an empty-cell unit; code and
literal HTML use logical newline-delimited units, including a final newline when
present. Loaded images override word granularity with their atomic alt-text range.

Use Qt text boundary finding for Unicode word and grapheme boundaries; classify
whitespace as a contiguous run and punctuation as one grapheme. Resolve a hit to
its semantic unit in ViewState. Store the initial selected unit and drag
granularity; extension includes whole endpoint units in either direction and
preserves the initial unit when reversing. Shift-click uses existing anchor logic.

## Gesture and link lifecycle

Extend MarkdownView's existing desktop pointer handlers. Count repeated presses
using platform double-click timing and distance, resetting after an unrelated
press, cancellation or document lifecycle reset. On the second/third press select
the appropriate unit immediately so a subsequent drag uses that granularity.
Exclude action hits before changing selection or reserving gestures.

Keep a pending mouse link activation until the platform double-click interval
expires. Cancel it on repeated clicks, drag, Shift-click, source replacement,
selection disablement or window deactivation. Keep touch and selection-disabled
link paths as implemented. Preserve source fallback for custom renderers.

## Host scrolling interface

Add nullable Item `selectionViewport` and real-valued
`selectionScrollRequested(deltaY)` on MarkdownView. Track the pointer in window
coordinates throughout an active selection drag. Map it to the viewport for edge
speed and back to the document for selection after host scrolling. Use elapsed
monotonic time for a 16 ms timer, cap elapsed ticks at 50 ms, and emit signed
vertical displacement at a linear speed within 32 logical pixels, capped at
600 logical pixels/second outside. Small overlapping edge zones use the nearest
edge; an exact tie produces no scrolling.

Recalculate after synchronous host handling and on subsequent ticks so a
stationary pointer follows exposed content. Guard viewport and window lifetime;
stop the timer and clear gesture state on every required cancellation path.
Never find ancestors or assign host scroll properties. The playground passes its
visible Flickable viewport and clamps contentY in its signal handler.

## Compatibility and delivery

Retain dependencies, SOVERSION 1 and SameMajorVersion compatibility. Update
package/QML versions and installed-consumer checks to 1.8.0/1.8, including future
version rejection. Finalize all bounded current-release documentation and run
`python3 scripts/check-docs.py --release`. Follow the repository
[development workflow](../../docs/development-workflow.md) for focused checks,
configured minimum/development shared/static matrices and final-head PR gates.
Tagging and publication require separate authorization.
