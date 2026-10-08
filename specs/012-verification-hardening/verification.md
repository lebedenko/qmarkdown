# Verification

Feature 012 was approved on 2026-10-08 through the explicit implementation request. Versions remain 0.7.0/0.7. Planned checks are R1/T1 packaging/CI, R2/T2 independent oracle faults/corpus, R3/T3 controlled lifecycle ordering, and R4/T4 opt-in benchmark evidence and exclusion checks.

## Actual local checks (2026-10-08)

Environment: Arch Linux x86_64, Qt 6.12.0, GCC 16.2.1 (20260810), CMake 4.4.4, shell Python 3.14.7; CMake's test Python was 3.12.13. GUI checks used offscreen. No project dependency downloads occurred.

- `python3 tests/commonmark/test_baseline.py --probe build-shared/tests/qmarkdown-commonmark-probe`: **37 tests passed**, including exact authored IDs/checksum/review/model validation and missing-expectation, literal-text, boundary, nesting, metadata, comparison-method and stale-ledger faults.
- `python3 scripts/verify-commonmark.py --probe build-shared/tests/qmarkdown-commonmark-probe --report /tmp/commonmark-012-final.json`: **652 parser passes, 626 model passes, zero mismatches, 26 uncheckable**. 583 passes use HTML projection; 43 compare entire source-authored raw models. There are 250 examples with comparison limits and 113 with semantic information loss.
- The same runner with `--strict --report /tmp/commonmark-012-strict.json`: **exit 1 as required** for remaining evidence limits. Baseline success does not establish full conformance.
- A per-ID ledger audit confirmed only IDs 148–167 and 169–191 were removed, every retained entry is unchanged, and all 43 production raw models retain their historical fingerprints. Complete independent expectations, rather than those hashes, now decide these passes. Each removed exception has a rule/boundary review in the authored fixture. No production defect was established. The initial authored expectation for 148 incorrectly treated closing `pre` as a paragraph-interrupting type 6 tag; review of §4.6 corrected it to inline literal type 7, without changing production code or accepting a mismatch.
- `cmake --build build-shared --target qmarkdown-resource-test --parallel 2`, then 20 bounded subprocess invocations of `build-shared/tests/qmarkdown-resource-test staleDecoderCompletion pendingNetworkDestruction activeDecodeDestruction`: **all passed**. Each run covers replacement, policy revocation/restoration, base URL changes, five repeated active cancellation/success cycles, pending-network destruction and active-decode destruction. The destructor release thread observes watcher destruction during cancellation before releasing the real decoder. Semaphore waits are bounded and cleanup always releases gates. No sleep-based stale-completion race remains.
- `python3 scripts/verify-packaging.py --qt-root /usr --work-dir /tmp/qmarkdown-012-packaging-final > /tmp/qmarkdown-012-packaging-final.log 2>&1`: **passed**. Each shared/static variant passed all six CTest entries, `all_qmllint`, 140-symbol cmark prefix/privacy checks, installation, relocation metadata/notices and copied installed consumers. Shared direct-link and plugin-only consumers both ran; static used custom `share/qml` and no QMarkdown runtime import path. Every configuration selected `/usr/lib/cmake/Qt6` explicitly. The tests/examples/benchmarks-disabled library-only build passed. Benchmark defaults are OFF in all three caches. Lint retained the existing informational unused import in the import-only example.
- After strengthening the repeated-generation scenario, rebuilt each packaging resource target and ran `ctest --test-dir <build-shared/build-static> -R qmarkdown-resources --output-on-failure`: **both passed**, about 16.5 seconds each including the existing 15-second deadline check.
- `cmake -S . -B /tmp/qmarkdown-012-invalid-benchmark -DBUILD_TESTING=OFF -DQMARKDOWN_BUILD_EXAMPLES=OFF -DQMARKDOWN_BUILD_BENCHMARKS=ON`: **failed as required** with the explicit test-dependency diagnostic.
- Release benchmark build configured with `-DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DQMARKDOWN_BUILD_EXAMPLES=OFF -DQMARKDOWN_BUILD_BENCHMARKS=ON`. The standalone target builds; it is absent from CTest and installed CMake exports. `cmake --build build-benchmarks --target QMarkdownPlugin --parallel 2` and `cmake --install build-benchmarks --prefix /tmp/qmarkdown-012-benchmark-install` passed; no benchmark executable was installed.
- `QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-benchmarks/qml" timeout 120 build-benchmarks/tests/qmarkdown-benchmark --smoke > /tmp/qmarkdown-benchmark-smoke-final.json`, followed by `python3 scripts/verify-benchmark.py /tmp/qmarkdown-benchmark-smoke-final.json`: **passed**. Five controlled JSON faults (missing workload, duration summary, reset count, duplicate workload and substituted font) were rejected.

## Full Release measurements

`QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-benchmarks/qml" timeout 600 build-benchmarks/tests/qmarkdown-benchmark > /tmp/qmarkdown-benchmark-full-final.json`, then `python3 scripts/verify-benchmark.py /tmp/qmarkdown-benchmark-full-final.json`: **passed**, bounded by the 600-second execution timeout. Two warmups and ten measurements completed for every workload. Both Noto families resolved exactly as requested at 16 pixels and 480-pixel width. Full run durations/counts and environment are retained in [benchmark-linux-qt6.12-release.json](benchmark-linux-qt6.12-release.json). Timing is advisory; results include observation and event-processing overhead.

| Operation | Source bytes | Images | Median ms | Maximum ms | Qt Quick items | Root resets |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| parse-mixed | 1024 | 0 | 0.019 | 0.029 | 0 | 0 |
| replace-mixed | 1024 | 0 | 0.005 | 0.006 | 0 | 1 |
| layout-mixed | 1024 | 0 | 4.573 | 5.308 | 135 | 1 |
| parse-mixed | 10240 | 0 | 0.198 | 0.205 | 0 | 0 |
| replace-mixed | 10240 | 0 | 0.064 | 0.067 | 0 | 1 |
| layout-mixed | 10240 | 0 | 55.464 | 61.174 | 1435 | 1 |
| parse-mixed | 102400 | 0 | 2.638 | 2.869 | 0 | 0 |
| replace-mixed | 102400 | 0 | 0.777 | 0.855 | 0 | 1 |
| layout-mixed | 102400 | 0 | 734.691 | 763.642 | 14461 | 1 |
| parse-mixed | 1048576 | 0 | 62.477 | 64.259 | 0 | 0 |
| replace-mixed | 1048576 | 0 | 21.950 | 22.294 | 0 | 1 |
| layout-mixed | 1048576 | 0 | 9997.377 | 10263.665 | 148153 | 1 |
| parse-dense | 10240 | 0 | 3.945 | 4.076 | 0 | 0 |
| replace-dense | 10240 | 0 | 0.000 | 0.000 | 0 | 1 |
| layout-dense | 10240 | 0 | 4105.987 | 4599.670 | 6 | 1 |
| sequential-local-images | 51 | 1 | 0.417 | 0.569 | 9 | 2 |
| sequential-local-images | 408 | 8 | 4.631 | 5.304 | 44 | 9 |
| sequential-local-images | 1654 | 32 | 60.834 | 62.041 | 164 | 33 |

Counts were identical across all ten measured runs for each workload. Initial smoke/full output exposed a DejaVu-to-Noto font substitution; that evidence was superseded. The executable now requires Noto Sans and Noto Sans Mono explicitly and rejects substitution; CI installs them. No production rendering optimization was made in response to timings.

## Initial CI review before remote execution

Workflow YAML structure, triggers, Qt matrix and direct 40-character action pins were validated locally. Actionlint is not installed, so that check was unavailable. No GitHub workflow was dispatched or repository metadata changed; Ubuntu 24.04 / Qt 6.8.0 and 6.11.3 compatibility remains **pending CI execution**. A successful local Qt 6.12.0 run cannot establish those targets. Incompatibilities must be reported without weakening tests or silently raising the minimum.

The [official SDK directory](https://download.qt.io/online/qtsdkrepository/linux_x64/desktop/) lists `qt6_680` and `qt6_6113`. Read-only GitHub tag/source inspection confirmed checkout v4.2.2 (`11bd71901bbe5b1630ceea73d27597364c9af683`), upload-artifact v4.6.2 (`ea165f8d65b6e75b540449e92b4886f43607fa02`) and install-qt-action v4.1.1 (`c6c7281365daef91a238e1c2ddce4eaa94a2991d`). The [Qt installer descriptor](https://github.com/jurplel/install-qt-action/blob/c6c7281365daef91a238e1c2ddce4eaa94a2991d/action/action.yml) is a Node20 sub-action; invoking it directly avoids the composite wrapper's mutable nested action references. CI provisions Ubuntu build/runtime dependencies and Noto fonts explicitly, disables the action's implicit apt installation, and pins aqtinstall 3.3.0 / py7zr 0.22.0. Qt is selected explicitly for packaging, consumers, library-only and benchmark configurations. Failure artifacts retain CTest logs, CommonMark reports and configure evidence.

## CI failure follow-up (2026-10-09)

The user explicitly requested fixes after inspecting [run 37835853141](https://github.com/lebedenko/qmarkdown/actions/runs/37835853141), commit `210497e7ed1f5be66c7d39adbfe68ebecddd6068`. Both Ubuntu jobs compiled successfully and failed in shared-build `qmarkdown-view`, before static packaging or benchmarks could run.

- Both versions reported heights 544 versus 499 in `pointRenderingAndRuntimeUnits`: the test removed italic ranges from only one view for a native-text comparison and did not restore them before comparing shared-style views. Restore those ranges before subsequent comparisons.
- Qt 6.8.0 reported Fusion SpinBox text-binding loops. Its SpinBox calls validator `fixup()` from the content item's text-change handler, rewriting display text during binding evaluation. The viewer's size validator now retains QDoubleValidator range/precision checks and the control's locale, uses standard notation, and leaves text normalization to the SpinBox formatter. This helper is confined to the existing viewer tools module; the library API and renderer are unchanged.
- The preview-clear failure followed the warnings: QtTest skips asynchronous retry waits once the test has failed. Clearing passes when the warnings are fixed; no production document-clearing change was needed.
- The divider test's fixed offset misses Fusion's two-pixel handle. Derive the drag point from the gap between the source and preview panes instead. Fractional size editing now uses real keyboard input rather than setting a value and emitting a user signal manually.
- Follow-up packaging reached a separate SDK-selection defect: consumer configuration overwrote `CMAKE_PREFIX_PATH` with the relocated package alone. On a host with system Qt 6.12, Qt 6.8 then selected incompatible system CoreTools. Both direct and plugin-only consumer configurations now search the relocated package and the explicit SDK together; `Qt6_DIR` stays explicit.
- The installed Qt 6.8 consumer then returned 26 because its manually delivered mouse events did not exercise the platform event path used by native input. The probe now waits for linked-text layout, clicks within the rendered line, and uses `QTest::mouseClick`, as the existing interaction suite does. Qt Test is a dependency of this test driver only, not of the installed library or its CMake exports.

Local verification uses official Linux Qt 6.8.0 and 6.11.3 SDKs downloaded under `/tmp/qmarkdown-ci-qt`, on Arch Linux with GCC 16.2.1. Controls use Fusion and rendering uses offscreen, matching the failing CI control style. This is local SDK evidence, not a successful Ubuntu Actions rerun.

- Focused `pointRenderingAndRuntimeUnits viewerTheme viewerFontUnits viewer` runs: **all four scenarios passed on both SDKs**, including actual fractional keyboard entry, unit changes, preview clearing and divider dragging.
- Release benchmark `--smoke` runs and `scripts/verify-benchmark.py`: **passed on both SDKs**.
- Final `QT_QUICK_CONTROLS_STYLE=Fusion python3 scripts/verify-packaging.py --qt-root /tmp/qmarkdown-ci-qt/<version>/gcc_64 --work-dir /tmp/qmarkdown-ci-packaging-final-<version>` runs: **both passed**, using version/directory pairs `6.8.0`/`6.8` and `6.11.3`/`6.11`. Each shared/static variant passed all **seven CTest entries**, `all_qmllint`, 140-symbol parser privacy checks, install/relocation metadata and notices, and installed consumers. Shared direct-link and plugin-only consumers passed; static consumers passed with the custom `share/qml` directory. Both library-only builds passed. Logs are `/tmp/qmarkdown-ci-packaging-final-6.8.log` and `/tmp/qmarkdown-ci-packaging-final-6.11.log`.
- Qt 6.8 `all_qmllint` exits zero with native type-resolution warnings involving QQuickPaintedItem/private render types and QColor metadata; this is not a warning-free lint claim. Qt 6.11 retains the informational unused import in the import-only example.
- `python3 -m py_compile scripts/verify-packaging.py` and `git diff --check`: **passed**. Fixes remain local; no commit, push or remote rerun was performed.

## Limits

The worker pool remains single-threaded. Restart cancels publication but an active decode continues; new work waits behind it and destruction waits for it. These are responsiveness implications, not improvements in this iteration. Benchmarks count root model resets and all Qt Quick scene items, including structural items. Sequential image evidence uses real authorized local reads/decoding, controlled completion gates and the production image projection/model replacement path. It does not claim network, GPU, allocation or streaming performance. No optimization or automatic timing thresholds were introduced.

Final source checks: workflow/Task YAML inspection, Python byte compilation and `git diff --check` passed. No commits or releases were created.
