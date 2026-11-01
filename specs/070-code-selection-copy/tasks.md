# Mapped tasks

Approved with the supplied implementation plan on 2026-10-29.

- R1: Add a prose/code fixture with spaces, tabs, blank lines, long lines and
  Unicode; exercise forward/reverse real drags and Copy shortcuts across wrapped,
  unwrapped, numbered, highlighted and internally scrolled code. Compare offsets,
  selectedText and copyRequested. Retain paragraph/cross-block/whole-code controls.
- R2: Reproduce exhausted/non-overflowing code-edge outer requests away from outer
  edges; remove boundary forwarding. Check available internal range, outer edges,
  and stationary-pointer endpoint movement.
- R3: Run selection/code-block tests first; retain release/cancellation and wheel/
  touch coverage. Run Core runtime, QML lint, symbol privacy, installed consumers
  and syntax-enabled coverage on configured Qt 6.8.0 and 6.12.0; run doc checks.
- R4: Update selection guide/report and separate actual verification record;
  capture native evidence if available, otherwise mark it pending. Deliver one PR.
