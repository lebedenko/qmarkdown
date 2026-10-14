# Verification

## Planned checks

Focused offline documentation/publication/upload tests; normal/strict documentation and local-link validation; whitespace; configured Qt 6.8.0 and 6.12.0 full shared/static matrix.

## Actual results

On 2026-10-14, the final implementation passed:

- `python3 tests/test_release_delivery.py`: 15 tests, including subcases for stale versions below headings, mixed summary/download versions, wrong repositories/assets/imports/compatibility bounds, publication disclaimers and future-download wording. Historical records and explicitly separated development work pass; newer development metadata passes ordinary mode and fails strict release mode.
- `python3 tests/test_release_publish.py`: 8 fake-GitHub tests. Invalid documentation leaves zero remote writes, no tag and no release. Valid publication and identical-existing-release retries retain downloaded-hash checks; corrupt downloads, moved main and differing assets still fail.
- `python3 tests/test_release_upload.py`: 10 tests covering asset validation, retry and remote-byte comparison.
- `python3 scripts/check-docs.py` and `python3 scripts/check-docs.py --release`: current summaries and local links pass.
- `git diff --check`: pass.
- `python3 scripts/ci-toolchain/qt_versions.py --sync-host qmake6`: host is 6.12.0, matching the configured development target; no configuration change.
- `python3 scripts/run-ci.py --qt-role minimum`: exit 0 on Qt 6.8.0. Final-input local evidence: `build-ci/6.8.0/20261014T154743Z-ya740309/{ci.log,result.json,environment.json}`.
- `python3 scripts/run-ci.py --qt-role development`: exit 0 on Qt 6.12.0. Final-input local evidence: `build-ci/6.12.0/20261014T154745Z-jd61rtpu/{ci.log,result.json,environment.json}`.

Each final full matrix run passed 17 shared and 17 static CTest tests, installed/relocated consumers and compatibility requests, QML lint, private-symbol checks, release-build checks and advisory benchmark smoke validation. Earlier snapshots also passed but are not the final-input evidence. Initial sandbox attempts could not access Docker; the final runs succeeded with authorized daemon access.

Hosted PR checks remain a separate required gate; no protected merge or publication is claimed.

## Remote follow-up

A read-only `gh release view v1.7.0 --json url,body,assets` confirmed all four documented asset names and the existing [v1.7.0 release](https://github.com/lebedenko/qmarkdown/releases/tag/v1.7.0) description ending with “Publication requires separate authorization.” A targeted description correction is separate from this repair. No remote release, tag or asset modification is included.
