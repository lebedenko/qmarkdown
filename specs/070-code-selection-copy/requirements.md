# 070 — Code selection and copy

Approved on 2026-10-29 through the user request to implement the supplied plan.
Approval covers requirements, design, mapped tasks, implementation, verification
and one reviewable PR. Merge and publication require separate authorization.

## Requirements

R1. Partial code selection and copy preserve exact rendered source whitespace,
newlines and Unicode grapheme boundaries in both directions.
R2. Desktop nested selection scrolls internally first. Exhausted or non-overflowing
code edges request outer movement only in the outer viewport vertical edge zone.
R3. Preserve speed, timer, cancellation, stationary-pointer endpoint updates,
wheel/touch handoff, public APIs, host clipboard ownership, versions and dependencies.
R4. Preserve the [original report](../../docs/bug-code-block-partial-selection-copy.md),
distinguish reported symptoms from reproduced defects, and leave native local and
SSH/tmux acceptance pending until actual payload/paste captures exist.

## Native diagnostic acceptance extension

Approved on 2026-11-02 by the owner request to implement the supplied native
acceptance plan. Scope: isolated installed shared builds of b22d519 and e399594,
temporary md-viewer instrumentation through public APIs, byte capture tooling,
owner-run native scrolling/copy comparisons and evidence in this feature. No
shipping API, clipboard, version, dependency, system installation, merge or release
changes. Native gestures and affected SSH/tmux paste require owner participation.

R5. Correlate selection offsets/text, emitted text, clipboard writer/readback and
local/remote paste by action ID; preserve exact UTF-8 bytes and transport framing.
R6. Validate tools and both app builds; native acceptance requires literal expected
substrings and actual captures, including a minimized original if needed.
