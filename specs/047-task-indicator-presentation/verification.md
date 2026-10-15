# Actual verification — 2026-10-15

The approved plan is recorded in requirements/design/tasks. Results below are actual checks against this working-tree change, not release-publication evidence.

## Focused and host checks

- Host Qt 6.12.0 build and QML lint passed. Lint retains the existing import-only example’s unused-import informational message; no new warnings.
- Task layout and custom lifecycle regressions passed: recognized/ordinary, ordered/nested, empty and multiline tasks, width 1/0/negative, live font/color/style replacement, custom dimensions including nonpositive/NaN/infinity, destruction, non-Item and creation-error fallback, synchronous component replacement, unchanged source/model/selection. Existing task-image and viewer tests were updated for indicators being instantiated only on task rows.
- Pointer, touch, Space and accessibility toggle attempts preserve task state and source. Accessible hosts report read-only checkbox state, no focus, no accessible children and no action interface; their accessible parent membership and global geometry are verified. Existing link and document selection regressions pass.
- Nine viewer presentation tests passed (Fusion/Basic/Material × 1/1.25/2 scales). Each tests light and dark palettes, compares the detached checked indicator image pixel-for-pixel with an ordinary active-style CheckBox indicator, and checks inert input/accessibility behavior. All 18 captures were visually inspected, including unchecked indicators and the library fallback. Natural style sizes, first-line alignment, transparent backgrounds and rounded fallback checkmarks were accepted.
- Host captures: `build-shared/tests/task-presentation/{Fusion,Basic,Material}/{1,1.25,2}/{light,dark}.png`. Contact sheet inspected at `/tmp/task-contact-accessible.png`.
- `python3 scripts/check-docs.py`, release-document regression tests and `git diff --check` passed. Development documentation validation now checks the published version’s release notes/changelog, avoiding fabricated 1.10 release claims; explicit release validation remains strict.
- Working-tree source archive built and its extracted library configured, built and installed with host Qt 6.12.0: `build-archive-047-accessible/`. New private host and painted type are included in the source allowlist and installed QML inventory.
- `readelf -d build-shared/src/QMarkdown/libQMarkdown.so` shows no Controls dependency. Library sources have no Controls imports or link targets; viewer/presentation tests own Controls.

## Configured SDK matrix

Qt synchronization selected the already configured development SDK 6.12.0 without changing toolchain metadata. Final runs passed after the action-free accessibility fix:

- `build-ci-047-minimum-accessible/` — Qt 6.8.0: passed, exit 0; 27/27 CTest entries in both shared and static builds.
- `build-ci-047-development-accessible/` — Qt 6.12.0: passed, exit 0; 27/27 CTest entries in both shared and static builds.

Both runners passed shared/static CTest, QML lint, symbol privacy, relocated installed consumers, historical imports/package requests, exact 1.10.0 acceptance, new indicator consumer, library-only builds and benchmark smoke checks.

## Issues found and resolved during verification

- Loader implicit dimensions are read-only; layout now reads explicit preferred dimensions from its loaded item.
- A writable host `checked` property allowed Qt accessibility to toggle the visual state. It is now read-only and derived from parsed task state.
- Synchronous host replacement during a binding evaluation caused binding loops. Creation is deferred while invalidation/destruction remains immediate, preserving the creation token guard.
- Old image-task assertions indexed a hidden indicator on ordinary rows; they now assert that no indicator exists.
- The initial visual fixture’s unqualified Material import selected Material for ordinary controls in every style. A qualified attached-style import and exact image comparisons now verify the intended active style. Fixture palette self-bindings were replaced with independent values.
- Initial Docker access was sandbox-blocked; authorized Docker runs proceeded. Earlier matrix attempts retained at `build-ci-047-minimum-verified/` and `build-ci-047-development/` failed on the now-corrected task-image assertion and development-document fixtures; they are not passing evidence.
- The first installed API consumer checked geometry before deferred artwork creation; it now waits for the requested dimensions. Its focused run against the extracted host archive passed. Earlier `*-final/` matrix runs stopped at this test.
- All 18 minimum-Qt shared captures were additionally inspected (`/tmp/task-contact-minimum.png`); supported styles match ordinary Controls checkboxes at all scales. Minimum-Qt lint retains the repository’s unresolved native-type metadata diagnostic category (also emitted for the new painted indicator); host/development lint resolves these types.

- A final direct accessibility probe found Qt Quick advertised inert Toggle/Press actions despite read-only state. A private host type now supplies an action-free public-Qt accessibility interface. Focused task and all nine visual tests passed again, including new assertions for absent action interface, accessible parent membership and nonempty global geometry. Full SDK reruns are retained at `build-ci-047-minimum-accessible/` and `build-ci-047-development-accessible/`.

## Completion

T1–T5 are complete. Runtime, tests, packaging, API/task/viewer documentation and development metadata form one working-tree change. No release was published and no merge was performed. Final records are `result.json`, `ci.log`, and `packaging/` under each completed matrix directory; capture files are under each shared/static test directory.
