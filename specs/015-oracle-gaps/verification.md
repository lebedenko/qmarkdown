# Verification

Approved on 2026-10-09 through the user’s explicit instruction to implement the Iteration 015 plan. Approval covers requirements, design, tasks and verification below; production behavior and versions remain 0.7.0/0.7.

## Planned checks

Focused fixture and report faults; full corpus and unchanged production fingerprints; deterministic reports; baseline exit 0 and strict exit 1; seven shared/static CTest checks; task ci-6.8 and task ci-6.11. Actual results will be recorded separately.

## Pre-change observations

`python3 scripts/verify-commonmark.py --report /tmp/015-before.json`: exit 0; 652 parser passes, 626 model passes, zero mismatches, 26 uncheckable; 250 examples with limits and 113 with losses. Working tree initially clean.

## Host results (2026-10-09)

Linux host, Qt 6.12.0, GCC 16.2.1, Python 3.14.7. `python3 tests/commonmark/test_baseline.py --probe build-shared/tests/qmarkdown-commonmark-probe`: all **42 tests pass**, including five new remaining-fixture tests with subcases for exact IDs, provenance/review, invalid model metadata, loss/limit shape, altered entity spelling/newlines, block boundaries/nesting, tightness, wrong comparison methods, annotation tampering and stale ledger entries.

`python3 scripts/verify-commonmark.py --report /tmp/015-after.json`: exit **0**, 652 parser passes, 652 model passes, zero mismatches and zero uncheckable; 224 examples with limits and 134 with losses. `python3 scripts/verify-commonmark.py --strict --report /tmp/015-strict.json`: exit **1**, as required. The two reports are byte-identical, and repeated analysis is deterministic. Comparing the pre/post report trees confirms all **652 raw native models unchanged**. Comparing against the original ledger confirms every unrelated entry and all retained model hashes unchanged. Five superseded exceptions (21, 31, 39, 308, 309) were removed; 21 were converted to complete source-authored passes with inline HTML identity losses, including soft-break loss for 494. The 43-entry HTML-block fixture is unchanged.

The initial `ctest --test-dir build-shared --output-on-failure` and static equivalent each passed six entries but failed viewer checks because old binaries lacked the current SizeValidator registration. `cmake --build build-shared --parallel 2` and `cmake --build build-static --parallel 2` refreshed both builds without source changes. The same CTest commands then passed **all seven checks** in each variant (22.94 s shared, 22.12 s static). Offscreen rendering is set by CTest.

## Container execution

Initial sandbox attempts at `task ci-6.8` and `task ci-6.11` failed to access `/var/run/docker.sock`. Required Docker access was approved; both commands were rerun outside the sandbox. Both reruns passed; actual results are below. Hosted workflow execution remains pending.

Both used Ubuntu 24.04 amd64 with CMake 3.28.3, GCC 13.3.0, Python 3.12.3, cmark 0.31.2, official Qt SDKs, offscreen/Fusion rendering and UID/GID 1000. Image: `ubuntu:24.04@sha256:f610ab94648195aa356059f5b41d6085c9d4d903c072430cdd1af7bdb646106b`.

| Command | Actual result | Elapsed seconds | Retained evidence |
| --- | --- | ---: | --- |
| `task ci-6.8` | Passed, exit 0 | 1327.077 | `build-ci/6.8.0/20261009T094508Z-_xdqzerw/` |
| `task ci-6.11` | Passed, exit 0 | 1395.880 | `build-ci/6.11.3/20261009T094510Z-jwantopq/` |

Each passed all seven shared and all seven static CTest entries, QML lint commands, both 140-symbol parser privacy checks, install/relocation and notice checks, shared direct-link/plugin-only consumers, static installed consumers, a tests/examples/benchmarks-disabled build, and Release benchmark smoke plus JSON validation. All four `packaging/build-{shared,static}/tests/commonmark-report.json` files report 652 parser and 652 model passes, zero mismatches/uncheckable, 224 examples with limits and 134 with losses. Comparing every raw native model to the pre-change host report confirms all 652 are unchanged across both versions and variants. The snapshots contain the reviewed fixture and ledger; the final test assertion simplification was also checked by the host oracle suite.

The elapsed times include slow SDK downloads: Qt declarative installation alone took about 1126 seconds on 6.8 and 1192 seconds on 6.11. No timeout or code failure occurred in either approved rerun. Environment/result JSON, full logs, installer and package manifests, tool versions, packaging reports and benchmark JSON are retained in the ignored artifact directories.

Qt 6.8 QML lint emits existing native type-resolution warnings despite exit 0; Qt 6.11 emits an unused-import informational message and optional Qt6TaskTree discovery warnings. Vulkan headers are unavailable. Required checks pass; warning-free builds/lint are not claimed. Hosted GitHub checkout/upload/workflow execution remains pending. No production files, public interfaces, dependencies, or versions changed. Semantic retention remains the next conformance workstream.

## Final review

`python3 -m py_compile tests/commonmark/baseline.py tests/commonmark/test_baseline.py`, the final 42-test host oracle run, and `git diff --check` passed. The existing HTML-block fixture and pinned corpus are unchanged; no unrelated ledger entry was modified. Changed files are the new remaining-expectations fixture, oracle runner/validation and fault tests, reviewed ledger, corpus README/provenance, Feature 015's four records, and the specification index/roadmap/standards. Feature 014's completed local results are now recorded in the roadmap while hosted execution remains pending.
