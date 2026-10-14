# Requirements — 1.8 desktop reading improvements

Status: **approved on 2026-10-14** by the user’s explicit “approved” response.
Approval covers requirements, design and T1–T5 implementation/verification.
Protected merge, tagging and publication remain separately authorized.

## Requirements

- R1: Deliver package 1.8.0 / QML 1.8 with existing custom fenced-block renderers
  and the implemented release documentation gate, plus the desktop selection
  improvements below. Preserve documented 1.x compatibility and dependencies.
- R2: Under `selectionEnabled`, double-click selects a Qt Unicode word,
  contiguous whitespace run or single punctuation grapheme. Loaded images are
  atomic. Triple-click selects a complete prose paragraph or heading, clicked
  table cell, or logical code/literal-HTML line including its terminating newline.
  Prose includes image alt text but excludes synthetic list prefixes and sibling
  separators. Preserve semantic paragraph identity across resource projection.
- R3: Repeated-click dragging extends by words or the corresponding triple-click
  units. Shift-click preserves existing anchor extension. Use platform click
  timing and distance thresholds. Delay selection-enabled desktop mouse link
  activation until the double-click interval expires; repeated clicks, dragging
  and Shift-click suppress it. Touch and selection-disabled links keep existing
  behavior. Exclude actions; hosts own clipboard writes. Custom renderers remain
  replaced by selectable source when selection is enabled.
- R4: Expose nullable Item `MarkdownView.selectionViewport` and
  `selectionScrollRequested(real deltaY)`. During desktop selection dragging,
  request scrolling within 32 logical pixels of either vertical viewport edge,
  increasing linearly to 600 logical pixels/second and retaining maximum speed
  outside. Use a 16 ms timer, elapsed-time deltas capped at 50 ms per tick, and
  positive downward deltas. Without a viewport, emit no requests.
- R5: Hosts scroll and clamp; the library neither modifies Flickables nor
  discovers ancestors. Recalculate selection from the stationary window-space
  pointer after scrolling. Stop requests on release, cancellation, window
  deactivation, disabled selection, viewport removal/destruction or source
  replacement. Preserve existing selection lifecycle. Integrate the playground.
- R6: Extend behavioral and installed-consumer coverage, retain existing
  selection, renderer, resource-policy and conformance regressions, and run
  focused checks followed by relevant shared/static Qt verification and required
  final-head PR gates. Record actual results separately from planned checks.
- R7: Finalize release notes, changelog, availability and installation guidance
  together and pass the strict release documentation gate. Preserve historical
  records. No tagging or publication is authorized by this specification.

## Deferred scope

Horizontal autoscrolling, streaming, bundled highlighting/Mermaid/math, touch
handles, caret navigation, additional copy formats, richer syntax/images/styles
and broader platforms remain deferred. Integration and evidence belong here.
