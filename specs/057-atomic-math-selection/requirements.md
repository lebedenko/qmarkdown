# Requirements — Atomic math selection

Approval: the user explicitly requested implementation of the supplied atomic math selection plan on 2026-10-18. This approves the requirements, design and tasks below. Version changes, merge, tagging and publication are excluded.

R057.1: Keep eligible native fenced and standalone math rendered during selection. Find and unavailable, pending or failed rendering show selectable TeX source. Preserve host renderer precedence.

R057.2: Boundary and Shift-click use image-style endpoints. Double/triple-click select the complete body including retained newline; dragging extends in complete repeated-click units. Ordinary dragging treats formulas atomically.

R057.3: Highlight the formula rectangle on nonempty range overlap, clipped to its viewport. Copy retains exact projection substrings, whitespace and document separators. Presentation, Find and asynchronous completion preserve endpoints.

R057.4: No public API, dependencies or backend changes. Preserve scrolling, touch, wheel handoff, actions and unlinked formula behavior.
