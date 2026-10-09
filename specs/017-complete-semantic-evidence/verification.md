# Verification

Approved on 2026-10-09 by the explicit instruction to implement Iteration 017. Approval covers requirements, design, tasks and verification below; production changes require renewed approval.

## Planned

Focused oracle tests and six semantic faults; invalid/missing/duplicate fixtures and stale reports; aggregation faults. Strict corpus; raw equality against `/tmp/017-before.json`; shared/static CTest and both local Qt CI tasks.

## Actual

Initial working tree clean. Pre-change corpus exit 0: 652 parser/model passes, zero mismatches/uncheckable/losses, 390 limited examples. Raw models saved in `/tmp/017-before.json`.

Host focused oracle suite: **56/56 passed**, including six controlled source faults, provenance/shape/ID/review rejection, report schema 3 rejection, missing/fabricated checks and failed HTML despite passing source. Existing 69 authored fixtures are byte-unchanged. Initial URL expectations 526, 538 and 603 used HTML escape spelling; source review corrected the bracket/backslash destinations before acceptance. No production defect found or production change made.

`python3 scripts/verify-commonmark.py --strict --report /tmp/017-strict.json`: exit **0**, 652 parser passes, 652 model passes, zero mismatches/uncheckable/aggregate limits/losses. Report schema 4 retains 583 HTML checks (390 limited checks retained visibly) and 459 complete source checks. Each of all 390 former ledger rows is mapped to source evidence in `ledger-review.json`; ledger exceptions are empty. Comparing `nativeModel` for each of all 652 IDs against `/tmp/017-before.json` confirms exact raw equality, including all presentation fields and recursive trees.

Host tools: Qt 6.12.0 (probe), GCC 16.2.1, CMake 4.4.4, Python 3.14.7, bundled cmark 0.31.2. `ctest --test-dir build-shared --output-on-failure`: **7/7 passed**, 23.82 s. Static equivalent: **7/7 passed**, 23.05 s. Both include native view/resource regressions. No production sources changed, so existing host binaries exercise the identical implementation. Python compilation and `git diff --check` pass.

Both initial local CI attempts failed with Docker socket permission denial in the sandbox (task exit 201, inner exit 1). Approved Docker reruns completed successfully at `build-ci/6.8.0/20261009T150128Z-sztuupue/` and `build-ci/6.11.3/20261009T150131Z-ag5yxkdw/`; their final results are recorded below.

The code-owned semantic ID set equals exactly the 390 keys in `HEAD:tests/commonmark/ledger.json` (iteration 016). Every former limits/losses array in the removal record equals that ledger entry. Both approved CI snapshots byte-match the final baseline, oracle tests, semantic fixture, empty ledger, CLI and removal record. The unchanged upstream corpus, existing authored fixtures and all production/public/version/dependency files remain outside the diff.

## Final local CI results

Both approved tasks passed against the final code/fixtures/tests on pinned Ubuntu 24.04 amd64 (`ubuntu:24.04@sha256:f610ab94648195aa356059f5b41d6085c9d4d903c072430cdd1af7bdb646106b`), CMake 3.28.3, GCC 13.3.0, Python 3.12.3, cmark 0.31.2, UID/GID 1000, offscreen/Fusion rendering and read-only source mounts.

| Command | Actual result | Elapsed seconds | Retained evidence |
| --- | --- | ---: | --- |
| `task ci-6.8` | Passed, exit 0; Qt 6.8.0 | 1339.020 | `build-ci/6.8.0/20261009T150128Z-sztuupue/` |
| `task ci-6.11` | Passed, exit 0; Qt 6.11.3 | 1465.767 | `build-ci/6.11.3/20261009T150131Z-ag5yxkdw/` |

Each task passed seven shared and seven static CTest entries, QML lint, both 140-symbol bundled-parser privacy checks, installation/relocation/notices, shared direct-link/plugin-only and static installed consumers, the tests/examples/benchmarks-disabled build, and Release benchmark smoke/JSON validation. SDK downloads dominated runtime: Qt Declarative took 1126.35 s for Qt 6.8 and 1257.07 s for Qt 6.11. All required checks finished within their existing timeouts.

All four matrix `packaging/build-{shared,static}/tests/commonmark-report.json` files validate against final schema 4, all fixtures and the empty ledger: 652 parser/model passes, zero mismatches/uncheckable/aggregate limits/losses and strict acceptance true. All 652 raw models equal the iteration 016 capture and host report. Every production source in both CI snapshots byte-matches current source. Tests, CLI and evidence files also match both snapshots. The final host CLI `--strict --report /tmp/017-final-strict.json` exits 0.

Existing Qt 6.8 native type-resolution lint warnings and Qt 6.11 unused-import/optional Qt6TaskTree discovery messages remain; Vulkan headers are unavailable. These checks still return success; warning-free builds are not claimed. Remote CI execution and release publication remain deferred.

## Final changed files and outcome

Added `tests/commonmark/semantic-expectations.json` and Feature 017 requirements/design/tasks/verification plus the individual `ledger-review.json`. Updated `baseline.py` (source loading, dual checks and schema 4 validation), `test_baseline.py` (faults and evidence validation), `ledger.json` (no exceptions), CLI status wording, corpus README/provenance, specification index/roadmap/standards. Existing 69 authored models, upstream corpus, probe, production/public APIs, dependencies and 0.7.0/0.7 versions are unchanged.

Focused 56-test oracle suite, strict complete corpus, raw equality, shared/static host CTest and both local CI tasks pass. Python compilation and final `git diff --check` pass. All authorized tasks are complete. Pinned-corpus semantic evidence is complete; native presentation and release requirements remain separate, and v1.0 readiness is not declared.
