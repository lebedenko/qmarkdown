# Design

Approved with the supplied implementation plan on 2026-10-29.

Remove vertical code-boundary delta forwarding in `ViewState::scrollSelection`.
Retain bounded internal X/Y movement. A successful internal Y movement suppresses
outer requests for that tick; otherwise use only the existing outer-edge speed.
Continue extending the endpoint from the stationary scene-space pointer each tick.
Wheel and touch paths are independent and unchanged.

Trace projection registration, clipped/scrolled coordinates and cursor geometry
with real mouse drags and Ctrl+C. Expected payloads are independently specified
literal substrings. Change projection or geometry only for demonstrated defects;
never trim, escape or sanitize copied text. Tests use source offsets only to place
mouse coordinates, and compare public offsets/text and emitted payloads.

Native acceptance compares escaped text and UTF-8 bytes from emitted payload,
clipboard, local paste and SSH/tmux paste, with fixture, gestures, Qt/backend,
terminal, SSH and tmux versions. Offscreen tests cannot establish native acceptance.

## Native diagnostic acceptance extension

Approved on 2026-11-02 by the owner request to implement the supplied native
acceptance plan. Scope: isolated installed shared builds of b22d519 and e399594,
temporary md-viewer instrumentation through public APIs, byte capture tooling,
owner-run native scrolling/copy comparisons and evidence in this feature. No
shipping API, clipboard, version, dependency, system installation, merge or release
changes. Native gestures and affected SSH/tmux paste require owner participation.

Use the same host Qt, math ON and highlighting ON in both installations. Snapshot
the current md-viewer working tree without altering it; record revision, dirty
status and snapshot hashes. Instrument emitted signals before existing forwarding
and the host clipboard writer before/after setText. JSONL records contain action
IDs, escaped text, UTF-8 length/hex/SHA-256. Raw terminal capture disables newline
conversion and brackets paste explicitly; retain wire bytes separately. See the
[runbook](diagnostics/README.md) for the matrix and acceptance criteria.
