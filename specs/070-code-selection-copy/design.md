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
