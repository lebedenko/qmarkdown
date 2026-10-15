# Task-indicator presentation requirements

Approved on 2026-10-15 by the user instruction to implement the supplied plan. Approval covers the stated requirements, design, tasks and verification; release publication is excluded.

- R1: Add nullable `MarkdownView.taskIndicator` in development package 1.10.0 / QML 1.10; preserve historical imports. Custom Item roots require live `checked` and `markdownStyle` inputs and supply preferred dimensions.
- R2: Library owns state, geometry, a single read-only accessible checkbox and inert interaction. Trusted host artwork must not introduce focus, pointer handlers, actions or accessibility nodes. Appearance changes preserve source and selection.
- R3: Default artwork is an antialiased transparent rounded outline and continuous rounded checkmark, body-colored, sized max(16, round(body line height × 0.85)). Keep markers, nesting, eight-pixel gap, first-line centering, positive-width content reservation and clipping.
- R4: Host replacement destroys instances; errors and non-Item roots diagnose and fall back. Guard synchronous replacement during creation.
- R5: Viewer uses the active Controls CheckBox indicator, enabled but decorative, with a default-on host-style toggle. Controls remains a host-only dependency.
- R6: Verify lifecycle, layout, input/accessibility, selection, visuals across Fusion/Basic/Material, light/dark, 1×/1.25×/2×, lint, shared/static, installed consumers and configured SDK packaging.

This supersedes only feature 035’s indicator presentation contract; its historical approval and parser behavior remain intact. Task editing, global theme selection, general theming and release publication are excluded.
