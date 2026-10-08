# 011: CommonMark conformance baseline verification

**Approval:** Requirements, design and T1–T6 were explicitly approved on 2026-10-08 before implementation. Scope is tests/documentation; production behavior and package/module 0.7.0/0.7 remain unchanged.

## Planned checks

- Pinned fixture checksum, attribution/license, schema and complete unique identifiers; offline build/run.
- All official examples through the bundled parser, with exact expected/actual HTML and no skipped examples.
- All official examples accounted for in production-model comparisons; independent oracle fixtures, explicit loss/uncheckable inventory and UTF-16 assertions.
- Deterministic report, truthful separate totals and reviewed per-ID ledger; baseline/strict failure semantics tested using controlled faults.
- Existing native renderer/resource regressions remain separate evidence.
- Focused checks followed by shared/static CTest, lint and symbol privacy; packaging checks conditional on affected integration boundaries. Record Qt/tool versions and minimum-Qt availability.

## Actual results

Executed on 2026-10-08 on Linux, using Qt 6.12.0, bundled cmark 0.31.2, GNU C/C++ 16.2.1, Python 3.12.13 (CMake/CTest) and Python 3.14.7 (shell runner).

- T1: imported the unchanged official `spec.json` and accompanying CC BY-SA 4.0 license text; retained author/version/origin/date and checksum in [provenance](../../tests/commonmark/PROVENANCE.md) and `fixture.json`. Derived 652 examples, IDs 1–652; schema/IDs/checksum validate offline. Fixture SHA-256: `d431b29d97b6f73e69d547109cf5081578fac931e72afe95639ebe766c1b2a20`.
- T2: private probe compiled with the same prefixed bundled objects and production adapters. All 652 exact official HTML comparisons passed with `CMARK_OPT_DEFAULT` parsing and `CMARK_OPT_UNSAFE` only in test serialization: total 652, pass 652, fail 0, skipped 0. No public or installed helper was added; production HTML rendering remains absent.
- T3: all 652 production-model outputs were validated and retained in reports. Independent HTML projection: 583 projection passes, 0 mismatches, 69 explicitly uncheckable, 0 skipped. Authored checks cover UTF-16/astral offsets, nested formatting, code whitespace/info, recursive tight/loose lists/quotes, heading separators, titles, empty links, adjacent/empty/formatted/linked images, literal raw HTML, entity LFs, source-line normalization and original URL strings.
- T4: manually inspected pinned source/HTML and all uncheckable native outputs; reviewed code/list/image source metadata and passing projection families. The ledger records 363 exact-ID exceptions with reasons, review notes and raw-model hashes, including opaque fields. Counts overlap: 293 examples have comparison limits and 113 have detected semantic information loss. There are no established bundled-parser/adapter defects in this baseline; oracle and intentional projection limits remain.
- T4: 33 authored oracle/model/fault tests passed on both Python versions. Faults cover missing/duplicate/invalid fixtures, changed checksum, missing/unknown response fields, wrong IDs/options/version, split UTF-16 characters, invalid ranges/list placement, invalid report objects/counts/differences, invalid ledger provenance/IDs/review, newly introduced parser/model/oracle failures, unchanged statuses with worse evidence, hidden raw HTML/image formatting changes and stale improved ledger entries. Probe requests reject duplicates, extra fields and fractional IDs.
- T4: initial shared CTest exposed a Python-version difference in malformed short-comment recovery (example 626). Explicit construct rejection and an authored regression case resolved it within test infrastructure. Final Python 3.12/3.14 reports are identical apart from recorded tool versions; no production code was changed.
- T4: repeated Python 3.14 runs produced byte-identical JSON reports, SHA-256 `4169758342e2e1c964225e97620e21441e1f5bce4acf38b2f7ba38a307889e10`. Strict mode returned exit 1 and still wrote the complete report, as required for incomplete evidence. Baseline mode returned 0. A nonexistent probe returned the documented execution-error exit 2.
- T5: both new checks are integrated into ordinary CTest. The runner, strict mode, exit codes, licenses, comparison rules and reviewed gap inventory are documented in [tests/commonmark/README.md](../../tests/commonmark/README.md), root README and standards/roadmap/index.
- T5: a fresh library-only configure/build with `BUILD_TESTING=OFF`, examples off and Python package finding disabled succeeded. The disable-find variable was unused because no Python lookup ran; no fixtures were needed by the build. Exported package configs/targets and generated test install rules in shared/static trees contain no new probe, fixture or Python entries.
- T6: shared and static builds each passed full CTest **6/6**, QML lint and symbol privacy (**140 prefixed external definitions**, no cmark dynamic exports). Existing native renderer/resource checks passed separately from official semantic evidence. Lint emitted only the established import-only unused-import informational message. After final validation-only cleanup, the two affected CommonMark CTest checks passed again in both variants.
- T6: local documentation links and `git diff --check` passed. No production source, public API, parser bundle, package version, resource policy or dependency lockfile was changed. Relocated packaging was not rerun: the new targets are test-only, with no changes to installation/export/configuration contents; the Feature 010 packaging evidence remains the separate package record.

## Reproducible commands

Fixture acquisition is recorded in [provenance](../../tests/commonmark/PROVENANCE.md). Configure/build/run commands used:

```sh
cmake -S . -B build-shared -DBUILD_TESTING=ON -DQMARKDOWN_BUILD_EXAMPLES=ON
cmake --build build-shared --target qmarkdown-commonmark-probe --parallel 2
python3 tests/commonmark/test_baseline.py --probe build-shared/tests/qmarkdown-commonmark-probe
python3.12 tests/commonmark/test_baseline.py --probe build-shared/tests/qmarkdown-commonmark-probe
python3 scripts/verify-commonmark.py
python3 scripts/verify-commonmark.py --strict --report /tmp/qmarkdown-commonmark-strict-final.json
cmake --build build-shared --parallel 2
ctest --test-dir build-shared --output-on-failure
cmake --build build-shared --target all_qmllint
python3 scripts/verify-cmark-symbols.py build-shared
cmake -S . -B build-static -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=ON -DQMARKDOWN_BUILD_EXAMPLES=ON
cmake --build build-static --parallel 2
ctest --test-dir build-static --output-on-failure
cmake --build build-static --target all_qmllint
python3 scripts/verify-cmark-symbols.py build-static
cmake -S . -B /tmp/qmarkdown-commonmark-library-only -DBUILD_TESTING=OFF -DQMARKDOWN_BUILD_EXAMPLES=OFF -DCMAKE_DISABLE_FIND_PACKAGE_Python3=ON
cmake --build /tmp/qmarkdown-commonmark-library-only --parallel 2
ctest --test-dir build-shared --output-on-failure -R qmarkdown-commonmark
ctest --test-dir build-static --output-on-failure -R qmarkdown-commonmark
```

Reports: `build-shared/tests/commonmark-report.json`, `build-static/tests/commonmark-report.json`, `/tmp/qmarkdown-commonmark-python312.json`, `/tmp/qmarkdown-commonmark-python314.json`, `/tmp/qmarkdown-commonmark-repeat-{a,b}.json`, `/tmp/qmarkdown-commonmark-strict-final.json`. Reports enumerate every example, section totals, raw native models, comparison limits/losses, statuses and mismatch evidence when present; they contain no timing or build paths.

## Limits and next proposal

Full CommonMark conformance remains **not verified**; the strict gate fails as expected. A projection pass is a check of the specified native projection, not proof of all source semantic distinctions. Raw hashes are regression guards, not independent oracle expectations. The 69 uncheckable examples are accounted for and fingerprinted, but lack independent official-model comparisons; additional information loss may be present in them.

Qt 6.8 and Python 3.9 execution were unavailable. Runtime verification used Qt 6.12.0 and Python 3.12/3.14. Native tests use offscreen rendering and synthesized input, as in the existing suite. There were no public network requests during builds/tests; only the explicit one-time fixture/license acquisition used networking.

Recommended next bounded specification: independently authored, source-aware native expectations for the 43 HTML-block cases currently uncheckable (148–167 and 169–191), including quote/list boundaries and mixed literal/native blocks. This is the largest oracle gap affecting file-preview semantics; it does not establish a need to change production behavior. Remaining oracle cases and richer private semantic retention should be designed separately; see the complete [gap inventory](../../tests/commonmark/README.md). No follow-up implementation is authorized by Feature 011 approval.
