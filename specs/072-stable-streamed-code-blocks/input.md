# Supplied implementation plan

The user supplied the following plan on 2026-11-04 and explicitly requested implementation and verification in a fresh context.

## Stable streamed code blocks

### Summary

Fix scroll resets and highlighting flicker during code-block appends. As selected, height-limited code blocks will automatically follow appended code while the reader is at the bottom. Scrolling upward pauses following; returning to the bottom resumes it.

Outer document scrolling remains host-owned. No new public property, dependency, or version change is required.

### Implementation

#### Preserve scroll state and follow appended code

- Distinguish retained streaming publications from source replacement using private publication context. Carry this classification through deferred layout restoration; do not infer it from `streamingEnabled` alone.
- For a retained code block whose text grows by suffix, preserve the first visible source anchor and horizontal offset instead of resetting both axes.
- Before applying growth, capture whether the viewport is within one logical pixel of its old bottom. If so, follow the new bottom after layout settles, including the first transition into overflow.
- Otherwise restore the reader’s source anchor. Preserve horizontal position, clamped to the new bounds; wrapping continues to reset horizontal scrolling.
- Keep follow intent separate from temporary geometry changes. Programmatic restoration and highlighting completion must not count as reader movement. Wheel, touch, scrollbar, selection scrolling, and Find navigation must update the position used for subsequent appends.
- Cancel stale deferred work on replacement, delegate destruction, or presentation reset. Preserve existing reset behavior for unrelated text and non-streaming replacement.
- Apply following only to the built-in fenced-code viewport. Custom renderers and math retain their existing behavior.

#### Stabilize highlighting

- Select the richer text component from highlighting configuration, rather than whether asynchronous ranges happen to be nonempty. Keep that component alive across pending, successful, empty, and failed results.
- During eligible streaming suffix growth, retain the last accepted highlighting spans over their unchanged source prefix. Display newly appended text with the normal code color until current results arrive.
- Replace retained spans atomically when the latest request completes, including an empty result. Preserve cancellation, generation checks, provider fallback, and resource limits.
- Clear retained highlighting on source replacement, language/provider changes, disabling highlighting, or input becoming ineligible. Keep this retention policy private and limited to streamed code updates.

#### Completion and compatibility

- Fence closure preserves the final scroll position; later appends outside that block cannot move its viewport.
- `flushMarkdown()` publishes pending growth with the same following rules. It remains a publication operation, not an end-of-stream signal.
- Disabling streaming flushes pending growth, then leaves ordinary scrolling active.
- Initial rendering of an already-long block remains at the top. Following begins on subsequent eligible growth when the reader is at the bottom.

### Regression coverage

Add focused cases to the code-block, syntax-request, syntax-view, and streaming suites:

- Repeated appends with controlled delayed highlighting; assert stable text-item identity and retained prefix colors between results.
- Empty results, provider failure, out-of-order completion, cancellation, and replacement while work is pending.
- Overflow onset, continued bottom-following, manual scroll-up preservation, and resumption at bottom.
- Horizontal-position preservation, wrapping, line numbers, selection, Find, and height changes during updates.
- Fence closure, appends after closure, explicit flush, disabling streaming with pending input, and clear/shorten/unrelated replacement.
- Both source ownership paths: `appendMarkdown()` and a growing `markdown` binding.
- Assert that internal following does not request outer document navigation.

Run focused tests first, then Core verification with syntax-enabled coverage on configured Qt 6.8.0 and 6.12.0, including runtime tests, QML lint, symbol privacy, and installed consumers. Capture a rendered streaming sequence; report native visual acceptance separately from offscreen results.

### Specifications and delivery

- Create specification **072 — Stable streamed code blocks**, preserving the supplied report as input and keeping requirements, design, mapped tasks, and actual verification separate.
- Update the streaming and code-block guides to document the internal-following exception and flush semantics.
- Keep the consumer viewport-height configuration fix outside this change.
- Deliver implementation, tests, documentation, and verification in one reviewable PR. Record approval before implementation; merge, publication, and system-package replacement require separate authorization.
