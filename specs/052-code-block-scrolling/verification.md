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
