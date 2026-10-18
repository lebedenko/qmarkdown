# Verification — Atomic math selection

## Planned checks

See [tasks](tasks.md). Planned checks are not successful results. Required pinned Qt 6.8.0/6.12.0 shared/static and math ON/OFF matrices remain the PR delivery gate.

## Actual results — 2026-10-18

- Host Qt is 6.12.0; `qt_versions.py --sync-host /usr/bin/qmake6` confirmed the configured development target without changing it.
- Shared focused math, selection (100%/125%), reading, custom renderer and code-block checks passed **6/6** (`/tmp/qmarkdown-atomic-relevant.log`). QML lint passed (`/tmp/qmarkdown-atomic-final-build.log`).
- Static math and selection (100%/125%) checks passed **3/3** (`/tmp/qmarkdown-atomic-static.log`).
- Backend-OFF shared math and selection checks passed **3/3** after the final registry-membership refinement (`/tmp/qmarkdown-atomic-off-final.log`).
- Installed shared math and selection consumers and the static math consumer built and ran successfully (`/tmp/qmarkdown-atomic-consumer.log`, `-selection-consumer.log`, `-static-consumer.log`).
- New regressions cover fenced/standalone/nested multiline bodies, both repeated-click granularities, boundary and reverse dragging, exact partial body copy across Find closure/asynchronous rendering, resize/style changes, whole-formula overlap highlight, centering/viewport clipping, horizontal scrolling, Select All/clear/replacement, and registered host precedence including an explicit undefined entry. Existing selection regressions retain keyboard/actions, text/image and lifetime coverage.
- Visually inspected offscreen 100% and 125% captures of centered and overflow highlights. The rectangle covers the formula and clips at the viewport; nested list/quote decorations remain outside it. Captures are local `/tmp/qmarkdown-atomic-highlight*.png`, not release assets. Native desktop acceptance remains outstanding.
- Broad host suite passed **30/32** (`/tmp/qmarkdown-atomic-suite.log`). Both styling variants failed `StylingTest::sharpnessPresentationFixtures` at `tst_styling.cpp:1024`: `view->height() > flick->height()`. This fixture does not use math; the failure remains unresolved and is not claimed as a full-suite pass. Packaging verification is subject to this same gate.
- The pinned minimum runner failed before building: Docker lacks Buildx and rejects `docker build --progress=plain` with `unknown flag: --progress`. Retrying outside the sandbox produced the same error (`/tmp/qmarkdown-atomic-ci-minimum-retry.log`). The full pinned matrix has not passed locally; use the required PR packaging jobs.
- Documentation/local links and `git diff --check` passed. No version, release summary, merge, tag or publication changes.

## Corrected fixture attempts

The initial math regression searched QObject ownership for a Loader-created text leaf; it now traverses the visual tree. The nested Select All expectation was corrected to retain the existing list bullet projection. Both corrected fixtures pass in the focused runs above.
