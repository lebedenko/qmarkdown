# Verification — Standalone display math

Approval: requirements, design and tasks approved by the user response “approved” on 2026-10-16. Dependency audit and verification remain implementation tasks; no success is implied. One cohesive 1.13.0 / QML 1.13 release PR is planned; merge, tagging and publication require separate authorization.

Release delivery approval — 2026-10-17: the user authorized the release plan for PR #36, including documentation completion, fresh final-head Qt 6.8.0/6.12.0 CI and retained full gate, squash merge through normal protections, `release.yml` dispatch with `release_pr=36`, tagging `v1.13.0`, stable/latest publication and downloaded hash verification of exactly four assets. Retain package 1.13.0 / QML 1.13 and existing APIs; no runtime changes. Stop on failed gates, changed inputs or differing published bytes. Historical approvals and results below remain unchanged; this approval supersedes earlier statements that delivery still requires authorization.

## Planned checks

See the requirement-to-check map in [tasks](tasks.md). Run narrow fixtures first, then the relevant exact Qt 6.8.0/6.12.0 shared/static matrix, QML lint/imports, relocated installs, extracted-source builds and Arch packaging. Math ON/OFF both require coverage. Feature 055 additionally requires representative native Wayland acceptance and captures at 100%, 125%, 150% and 200%, including sharpness regressions. Required final-head packaging jobs remain the delivery gate.

## Actual results — 2026-10-16

- Requirements, design and tasks were approved by the user response “approved”.
- Host Debug shared suite passed **23/23**, including **652/652** CommonMark
  semantic passes (zero losses, limits or uncheckable cases). Focused math tests
  also passed in static and backend-OFF configurations.
- Both pinned Qt **6.8.0** and **6.12.0** complete candidate matrices passed:
  shared/static, syntax highlighting OFF/ON, **32/32** CTest tests per variant,
  QML lint/import compatibility, installed and relocated consumers (including
  native math through linked and plugin-only consumers), license payloads and
  benchmark smoke. Separate math-OFF tests passed on both SDKs.
  Retained directories: `build-ci-math-minimum-final` and
  `build-ci-math-development-final`. Qt 6.8 lint retains its existing QFont/QColor
  metadata warning pattern; the lint targets pass. Host Qt 6.12 lint passes.
- Focused `tst_math.cpp` coverage includes nested/container delimiters, blank
  lines, protected/unmatched syntax, all 32 GFM flag combinations, literal source
  projection/copy/Find, nine formula families, invalid/malformed/over-limit input,
  resource removal in a child process, pending replacement/destruction, style
  inheritance/reset, custom precedence, narrow overflow and wheel/trackpad handoff.
- Representative formula captures passed ink/sharpness checks at **100%, 125%,
  150% and 200%**. Inspected 100% and 125% captures show complete fractions,
  radicals, sums, integrals, matrices and aligned equations. Native Wayland at
  125% passed capture, wheel/trackpad handoff and source-mode transitions; its
  captured output was visually inspected. Local images remain in `/tmp` and are
  not release assets.
- Strict release documentation/local-link validation and repository whitespace
  checks passed. Upstream vendor whitespace is preserved by `.gitattributes`.

## Corrected attempts

Earlier candidates exposed extra hidden text in traversal, missing glyphs during
QPicture tiny-font replay, partial acceptance of missing arguments, an existing
FormattedText window-detachment destructor assertion in Debug, and release
consumer fixtures requesting exact 1.12.0. These were corrected before the final
candidate matrices. The failed attempts remain under `build-math` logs and
`build-ci-math-*-full`; their results are not completion evidence. The sandbox-only
initial CI attempt could not access Docker; the authorized Docker runs succeeded.

## Remaining delivery verification

Extracted-source and Arch checks, and the final PR-head required packaging jobs,
are recorded below when complete. Merge, tagging and publication are not
performed or authorized by these records.

## Final source and Arch distribution checks

- Archive: `build-dist-math-final/qmarkdown-1.13.0.tar.gz`; SHA-256
  `2c8223bb046449fbae340171dd58072977dab693bae22b07aa40c50f26b91f3b`. Its explicit 292-file payload excludes unused Greek/Cyrillic
  alphabet resources and upstream sample sources.
- Extracted source build/install with highlighting ON passed on Qt 6.8.0 and
  6.12.0 (`build-dist-math-final/extracted-minimum` and
  `build-dist-math-final/extracted-development`). The full candidate matrix above
  covers both highlighting modes and shared/static installed/relocated consumers.
- Arch package from that same archive passed metadata, dependency, license/resource
  payload, namcap, ownership, linked/plugin-only native math consumers, integrity
  and pacman removal checks (`build-arch-math-final`). No system package was
  replaced; lifecycle checks ran in disposable containers.
- Final PR-head required packaging jobs remain mandatory. No merge, tag or
  publication has been performed.

## Post-review bounds and refreshed distribution — 2026-10-17

- Recursive command/argument entry and inline layout methods now share the depth
  guard. Logical painted area is capped at 4 megapixels before item texture
  allocation. `recursiveAndPaintBudgets` verifies nested unbraced commands and
  oversized matrices fall back, followed by successful rendering of a valid input.
- Focused math tests and the full host Debug suite passed **23/23** after these
  corrections (77.46 seconds, `/tmp/qmarkdown-math-budget-ctest.log`).
- Refreshed archive: `build-dist-math-bounded/qmarkdown-1.13.0.tar.gz`, SHA-256
  `d57b685e69dfb8e4dcaf7f8fcb5bfd514a5ac5d38fba886307f7f5634aec420c`. Both exact Qt extracted-source build/install checks passed
  (`build-dist-math-bounded/extracted-minimum`, `extracted-development`).
- The same archive passed the complete Arch metadata, dependency, license/resource
  payload, ownership, namcap, math consumers, integrity and removal checks
  (`build-arch-math-bounded`). This supersedes the preceding archive for delivery.
- Release [PR #36](https://github.com/lebedenko/qmarkdown/pull/36) contains the
  implementation and evidence. The earlier PR-head run is superseded by the
  bounds correction; the final head must pass both required packaging jobs and
  retain its release gate. Current final-head results are tracked in the PR's
  checks and verification section rather than prewritten here. CI additionally
  checks math-OFF with shared and static linkage on both SDKs. No release PR merge,
  tagging or publication has been performed.

## Hosted image-selection fixture correction — 2026-10-17

The preceding PR run 38970204291 failed `SelectionTest::tableAndImageLifecycle`
in the development static build: resource completion created the image before
its containing layout polished the image position, so the fixture clicked the
preceding paragraph. The fixture now completes a window frame before capturing
hit-test coordinates. This changes tests only. It passed 25 repetitions on the
host and 25 against each previously verified Qt 6.8.0/6.12.0 static package
(`/tmp/qmarkdown-selection-frame-minimum.log`, `-development.log`). Production
archive bytes and source/Arch verification above remain unchanged. Final required
PR-head packaging results are retained in PR #36 after this fixture update.

## Release delivery preparation — 2026-10-17

Host `qmake6 -query QT_VERSION` reports 6.12.0, matching the configured development target; minimum remains 6.8.0. `python3 scripts/check-docs.py --release` and `git diff --check` passed after the release date and delivery-scope updates. Fresh final-head CI, merge and publication results will remain in PR #36 and Actions; no success is implied here.
