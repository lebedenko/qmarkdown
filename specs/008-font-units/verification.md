# 008: Verification

## Planned

Controlled application-font tests in px and fractional pt, restoring fonts after each test; heading proportions, fixed-family code and inline inheritance in body/headings; reset snapshots, independent roles, notifications, shared styles/replacement, runtime px↔pt, wrapping/contentHeight. Playground conversion at logical DPI, fractional input, passive inline inheritance, presets/reset and font field preservation. Native/private point geometry and layout invalidation. Installed point consumer plus existing pixels; focused tests, full suite, QML lint and relocated shared/static packaging. Scaling and mixed screens when available.

## Actual

Executed on 2026-10-08 with Qt 6.12.0 on Linux:

- T1 / R1–R3: controlled 13 px and 10.25 pt application-font tests pass; all six heading ratios, body/code normal weight, fixed family, independent code/headings, family-only inline inheritance in body/headings, notification deduplication, reset snapshots and new construction after application-font changes. Each test restores the original application font in cleanup; historical pixel regressions use a controlled 16 px application font.
- T2 / R4: playground tests pass for active-unit display, logical-DPI conversion both ways, fractional SpinBox parsing/editing, preservation of underline/letter spacing and resolve masks, passive inline selection/same-unit selection/family edits, explicit inline size overrides, Alternate pixel sizes and construction-based Neutral/Reset.
- T3 / R5: nativePointLayout passes for 13.125/13.25/13.49 pt, checking wrapped heights, line widths, public font precision, edits and a return to the application font. Runtime tests pass for shared views, same-unit QML fractional edits, Qt pixel-wins semantics, whole-font unit switches, content-height updates, style replacement/reset/destruction, and synthetic screen/DPI notifications invalidating cached layouts.
- Focused command: `QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH=build-shared/qml build-shared/tests/qmarkdown-view-test nativePointLayout applicationTypography pointRenderingAndRuntimeUnits viewerFontUnits` — 9 passed (including init/cleanup), no failures/skips. The same cases pass with `QT_SCALE_FACTOR=2`.
- `ctest --test-dir build-shared --output-on-failure` — 3/3 passed (import, parser, viewer).
- `cmake --build build-shared --target all_qmllint` — passed; only the established import-only unused-import informational message.
- `python3 scripts/verify-packaging.py` — passed after the final rendering correction: fresh shared/static builds, full CTest suites and lint, private cmark symbol checks, installation/relocation metadata and notices checks, shared linked and plugin-only consumers, and static consumers. Installed-consumer checks exercise point sizing and runtime height changes alongside existing pixels. Final log: `/tmp/qmarkdown-font-packaging-final.log`; artifacts: `/tmp/qmarkdown-packaging-h70_xzxy`.
- Physical display: `hyprctl monitors -j` reports one eDP-1 screen at 2560×1600, scale 1.25. Focused cases pass on Wayland with `QT_IM_MODULE=compose` (9 passed). The first run with the default input module failed because QtWayland emitted its disableSurface/focused-surface warning and tests reject warnings; this prevented asynchronous assertions from settling. The compose module removes that platform warning without changing renderer code.
- `git diff --check` — passed. No package/import version changes or dependency additions.

Limits: only one physical screen is connected, so migration between screens with different DPI/scales was not physically verified. Screen-change and logical-DPI-change signal invalidation is automated; offscreen 2× and physical Wayland 1.25× cover available scaling. Qt 6.8 compatibility was not executed; the available Qt version is 6.12.0.
