# Verification

Approved on 2026-10-09 through the user’s explicit instruction to implement Iteration 016. Approval covers all five semantic distinctions, private implementation, independent evidence, and verification; package/module remain 0.7.0/0.7.

## Planned checks

Focused parser/oracle tests, complete corpus, pre/post presentation equality for all 652 examples, controlled semantic faults, stale-format rejection, strict failure, shared/static seven-test CTest suites, task ci-6.8 and task ci-6.11.

## Pre-change observations

`python3 scripts/verify-commonmark.py --report /tmp/016-before.json`: exit 0, 652 parser/model passes, zero mismatches/uncheckable, 224 examples with limits, 134 with losses. Working tree initially clean. Pre-change evidence retained in `/tmp/016-before.json`.

## Host implementation results (2026-10-09)

Linux host: Qt 6.12.0, GCC 16.2.1, Python 3.14.7. Focused parser cases for retained breaks/HTML identity, repeated emphasis/strong nesting, reference titles/empty links, image-description trees and the private fixture prefix pass. The complete parser suite passes through CTest.

`python3 tests/commonmark/test_baseline.py --probe build-shared/tests/qmarkdown-commonmark-probe`: **50 tests passed**, including eight semantic tests with controlled faults and stale probe/report/authored format rejection. Presentation assertions remain separate from exact semantic assertions. The initial authored example 494 mistakenly treated two unterminated `<b` fragments as Html; source review corrected them to Text. Initial focused-test node-count/UTF-16 expectations were corrected from manual counts before final passing checks.

`python3 scripts/verify-commonmark.py --report /tmp/016-after.json`: exit **0**, 652 parser passes, 652 model passes, zero mismatches/uncheckable, 390 examples with limits, zero with losses. `--strict --report /tmp/016-strict.json`: exit **1**, as required; reports byte-identical. Comparing every pre/post native block after removing only the new `inlines` field confirms all **652 presentation models unchanged**, including text, ranges, links/images and recursive block metadata.

All 134 formerly loss-bearing IDs now have the addressed distinctions in independently compared nodes outside opaque image-description interiors: 65 soft-break, 29 title, 19 repeated-emphasis, 2 empty-link and 25 inline-HTML annotations removed (counts overlap). Source/rule review extends all 69 authored expectations. Every existing comparison limit is retained. Two source ambiguities are newly explicit: generated inline semantic tags versus literal source HTML (258 IDs), and serialized LF versus entity-decoded LF (64 IDs). The union of limited examples rises from 224 to 390 without masking semantic nodes.

[Individual ledger review](ledger-review.md): 24 exceptions removed, 99 added for explicit ambiguity, 243 retained entries updated, and 48 unchanged entries preserved exactly. Each changed row has independent semantic equality, unchanged presentation and a concrete rationale; refreshed hashes include new source semantics. No upstream fixture, public C++/QML interface, dependency or package/module version changed.

`cmake --build build-shared --parallel 2` and static equivalent pass. `ctest --test-dir build-shared --output-on-failure`: **7/7 passed** (23.07 s), including view/resource regressions and image-slice/source-tree assertions. `ctest --test-dir build-static --output-on-failure`: **7/7 passed** (22.56 s). Final host oracle run, corpus run, Python compilation and `git diff --check` also pass. Final container results are recorded below.

## Container execution

Initial sandbox `task ci-6.8` and `task ci-6.11` attempts failed to access `/var/run/docker.sock`. Both were rerun with approved Docker access and passed. The first approved snapshots predate the final empty-Text correction; both tasks were launched again against final source. Final results below are the acceptance evidence.

## Final normalization correction

An added normalization regression assertion failed before correction: standalone empty Text leaves were being discarded by normalization. Normalization now merges only adjacent Text nodes, and the HTML oracle avoids inventing empty text while splitting serialized newlines. This exposed cmark’s empty Text artifacts after trailing-space trimming in official IDs 556/587. Adaptation omits those no-op Text leaves while retaining every empty link/image container and SoftBreak. Source review verified both cases and refreshed only their semantic fingerprints; no limits/losses or presentation fields change.

The corrected 50-test host oracle suite and focused parser case pass. The final corpus (`/tmp/016-final-host.json`) has 652 parser/model passes, zero mismatches/uncheckable/losses and 390 limited examples; pre/post comparison again confirms all 652 presentation models unchanged. Final shared CTest after the correction passes **7/7** (23.06 s); final static CTest passes **7/7** (22.40 s). Both local CI tasks were launched again against this final source so their final results cover the correction.

The first approved CI pair passed before the final empty-Text correction: Qt 6.8.0 (`build-ci/6.8.0/20261009T131626Z-ebevusoc/`, 1335.108 s) and Qt 6.11.3 (`build-ci/6.11.3/20261009T131629Z-xfarl0o3/`, 1393.087 s), each exit 0. Their actual logs/results are retained, but the final-source reruns are the acceptance evidence. Final rerun snapshots were compared byte-for-byte with every production private source/header, oracle script, fixture/ledger, probe and changed parser/resource test; all match.

## Final container results (2026-10-09)

Both final-source tasks passed on pinned Ubuntu 24.04 amd64 (`ubuntu:24.04@sha256:f610ab94648195aa356059f5b41d6085c9d4d903c072430cdd1af7bdb646106b`), with CMake 3.28.3, GCC 13.3.0, Python 3.12.3, cmark 0.31.2, official Qt SDKs, UID/GID 1000, offscreen/Fusion rendering and read-only source mounts.

| Command | Actual result | Elapsed seconds | Retained evidence |
| --- | --- | ---: | --- |
| `task ci-6.8` | Passed, exit 0 | 1333.348 | `build-ci/6.8.0/20261009T134659Z-_psavegl/` |
| `task ci-6.11` | Passed, exit 0 | 1369.853 | `build-ci/6.11.3/20261009T134659Z-2lnauhu1/` |

Each passed all seven shared and all seven static CTest entries, QML lint commands, both 140-symbol bundled-parser privacy checks, install/relocation and notice checks, shared direct-link/plugin-only and static installed consumers, a tests/examples/benchmarks-disabled build, and Release benchmark smoke plus JSON validation. The SDK download dominated runtime: Qt declarative took 1142.77 seconds on 6.8 and 1165.49 seconds on 6.11. All required checks completed without timeout.

All four final `packaging/build-{shared,static}/tests/commonmark-report.json` files validate against the final schemas/fixtures/ledger: 652 parser passes, 652 model passes, zero mismatches/uncheckable/losses and 390 examples with limits. Every raw semantic model equals the final host model, and stripping only `inlines` confirms every presentation model equals the pre-change model. `strict_pass` is false for every matrix report. The final host CLI `--strict --report /tmp/016-final-strict.json` independently returned **exit 1**; this report is byte-identical to `/tmp/016-final-host.json`.

Qt 6.8 QML lint still emits existing native type-resolution warnings while returning 0. Qt 6.11 emits unused-import information and optional Qt6TaskTree discovery warnings; Vulkan headers are unavailable. These are existing environment/tooling limits; warning-free builds are not claimed. Hosted GitHub execution remains outside this local iteration and pending.

## Final review

Changed production files: `inline.h/.cpp`, `document.h/.cpp` and `resourcecontroller.cpp`. Changed evidence/tests: the CommonMark probe, oracle, baseline validation, both authored expectation fixtures, ledger and oracle tests, plus parser/resource tests. Changed documentation: Feature 016’s requirements/design/tasks/verification and individual ledger review; specification index/roadmap/standards; corpus README/provenance. The pinned upstream corpus, generated dependency lockfiles, public interfaces, dependencies and 0.7.0/0.7 versions are unchanged.

The final source passed focused parser cases, all 50 oracle tests, the complete corpus, shared/static CTest and both final local CI tasks. Python compilation and `git diff --check` pass. All tasks are complete; remaining independent comparison limits, GFM, streaming, selection/copy, highlighting and performance optimization remain deferred.
