# 060 — Actual verification

Implementation and verification in progress. Planned checks are in tasks.md; no planned result is claimed as passing.

## Compatibility finding requiring a decision

Qt 6.12 documents that revisions are not supported for grouped-property members:
https://doc.qt.io/qt-6.12/qtqml-cppintegration-definetypes.html

A runtime probe confirms `MarkdownStyle { table.borderMode: ... }` remains
accessible under import 1.13 despite REVISION(1,14). Named/uncreatable registration
and extra type revisions did not change this behavior; those experiments were
removed. The user explicitly approved preserving the API and documenting this framework
limitation on 2026-10-19. No strict runtime gating success is claimed.

## Host results (Qt 6.12.0)

- Host synchronization selected the already-configured 6.12.0 target; no toolchain file change was needed.
- Shared/static builds succeed. Focused shared import, view/playground, styling (scale 1 and 1.25), selection/copy (scale 1 and 1.25), and syntax-view checks pass 7/7 (73.28 s).
- Complete static CTest passes 32/32 (108.87 s), including the nine existing Controls-style/scale task-presentation cases.
- Backend-enabled build succeeds; syntax and actual-playground syntax-view tests pass 2/2 (9.12 s). Backend OFF is exercised by shared/static suites.
- QML lint succeeds; existing unused-import advice remains. Documentation/local-link validation and whitespace checks pass.
- Style tests cover all presets, equal assignments, inherited/live alternate colors, explicit transparency, individual resets, preserved overrides and reset-all.
- Rendering tests cover the four modes with/without alternation, header-only, single-column wrapping and quoted nested tables, per-table restart, invalid metrics and narrow widths. Scale 1/1.25 runs compare painted translucent intersections and inner/outer segments, and verify stable visible-mode insets.
- Playground checks exercise off-at-startup table decoration, visible-mode activation, host border color, alternation, reset-all, existing sample persistence and compact layout. Existing link, Find, resource and accessibility regressions are retained; selection/copy is additionally checked after live table mode/background changes and resizing.
- The initial shared suite passed 31/32; its only failure was the intentionally strict old-import rejection probe. The accepted Qt limitation is now explicitly expected in that probe, while revision metadata and 1.14 imports are asserted normally.

## Exact-container attempts

Fresh minimum (6.8.0) and development (6.12.0) attempts, including unrestricted Docker access retries, both stopped before verification: image preparation exits 125 because the installed Docker CLI rejects `build --progress`. Evidence: `build-ci/6.8.0/20261019T131352Z-6dfdfemf` and `build-ci/6.12.0/20261019T131440Z-g7b166m7`; logs `/tmp/table-ci-min-escalated.log` and `/tmp/table-ci-dev-escalated.log`. Successful final-head hosted packaging checks remain required.

Physical mouse/touchpad acceptance is unconfirmed in this headless session. Automated input and painted-output checks do not establish physical device acceptance.
