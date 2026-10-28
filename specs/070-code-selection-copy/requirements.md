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
