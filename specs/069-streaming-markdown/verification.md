# Actual verification

## Scope and baseline

Implemented on `iteration-069-streaming-markdown`, branched from merged `c2895f9`. The failed renderer and diagnostic commits remain on the original branch and are not runtime inputs here. User approval was recorded before implementation. No commit, push, PR, merge, release, or system-package replacement has been performed.

## Focused host checks — Qt 6.12.0

- Streaming tests passed: accepted source versus publication, one batched publication, explicit flush with no delayed duplicate, disabling, replacement while pending, identical/empty inputs, settings changes, nested source notifications, reentrant publication and destruction during source/publication notifications.
- Static equivalence passed for every character-published prefix of representative incomplete emphasis, links, fenced code, nested lists, Setext headings, GFM tables, late reference definitions, math, footnotes, CRLF and a split UTF-16 surrogate pair. Model roles, recursive child content, headings and copy projections were compared with fresh static views.
- All 652 pinned CommonMark examples and all five pinned GFM extension fixture sets passed final-view comparisons under character-sized and deterministic mixed-size chunks, with publication after each chunk.
- Recursive model identity, absence of resets, retained selections/Find occurrence, invalidated semantic selections and absence of streaming navigation passed. Custom renderer/action objects and live copy callbacks survived appends; replacement invalidated stale callbacks. Pending highlighting subscriptions survived unchanged code content. A retained focused link survived an earlier reference definition shifting its logical ID without navigation.
- Deterministic resource tests passed for retained in-flight requests, removed request cancellation, decoded-image reuse, completed failures, replacement/policy cancellation, removed decode results and a reentrant completion adding a new resource without double decoding.
- The initial broad host run identified the import-version expectation, a side-effectful semaphore assertion and focus restoration; these were corrected and focused checks repeated successfully. This initial run is not passing Full evidence.
- Qt-independent checks passed: documentation/version/local links and 87 Python tooling tests, including the remaining integrated fixture checks. `git diff --check` passed.

## Release measurements

See [harness and method](benchmark/README.md) and [raw results](benchmark/results-qt-6.12.0.json). Host Qt 6.12.0 / GCC 16.2.1, Release, offscreen; one timed pass, no warmup, concurrent CI contention. Compiler caching was not enabled for this host harness. Deterministic counts, rather than timings, establish acceptance.

| Size | Mode | Publications | Root resets | Delegate creations / removals | Image decodes / restarts | Retained images | Time (ms) |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 KiB | Replacement | 1 | 1 | 17 / 0 | 1 / 0 | 1 | 20.50 |
| 1 KiB | Streaming | 1 | 0 | 17 / 0 | 1 / 0 | 1 | 7.66 |
| 10 KiB | Replacement | 10 | 10 | 815 / 669 | 10 / 9 | 1 | 383.87 |
| 10 KiB | Streaming | 3 | 0 | 148 / 2 | 1 / 0 | 1 | 69.13 |
| 100 KiB | Replacement | 100 | 100 | 73,049 / 71,605 | 100 / 99 | 1 | 42,196.92 |
| 100 KiB | Streaming | 25 | 0 | 1,457 / 13 | 1 / 0 | 1 | 1,425.37 |

No incremental parsing, guaranteed frame rate or guaranteed throughput is claimed. The 1 KiB case has only one input chunk and therefore demonstrates retention semantics rather than batching.

## Full container verification

Both final Full runs passed with schema `qmarkdown-ci-v2`, profile `full`, and exit code 0. They captured immutable source snapshots, including uncommitted files. A byte comparison confirmed that all 177 shipping, test and tooling files match both acceptance snapshots; later edits are verification records and prose only.

- Qt 6.8.0 final snapshot: `build-ci/6.8.0/20261027T201258Z-s1mlyd60`.
- Qt 6.12.0 final snapshot: `build-ci/6.12.0/20261027T201257Z-fhfa0a54`.

Earlier runs captured intermediate source and do not establish final acceptance. Superseded containers were explicitly stopped when implementation inputs changed; their termination results are not functional acceptance failures. The first 6.8.0 run failed the focus regression before its fix. Final runs cover shared/static and syntax OFF/ON, math-disabled linkage variants, library-only configuration, installed/relocated consumers, QML lint and symbol privacy; development Qt also runs validated Release benchmark smoke. Both targets passed these checks. Qt 6.12.0 also passed validated Release benchmark smoke with image completions reconciling the model and only the initial assignment resetting it.

| Target | Verification time | Packaging / math / benchmark phases | Compiler cache |
| --- | ---: | --- | --- |
| Qt 6.8.0 | 899.081 s | 825 / 73 / not scheduled | Warm reused cache: 1,100 hits, 217 misses (83.52%) |
| Qt 6.12.0 | 1,035.909 s | 852 / 83 / 100 s | Warm reused cache: 1,106 hits, 356 misses (75.65%) |

Both toolchain images were reused. These are local container results, not hosted CI or release authorization. Per-run `result.json`, `ci.log`, `environment.json`, `phases.txt` and `ccache-stats.txt` remain under the exact snapshot directories above. Supervisor logs are `/tmp/qmarkdown-1.18.0-minimum-acceptance.log` and `/tmp/qmarkdown-1.18.0-development-acceptance.log`.

The installed streaming consumer passed in linked and plugin-only shared modes and static linked mode. The full matrices also retained all existing consumers and optional-feature checks. Final documentation/version/link validation and whitespace checks passed after recording evidence.

## Changed files

Package and tooling:

- `CMakeLists.txt`
- `scripts/verify-benchmark.py`
- `src/QMarkdown/CMakeLists.txt`

Runtime and playground:

- `examples/viewer/Main.qml`
- `src/QMarkdown/MarkdownView.qml`
- `src/QMarkdown/private/BlockSequence.qml`
- `src/QMarkdown/private/inline.h`
- `src/QMarkdown/private/linkcontroller.cpp`
- `src/QMarkdown/private/linkcontroller.h`
- `src/QMarkdown/private/presentation.h`
- `src/QMarkdown/private/private.qmltypes`
- `src/QMarkdown/private/resourcecontroller.cpp`
- `src/QMarkdown/private/resourcecontroller.h`
- `src/QMarkdown/private/viewstate.cpp`
- `src/QMarkdown/private/viewstate.h`

Tests and consumers:

- `tests/CMakeLists.txt`
- `tests/benchmark.cpp`
- `tests/installed-consumer/CMakeLists.txt`
- `tests/installed-consumer/streaming.cpp`
- `tests/tst_import.cpp`
- `tests/tst_keyboard.cpp`
- `tests/tst_providers.cpp`
- `tests/tst_renderers.cpp`
- `tests/tst_resources.cpp`
- `tests/tst_view.cpp`

Documentation and specifications:

- `CHANGELOG.md`
- `README.md`
- `docs/api.md`
- `docs/reading-navigation.md`
- `docs/streaming.md`
- `specs/069-streaming-markdown/benchmark/CMakeLists.txt`
- `specs/069-streaming-markdown/benchmark/README.md`
- `specs/069-streaming-markdown/benchmark/main.cpp`
- `specs/069-streaming-markdown/benchmark/results-qt-6.12.0.json`
- `specs/069-streaming-markdown/design.md`
- `specs/069-streaming-markdown/requirements.md`
- `specs/069-streaming-markdown/tasks.md`
- `specs/069-streaming-markdown/verification.md`
- `specs/README.md`


## Release preparation — 2026-10-27 UTC

Subsequent user approval extends delivery scope as recorded in requirements, design and tasks. Release preparation changes documentation only; runtime, tests and build inputs retain the locally verified snapshots above. Final task check passed all 87 Python tooling tests; release-mode documentation validation and git diff --check passed. Hosted final-head Full verification, protected merge, distribution checks, publication and downloaded hashes are recorded in the release PR and Actions, not preclaimed here.
