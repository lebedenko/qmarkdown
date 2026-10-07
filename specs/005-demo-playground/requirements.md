# 005: demo playground requirements

Approved on 2026-10-07 by the user’s explicit instruction to implement the supplied playground plan. Approval covers these requirements, design, tasks, and verification scope.

- R1: Independently scrollable live source/preview panes and draggable divider; retain editor/preview object names and executable path.
- R2: Overview (initial), headings, inline formatting/escapes/entities, fenced code, Unicode/long lines, and unsupported samples; selection/reload reset both scroll positions; Clear empties source. Memory only.
- R3: Collapsible standard Controls style panel: body/H1–H6/inline/fenced roles, family, 8–72 px, validated color, 0–48 spacing, Neutral/Alternate/default reset. One host-owned style; inline size inherits until overridden.
- R4: Fit/240/480/720 preview widths capped and centered; no horizontal scrolling; usable at 800×600.
- R5: Explain literal links/images/HTML and unfinished CommonMark support. No library API/parser/version/rendering changes. Controls optional with examples disabled.
