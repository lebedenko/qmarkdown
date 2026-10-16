# Verification — Bounded code-block scrolling

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Publish QMarkdown 1.11, then implement 1.12 code-block usability” plan. Scope includes Features 050–053 and one package 1.12.0 / QML 1.12 release PR. Merge, tagging and publication of 1.12 require separate authorization.

## Actual results — 2026-10-16

- QMarkdown 1.11.0 publication completed before this branch was created. [Publication run 38906093482](https://github.com/lebedenko/qmarkdown/actions/runs/38906093482) passed eligibility, both extracted-source Qt jobs, Arch lifecycle checks and publication. Publisher output: `PASS: published v1.11.0; all four downloaded SHA-256 values match`.
- Host SDK synchronization selected Qt 6.12.0 without changing configured targets.
- Local shared backend-ON build passed. `ctest --test-dir build-shared --output-on-failure`: **31/31 passed**, 93.54 seconds.
- Focused coverage: `scrollbarsAndShortBlocks`, `wrappingHeightGutterAndFind`, existing selection and reading navigation tests.
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
