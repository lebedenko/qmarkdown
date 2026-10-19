# 060 — Actual verification

Implementation is complete. Planned checks are in tasks.md; actual completed checks are recorded below. Final hosted gates and the remaining local installed-static packaging run are tracked in the combined PR checks and description.

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

## Final implementation checks

- Local packaging script's final shared build, complete CTest (32/32, 115.94 s), lint, package version requests, relocation and all shared installed-consumer variants passed. The script advanced to the static build; its remaining results will be recorded in the PR description alongside final-head hosted results, without changing the verified Git tree.
- Additional one-pixel Grid bounds checks with a 1000-pixel requested border pass at scales 1 and 1.25, alongside style contracts and painted intersection checks (4/4 QtTest results at each scale).
- Implementation code and documentation retain d11635a as an ancestor; no history rewrite, merge, tag or publication was performed.

Evidence: `/tmp/table-approved-focused.log`, `/tmp/table-approved-static-suite.log`,
`/tmp/table-packaging-final.log`, `/tmp/table-final-highlight-tests.log`,
`/tmp/table-final-lint.log`, `/tmp/table-metrics1.log`, `/tmp/table-metrics125.log`.
These local logs are ordinary execution evidence, not release artifacts. Required
hosted checks are `packaging (minimum)` and `packaging (development)` on the PR's
final head; an in-progress check is not a success claim.

## Release preparation checks — 2026-10-19

- Confirmed PR #39 merged at d75d42c4f0529c26c5bfc4dbb7169af373017da8 and remote main unchanged at that commit before branching release/1.14.0.
- Selected /usr/bin/qmake6 reports 6.12.0; explicit qt_versions.py --sync-host completed with no configuration change (minimum 6.8.0, development 6.12.0).
- Ordinary documentation validation, release documentation validation and git diff --check passed after final documentation edits.
- Changes are documentation only: release notes/changelog, current summaries, API/style/task/math/scrolling and installation guides, plus these existing feature records. Package and QML versions remain 1.14.0 / 1.14; runtime and dependency inputs are unchanged.
- Full final-head hosted packaging results and retained release-gate artifact will be recorded in the release PR before merge. Publication and downloaded-hash results belong to the release workflow; no success is claimed here in advance.
