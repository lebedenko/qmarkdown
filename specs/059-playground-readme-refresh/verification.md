# 059 — Actual verification

Implementation is on local `chore/playground-readme-refresh`; old local main is preserved as `archive/main-before-playground-refresh-0b22f44`. No push, PR, merge or release is authorized or performed. Versions and library defaults are unchanged.

Host Qt is 6.12.0; host KF6SyntaxHighlighting is 6.31.0. Shared/static builds and the separate backend-enabled build are exercised locally.

Both configured container gates were attempted (Qt 6.8.0 and 6.12.0). Both stopped before verification: Docker image preparation exits 125 because `docker build` rejects `--progress`. No exact-container packaging success is claimed. Evidence is retained in build-ci/6.8.0/20261019T120643Z-_vcfza7y and build-ci/6.12.0/20261019T120656Z-lu9lfp66.

Physical mouse/touchpad and native display checks are unavailable in this headless session. Automated input coverage is distinct from physical device verification.

## Results

- Final shared and static builds succeed; complete CTest runs pass 32/32 each (112.56 s shared, 107.94 s static). These include the final compact layout and current test code. Relevant view/syntax checks were also rerun after test changes.
- Backend OFF availability messaging, disabled syntax controls and source loading pass. Backend ON syntax tests pass 2/2, including the actual playground sample, nonempty token ranges, different Light/Dark ranges and magenta painted keyword pixels. The separate highlighted viewer builds successfully. The ordinary demo task is unchanged.
- Host KF6SyntaxHighlighting 6.31.0 emits `QObject::installEventFilter(): Cannot filter events for objects in a different thread.` on worker initialization in the playground syntax check. This is visible in the syntax-view test output; it does not prevent highlighting or painted-color verification. No library change or dependency replacement was made.
- Actual playground checks pass for Fusion/Basic/Material × scales 1/1.25/2 (nine combinations): normal/2× host indicator dimensions, fallback replacement, selection stability, light/dark host themes, all 19 keyed samples, reload/clear/edit, width controls and 800×600 splitter dragging. The original larger toolbar failed compact Basic/Material checks; moving optional controls into the bounded panel resolved them.
- Existing code-block tests exercise angle/pixel input, both edges, reversal, phased input, horizontal input, Shift-wheel, short blocks and wrapping. The new sample loads with tall/wide and non-overflow fences plus surrounding document content. An exploratory direct-sample synthetic probe passed phased edge handoff, but did not observe angle-wheel parent motion at the upper edge; that probe was not retained as a deterministic test. Direct-sample upper-edge angle-wheel handoff and physical mouse/touchpad behavior remain unconfirmed; the passing library regressions do not establish those manual results.
- QML lint target succeeds (existing advisory warnings remain); documentation validation and `git diff --check` pass.

Commands/evidence: `/tmp/059-final-shared.log`, `/tmp/059-final-static.log`, `/tmp/059-final-highlight.log`, `/tmp/059-final-lint.log`, and `/tmp/059-final-playground-{Fusion,Basic,Material}-{1,1.25,2}.log`. These local logs are not release artifacts.

## Follow-up: anchored artwork clipping

The user's screenshots exposed clipping of the 2× host artwork. The active host style anchors its indicator to its parent's vertical center. Scaling that indicator directly enlarged its parent while keeping a centering offset (observed local bounds: 32×32 artwork at y=8 inside a 32×32 adapter), clipping the bottom eight logical pixels.

The playground adapter now reparents artwork into a natural-size wrapper and scales the wrapper. This preserves style-owned anchors and transforms while reporting the scaled dimensions to the library. No library defaults or sibling style files changed. Added a deterministic vertically anchored fixture with bounds and rendered corner-pixel checks at 1× → 2× → 1×, plus an actual-playground bounds assertion.

Follow-up verification on host Qt 6.12.0:

- The new anchored regression fails with the prior implementation: a 32×32 indicator occupies `(0,8)`–`(32,40)` inside a 32×32 adapter. Restoring the wrapper fix makes it pass, including rendered corner-pixel assertions. Evidence: `/tmp/indicator-regression-before.log`, `/tmp/indicator-regression-after.log`.
- Shared and static builds succeed. Relevant CTest runs pass 10/10 each: the view suite and Fusion/Basic/Material task-presentation checks at scale 1/1.25/2, including the new anchored-artwork regression. Evidence: `/tmp/indicator-fix-tests.log`, `/tmp/indicator-fix-static-tests.log`.
- The actual playground task test also passes with the active host Controls style at scales 1/1.25/2 (including full scaled-artwork bounds), rather than only the explicit built-in styles.
- QML lint, documentation validation and whitespace checks pass. Previously recorded unrelated packaging and physical-input limits remain unchanged.

## Follow-up: shared demo startup defaults

The user explicitly requested and clarified that `task demo` and `task demo-highlighted` share all listed startup options. Updated Main.qml and StylePanel.qml initial values only; Neutral/Reset still restores the previous host-based presentation. Library defaults, package versions, build-task behavior and the sibling viewer are unchanged. Syntax controls start checked but remain disabled when the build lacks the backend.

Actual startup checks verify selection, math, all GFM, host task artwork, replacement markers, line numbers, code limits, PolishedDark, H1 separator, link underlining and both syntax choices. Both backend-OFF and backend-ON cases pass. Existing option tests explicitly disable aggregate GFM when testing individual toggles, and typography/reset tests establish their Neutral baseline.

Host Qt 6.12.0 verification: shared view/syntax suites 3/3 pass (18.81 s); static view/syntax suites 3/3 pass (13.55 s); backend-enabled syntax suites 2/2 pass (9.00 s), including painted colors and palette changes. QML lint, docs and whitespace checks pass. Logs: `/tmp/defaults-tests.log`, `/tmp/defaults-static-tests.log`, `/tmp/defaults-highlight-tests.log`, `/tmp/defaults-lint.log`. Prior unrelated container and physical-input limitations remain as recorded.
