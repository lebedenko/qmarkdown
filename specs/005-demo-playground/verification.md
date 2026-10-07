# 005: verification

Planned: V1 startup, samples/reload/clear/live edits, width/height, overflow, minimum layout, pointer/keyboard/divider; V2 role changes, invalid colors, presets/reset; V3 shared/static CTest, QML lint, examples-disabled build; V4 overview/fenced captures under both presets.

Actual results (2026-10-07, Qt 6.11.2, offscreen platform):

- V1/V2: focused `qmarkdown-view-test viewer` passed with warnings treated as failures. Covers all six samples, selection/reload scroll resets, clear, live source updates, fixed-width cap and wrapping/height increases, long-source overflow, role selection preserving settings, family edits preserving unresolved inline size, explicit size override, invalid-color rejection, valid color, Alternate/reset and stable style identity. Pointer Clear/Reload, keyboard source editing and preset activation, and pointer divider drag passed. Expanded style panel leaves more than 150 px of preview height at 800×600.
- V3: shared and static examples built; each CTest suite passed 3/3 (import, parser, view). Shared/static `all_qmllint` passed; only the pre-existing unused QMarkdown import info in the import-only example remains. An examples-disabled build in `/tmp/qmarkdown-playground-library` configured/built and passed 3/3 tests; no Qt6QuickControls2 package was discovered (the supplied disable-find variable was reported unused). Viewer integration is excluded in that configuration, while library view tests still run.
- Shared/static viewer executable launch checks produced no warnings or errors during two-second offscreen smoke runs (terminated by timeout as intended).
- V4: captured and visually inspected Overview and Fenced code under Neutral/Alternate, plus expanded controls at 800×600. Artifacts: `/tmp/playground-overview.png`, `.alternate.png`, `.fenced.0.png`, `.fenced.1.png`, `.minimum.png`. Inspection found dark desktop controls behind default dark preview text; corrected with a white host preview surface and recaptured. Fixed preview widths are centered and capped; fenced literal text wraps.
- Desktop limitation: input and captures use Qt's offscreen platform, not an interactive desktop session. Actual compositor behavior and platform-specific native themes were not manually verified. Screenshot artifacts are temporary and are not repository assets.

Changed files: root/example/test CMake lists; viewer Main.qml, new Samples.qml and StylePanel.qml; tests/tst_view.cpp; README.md and specs/README.md; all four Feature 005 records. Library code, parser behavior, APIs and package version are unchanged.

