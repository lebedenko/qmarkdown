# Verification — Optional source-line gutter

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Publish QMarkdown 1.11, then implement 1.12 code-block usability” plan. Scope includes Features 050–053 and one package 1.12.0 / QML 1.12 release PR. Merge, tagging and publication of 1.12 require separate authorization.

## Actual results — 2026-10-16

- QMarkdown 1.11.0 publication completed before this branch was created. [Publication run 38906093482](https://github.com/lebedenko/qmarkdown/actions/runs/38906093482) passed eligibility, both extracted-source Qt jobs, Arch lifecycle checks and publication. Publisher output: `PASS: published v1.11.0; all four downloaded SHA-256 values match`.
- Host SDK synchronization selected Qt 6.12.0 without changing configured targets.
- Local shared backend-ON build passed. `ctest --test-dir build-shared --output-on-failure`: **31/31 passed**, 93.54 seconds.
- Focused coverage: `lineGeometry` across 1/9/10/99/100 lines, wrap and highlighting modes, native baseline and color reset checks.
- QML lint target passed on the initial implementation. The final dynamic Loader interface is explicitly represented as a variant; lint will be checked again on the final inputs.
- Strict release documentation and local links passed using `python3 scripts/check-docs.py --release`; `git diff --check` passed.
- Earlier failing attempts exposed and corrected component ownership, missing-required diagnostics, source geometry, icon resource resolution and selection/panning priority. Their logs remain in local build directories; they are not release assets.

## Delivery verification status

The retained Qt 6.8.0 and Qt 6.12.0 candidate matrices passed shared/static builds, highlighting OFF/ON, tests, lint, historical imports, relocated consumers and benchmark smoke (`build-ci-112-minimum-verified`, `build-ci-112-development-verified`). Earlier `*-complete` runs stopped on missing-required diagnostics/empty-block geometry; earlier `*-final` runs stopped on the installed-consumer fixture. Those failures were corrected and preserved.

The subsequent typography and phased-wheel review has focused coverage, including a passing wheel/trackpad boundary test. Final-input full verification and refreshed distribution checks are pending. Final-head PR packaging checks and the retained release gate remain required; no merge or 1.12 publication is authorized.

## Distribution checks on implementation commit b46a7c6

- Extracted source builds/install passed on Qt 6.8.0 and Qt 6.12.0 with highlighting OFF and ON (`build-source-112-6.8.0`, `build-source-112-6.12.0`).
- Arch metadata, payload, namcap, consumers and pacman lifecycle passed (`build-arch-112`).
- A later focused review tightened proportional-number width, effective line-height caps and wheel boundary propagation. The final source archive and distribution checks will be refreshed for those runtime changes.
- The relocated consumer fixture now constructs its code style using a `MarkdownStyle` object; its focused relocated-package check passed (`build-consumer-112`).

## Final implementation inputs — commit 089dc30

- Final host shared/backend-ON build passed; the complete CTest suite passed **31/31**, 92.17 seconds (`/tmp/full-112-final-inputs.log`). New code-block coverage includes ordinary wheel and phased touchpad boundary handoff, plain Text anchor preservation, native Find reveal/manual-scroll stability, tiny viewport caps, source-line baselines, both scrollbar axes and invalid-component fallback.
- `all_qmllint` passed with no new warnings. The import-only example retains its existing unused-import informational message (`/tmp/lint-112-final-inputs.log`).
- Deterministic source archive: `build-dist-112-reviewed/qmarkdown-1.12.0.tar.gz`, built from commit 089dc30. All four extracted source build/install checks passed: Qt 6.8.0 and 6.12.0, highlighting OFF and ON (`build-source-112-reviewed-6.8.0`, `build-source-112-reviewed-6.12.0`).
- Arch package/lifecycle verification of that archive passed metadata, dependency, payload, ownership, namcap, consumer and removal checks (`build-arch-112-reviewed`). Checks ran once per archive candidate; downloaded release consumers were not rerun and no evidence was uploaded as release assets.
- `python3 scripts/check-docs.py --release` and `git diff --check` passed.
- The release PR must still pass both final-head packaging jobs and produce its retained release gate. These final-input jobs repeat the complete exact Qt matrix; candidate matrix results above do not replace that gate. Merge, tagging and publication remain separately authorized actions.

## PR verification and Qt 6.8 input fixture

Release [PR #35](https://github.com/lebedenko/qmarkdown/pull/35) contains this change. The first final-head run, 38917243005, exposed a fixture issue in the phased-touchpad row on Qt 6.8: its native delivery agent suppresses pixel-only compatibility ScrollUpdate events after an accepted event. The high-resolution fixture now supplies both pixel and angle deltas, with a touchpad device, ScrollBegin and monotonic event timestamps. Production library inputs are unchanged from 089dc30.

The corrected fixture passed **33/33 code-block cases** against the existing extracted/installed Qt 6.8.0 backend-OFF package (`build-touchpad-112-minimum`, `/tmp/touchpad-112-minimum-fixture-fixed.log`). This isolates the correction from library changes. Final-head packaging and release-gate results are tracked in PR #35's verification section and GitHub checks; they remain required before merge. No 1.12 merge/tag/publication action is authorized by these records.

## Live-font gutter regression

A focused review found that calling the FontMetrics `advanceWidth()` invokable does not by itself establish a font-notification dependency. The gutter binding now reads its font and DPI metrics explicitly. `gutterTracksFontChanges` changes the code font from 12 px to 28 px and verifies that gutter width grows. The focused code-block suite and QML lint passed on Qt 6.12.0. The source distribution checks are refreshed for this QML input change; final-head CI includes the regression on both supported Qt SDKs.

## Final font-corrected distribution — commit d5d3795

- [PR run 38921182308](https://github.com/lebedenko/qmarkdown/actions/runs/38921182308) passed both exact Qt packaging jobs and the retained release gate on preceding head 53ec0e2. This includes the corrected phased-touchpad fixture across all shared/static backend OFF/ON configurations.
- The final font-dependency change and its regression are in commit d5d3795. Focused host code-block tests and QML lint passed, with no new lint warnings.
- Refreshed archive: `build-dist-112-font-final/qmarkdown-1.12.0.tar.gz`. All four extracted source build/install checks passed (`build-source-112-font-final-6.8.0`, `build-source-112-font-final-6.12.0`). The Qt 6.8 backend-OFF installed package also passed **34/34 code-block cases**, including live font resizing and wheel/trackpad handoff.
- Refreshed Arch metadata, dependency, payload, ownership, namcap, consumer and removal checks passed (`build-arch-112-font-final`).
- The final PR head repeats packaging and release-gate verification after this evidence update. Its result is retained in GitHub checks and PR #35's verification section; that final-head gate remains mandatory before merge.

## DPI-corrected archive — commit 2104bee

- [Run 38924915443](https://github.com/lebedenko/qmarkdown/actions/runs/38924915443) passed both packaging jobs and the retained release gate on font-corrected head afc87d0, including the live-font gutter regression.
- The final private plain-source cache key adds effective line height (2104bee); the focused code-block suite, QML lint and all 34 code-block cases at 1.25× and 2× offscreen scaling passed.
- `build-dist-112-dpi-final/qmarkdown-1.12.0.tar.gz` passed all four extracted source builds/installs on Qt 6.8.0/6.12.0 with backend OFF/ON (`build-source-112-dpi-final-*`).
- The same archive passed Arch metadata/dependency/payload/ownership, namcap, consumer and pacman removal checks (`build-arch-112-dpi-final`).
- PR #35's GitHub checks and verification section retain the subsequent exact final-head packaging/gate result. No 1.12 merge, tag or publication has been performed.
