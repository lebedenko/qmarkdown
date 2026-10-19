# 058 — Design

Approved on 2026-10-19 through the user-supplied implementation plan. Approval covers requirements, design, mapped tasks, implementation, verification and one PR; no merge or release.

Use blocking acceptance only for actual internal movement, retaining the native viewport filter. Moving events are consumed completely; subsequent outward events propagate.

Implementation finding: setting QML WheelHandler.blocking to true accepts its event point before onWheel runs. Rejecting the QML WheelEvent leaves that point accepted on host Qt 6.12.0 and prevents boundary handoff. The bounded fix uses a private SourceWheelArea QQuickItem with public Qt wheel delivery, preserving the existing delta/clamping policy and setting both event and point acceptance from actual movement. No Qt private development dependency is introduced. This is an implementation correction within R1's approved behavior, replacing the proposed literal WheelHandler toggle.

Expose ordered identity as a private model role. Bind the public option through recursive sequences. Measure indicator lane width, replacing only unordered task markers. Clamp insets to width minus one and clip artwork. Reserve the first text block band and append task clearance. Presentation does not modify semantic models or accessibility hosts.

The sibling viewer's parent observer also defaults to blocking. A provider-independent
fixture reproduces its suppression of native angle-wheel scrolling on Qt 6.12.0.
The corrected observer fixture explicitly sets blocking false, alongside rejecting
the event. The sibling source remains unchanged as required; its boundary scrolling
needs that host correction. This limitation is recorded separately from provider checks.

When an internally consumed ScrollBegin carries deltas, notify enclosing native
items of a zero-movement begin so their gesture lifecycle can later accept handoff.
Consumed deltas never preload the outer drag threshold. Unconsumed phased input
therefore follows the host's native threshold rather than forwarding inner motion.
