# Stable streamed code blocks

Approved on 2026-11-04 through the user-supplied implementation plan and explicit request to implement it. Approval covers requirements, design, mapped tasks, implementation and verification. Merge, publication and system-package replacement require separate authorization.

## Requirements

R1. Retained streaming suffix updates preserve the built-in fenced-code reading position and horizontal offset. Height-limited blocks follow growth within one logical pixel of the old bottom, including overflow onset; scrolling up pauses and returning to bottom resumes following.
R2. Highlighting configuration selects a stable rich text component. Eligible streamed growth retains accepted prefix spans until latest results atomically replace them, including empty results. Replacement and configuration changes clear retained spans; cancellation, fallback and limits remain enforced.
R3. Initial long blocks start at the top. Closure and later outside appends preserve position. Flush publishes without ending the stream; disabling streaming flushes pending growth. Custom renderers, math and host-owned document scrolling retain their contracts.
R4. No public API, dependency or version change. Consumer viewport-height configuration is excluded.

The supplied report is preserved in [input](input.md).
