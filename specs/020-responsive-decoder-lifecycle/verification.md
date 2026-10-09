# Verification

Approved on 2026-10-09 by explicit user approval. Scope covers requirements, design and tasks for the responsive decoder lifecycle and its verification.

## Planned checks

| Requirements | Planned evidence |
| --- | --- |
| R1 | Held real decoder; controller, MarkdownView and QQmlEngine destruction returns before release; detached completion emits no publication and accesses no destroyed object. |
| R2 | Replacement and new-controller decode complete while one stale decoder is held; two held decoders prevent a third from entering; measured maximum concurrency is two. |
| R3 | Ordered dispatch with two occupied slots; queued cancellation and repeated replacements never invoke obsolete decoders; destroying one controller preserves another's queued/active work. |
| R4 | Queued deadline expiry finishes without decode; late results cannot publish; existing replacement/policy/base URL/stale completion/order/limits/network cancellation checks pass. |
| R5 | Separate application-shutdown process proves queued cancellation and drain of active workers before scheduler state destruction, using gates and bounded failure cleanup. |
| R6 | Public/dependency/version diff review; Qt 6.8 build; current lifecycle/coverage docs link actual evidence and preserve historical records. |
| R7 | Focused resource/view tests, python3 scripts/verify-commonmark.py --strict, shared/static CTest, task ci-6.8, task ci-6.11 and git diff --check. Record exact commands, counts, exit codes, environments, artifact locations and limitations. |

## Actual results (2026-10-09)

Explicit user approval was recorded before production edits. Implemented the private application-owned scheduler and controller lifecycle integration with unchanged public interfaces, dependencies and package/module 0.7.0/0.7. The job's QPromise/QFuture cancellation state supplies thread-safe cancellation; no separate cancellation flag is needed.

### Focused host checks

Built the existing shared configuration with `cmake --build build-shared --parallel 2`. Final focused commands:

```sh
build-shared/tests/qmarkdown-resource-test activeDecodeDestruction freshDecodeBesideStale queuedCancellationAndFifo queuedDeadline queuedExpiryAtDispatch expiredDecodeCannotPublish applicationShutdownDrain staleDecoderCompletion -o /tmp/020-focused-resources.txt,txt
QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/tests/qmarkdown-view-test activeDecodeTeardown staleImageCompletionPresentation imageRowsAndPolicyLifecycle
```

Resources: **14 passed**, zero failed/skipped, exit 0 (1.333 s). Native view: **7 passed**, zero failed/skipped, exit 0 (0.253 s). Totals include init/cleanup.

| Requirement | Actual evidence |
| --- | --- |
| R1 | `activeDecodeDestruction` and `activeDecodeTeardown:view,engine` return in less than one second while the held decoder has not completed; explicit release then completes real PNG decoding without publication. The engine row explicitly parents the host-created view to the engine and proves its controller is destroyed. |
| R2 | `freshDecodeBesideStale:replacement,new-controller` publishes fresh data before releasing stale decoding; the new-controller row destroys the stale controller first. `queuedCancellationAndFifo` occupies both workers and proves a third decoder cannot enter during a bounded gate probe. |
| R3 | `queuedCancellationAndFifo` releases only the second slot: third, fourth and latest replacement enter in FIFO order while the first remains held. A destroyed queued controller and 30 replaced queued jobs never decode. Weak references prove all obsolete callable storage is released before either occupied worker is released. Other controllers still publish their images. |
| R4 | `queuedDeadline` uses the private 100 ms fixture timeout and finishes without decoder entry while both workers remain held. `queuedExpiryAtDispatch` independently exercises expired dispatch without controller timer cancellation; its future finishes with no result. `expiredDecodeCannotPublish` proves cancellation during held decoding and no late image admission. Existing four stale-generation rows and native replacement/revocation/arrival cases pass. The unchanged production admission timeout is 15,000 ms. |
| R5 | `applicationShutdownDrain` runs a separate application process with two held real decoders and a queued job. Queue cancellation/finish produces a synchronization marker; the process remains alive until parent release. Application destruction then drains both workers, suppresses their results and never invokes the queued decoder. Parent process cleanup and decoder gate timeouts bound failure paths. |

During authoring, two compilation errors were corrected: include QFuture explicitly and make the child helper's captured future mutable for waitForFinished(). No failing final assertion remains. Gate cleanup keeps callable state alive independently of controller lifetime; expired/cancelled result access is guarded.

### Host regression suites

`python3 scripts/verify-commonmark.py --strict --report /tmp/020-strict.json`: **exit 0**, 652 parser passes and 652 model passes, zero mismatches/uncheckable examples or unresolved comparison limits/semantic losses.

`ctest --test-dir build-shared --output-on-failure`: **7/7 passed**, exit 0, 25.01 s. After `cmake --build build-static --parallel 2`, `ctest --test-dir build-static --output-on-failure`: **7/7 passed**, exit 0, 24.42 s. Both full resource suites record **27 passed** and both native suites **50 passed**, zero failures/skips, including init/cleanup. Existing document-order budgets, resource permissions/limits, network cancellation and presentation checks all pass. Logs remain in each build's `Testing/Temporary/LastTest.log`; corpus reports remain under `tests/commonmark-report.json`.

Host: Qt 6.12.0, GCC 16.2.1, Linux offscreen, bundled cmark 0.31.2. Malformed PNG rejection emits the existing libpng Read Error diagnostic; suites pass.

### Pinned local CI

Sandbox attempts could not access the Docker socket and returned Task exit 201 / runner preparation exit 1. Evidence: `build-ci/6.8.0/20261009T175736Z-mazrux91/` and `build-ci/6.11.3/20261009T175736Z-9082h0mz/`. Approved escalated reruns reused Feature 018's cached toolchains and fresh source/build snapshots:

| Task | Result | Shared/static CTest | Elapsed seconds | Evidence directory |
| --- | --- | --- | ---: | --- |
| `task ci-6.8` | Passed, exit 0; Qt 6.8.0 | 7/7 each; 24.06/23.93 s | 119.625 | `build-ci/6.8.0/20261009T175940Z-x7v6pd_5/` |
| `task ci-6.11` | Passed, exit 0; Qt 6.11.3 | 7/7 each; 24.13/24.03 s | 124.127 | `build-ci/6.11.3/20261009T175943Z-1k3tn24o/` |

All four CI native suites record **50 passed** and resource suites **27 passed**, zero failures/skips, including init/cleanup. Both tasks pass shared/static QML lint, parser symbol privacy, installed/relocated direct-link/plugin-only/static consumers, license notices, library-only builds and validated Release benchmark smoke output. Each run retains `environment.json`, `result.json`, `tool-versions.txt`, `ci.log`, `packaging/`, `benchmarks/` and the read-only source snapshot. Toolchain preparation took 0.012 s each; verification took 119.595/124.096 s. Resolved immutable image IDs and recipe fingerprints are retained in result.json.

Environment: pinned Ubuntu 24.04 amd64, GCC 13.3.0, CMake 3.28.3, Python 3.12.3, offscreen/Fusion, UID/GID 1000 and bundled cmark 0.31.2. Existing Qt 6.8 QML type-resolution warnings, Qt 6.11 optional Qt6TaskTree discovery warnings and unused-import information remain; Vulkan headers are unavailable. These checks pass; warning-free builds are not claimed.

### Final review

Production sources and both updated test files match both tested CI source snapshots byte for byte; final prose records were completed after snapshots. Shared/static corpus reports in both CI runs retain 652 parser/model passes with no unresolved mismatches, uncheckable examples, comparison limits or semantic losses.

Changed files: private decodescheduler.h/.cpp, resourcecontroller.h/.cpp, library source integration in src/QMarkdown/CMakeLists.txt, tests/tst_resources.cpp, tests/tst_view.cpp, README.md, specs/README.md, the current iteration 019 coverage assessment and all four iteration 020 documents. Iteration 019 historical requirements/design/tasks/verification and all other historical results are unchanged. Public headers/QML interfaces, dependencies, lockfiles and package/module versions remain unchanged.

`git diff --check`, local Markdown link checks and final scope/source review pass. All authorized tasks are complete.

### Limits

Linux offscreen evidence covers representative native behavior; other operating systems, GPU output, accessibility and hosted workflow execution remain unverified. The engine teardown fixture gives the engine explicit ownership of its host-created root; QQmlComponent roots are not implicitly engine-owned. Decoder execution cannot be interrupted, two held codecs can delay fresh jobs and application teardown may wait indefinitely for a codec that never returns. Decoder-library unloading during active work remains outside scope. No sanitizer run or cross-platform guarantee is claimed. Release publication and version changes are excluded.

