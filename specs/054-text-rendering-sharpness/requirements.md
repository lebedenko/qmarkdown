# Requirements — Text rendering sharpness

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Fix blurry text without changing layout or selection” plan. Scope covers the bounded renderer correction, regression coverage, representative captures, both Qt packaging targets and native Wayland candidate acceptance. System-package replacement and release publication are excluded.

- **R054.1:** Remove the demonstrated additional softness from every FormattedText path (selection, Find, formatted paragraphs, headings, tables and code), retaining glyph antialiasing and ordinary Qt Quick text as the crispness reference.
- **R054.2:** Preserve fonts, shaping, wrapping, padding, decoration geometry, selection coordinates, link hit testing, item placement, texture resolution and host scrolling. Keep interfaces, dependencies and minimum Qt unchanged.
- **R054.3:** Deliver implementation and rendered-output regression together; verify both supported Qt versions, representative scales and the actual Wayland backend using an isolated candidate install. Pixel-identical Dolphin rendering is not required.

The user clarified that any repository Markdown document may be used for native viewer acceptance; there is no specific original document to recover.
