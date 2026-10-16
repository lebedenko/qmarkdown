# Verification — Language-aware headers

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Publish QMarkdown 1.11, then implement 1.12 code-block usability” plan. Scope includes Features 050–053 and one package 1.12.0 / QML 1.12 release PR. Merge, tagging and publication of 1.12 require separate authorization.

## Actual results — 2026-10-16

- QMarkdown 1.11.0 publication completed before this branch was created. [Publication run 38906093482](https://github.com/lebedenko/qmarkdown/actions/runs/38906093482) passed eligibility, both extracted-source Qt jobs, Arch lifecycle checks and publication. Publisher output: `PASS: published v1.11.0; all four downloaded SHA-256 values match`.
- Host SDK synchronization selected Qt 6.12.0 without changing configured targets.
- Local shared backend-ON build passed. `ctest --test-dir build-shared --output-on-failure`: **31/31 passed**, 93.54 seconds.
- Focused coverage: `CodeBlocksTest::defaultsAndLegacyHeader`, `metadataAndCopy` and existing renderer/action lifecycle tests.
- QML lint target passed. Dynamic Loader interface warnings are tooling limitations; the import-only example retains its existing unused-import informational message.
- Strict release documentation and local links passed using `python3 scripts/check-docs.py --release`; `git diff --check` passed.
- Earlier failing attempts exposed and corrected component ownership, missing-required diagnostics, source geometry, icon resource resolution and selection/panning priority. Their logs remain in local build directories; they are not release assets.

## Pending delivery verification

Exact Qt 6.8.0 and 6.12.0 shared/static backend OFF/ON verification is running in retained snapshot directories `build-ci-112-minimum-final` and `build-ci-112-development-final`. Extracted source, Arch distribution checks and final PR gates have not yet completed. No successful result is claimed for these checks.
