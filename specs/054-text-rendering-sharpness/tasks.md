# Tasks — Text rendering sharpness

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Fix blurry text without changing layout or selection” plan. Scope covers the bounded renderer correction, regression coverage, representative captures, both Qt packaging targets and native Wayland candidate acceptance. System-package replacement and release publication are excluded.

- [x] T1 / R054.1–2: Disable texture smoothing, preserve antialiasing and existing geometry; review the bounded diff.
- [x] T2 / R054.1,3: Add rendered contrast regression and establish before/after results on Qt 6.8.0 and 6.12.0.
- [x] T3 / R054.2–3: Capture and inspect representative content at four scales, fractional placement and scrolling; run styling, selection, navigation and code-block checks.
- [x] T4 / R054.3: Run minimum/development shared/static packaging matrix with syntax backend OFF/ON.
- [x] T5 / R054.2–3: Install isolated candidate and verify repository Markdown in a native Wayland viewer at 125%, including selection, Find, scrolling and visual acceptance.
- [x] T6 / R054.3: Record actual verification separately and retain all changes as one reviewable unit.
