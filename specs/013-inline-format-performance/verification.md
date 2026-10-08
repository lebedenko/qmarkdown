# Verification

Approved on 2026-10-08 by the user’s explicit instruction to implement the supplied Feature 013 plan. Approval covers these requirements, design, tasks and format preparation only; package/module remain 0.7.0/0.7.

Planned checks: T1–T4 in [tasks](tasks.md). All planned local tasks were executed; actual results follow.

## Actual local checks (2026-10-08)

Environment: Arch Linux x86_64, Qt 6.12.0, GCC 16.2.1 (20260810). Shared/static checks use offscreen Qt; Release benchmarks retain Noto Sans/Noto Sans Mono at 16 pixels and a 480-pixel viewport. No dependency changes or downloads.

- Built `qmarkdown-formatintervals-test` and `qmarkdown-view-test` in `build-shared`. Focused helper CTest passed; direct helper execution passed all 14 QtTest cases, including nine data rows, 64 deterministic combinations, explicit adjacent boundary retention, maximum integer offset, invalid wide values and astral UTF-16 coverage.
- Focused renderer invocation `combinedFormatsAndLiveRanges formattedFontAndLayout linkLayout linkStyleLifecycle formattedStyleLifecycle`: all eight QtTest cases passed, including point/pixel link layout. New coverage verifies all three flags with link color/underline simultaneously, then replacement and removal of formatting/link ranges without changing text.
- `cmake --build build-shared --parallel 2` and the corresponding static build passed. `ctest --test-dir build-shared --output-on-failure` and the static counterpart: **7/7 passed in each**, 26.60 / 24.21 seconds. The full view suite includes application typography, native point layout, wrapping/bidirectional links, runtime units/DPI observation and style lifecycle.
- Shared/static `all_qmllint` passed, with the existing informational unused import in the import-only example. Shared/static `scripts/verify-cmark-symbols.py` passed: 140 mapped definitions, no cmark exports.
- CommonMark CTest records: **652 parser passes, 626 model passes, zero mismatches, 26 uncheckable** in each variant. Existing 250 comparison limits and 113 semantic-loss cases remain; no full conformance claim.
- Release targets `qmarkdown-benchmark`, `qmarkdown-inline-benchmark` and `QMarkdownPlugin` built. The first multi-target Make invocation regenerated CMake while resolving the newly added target and reported no rule for it; rerunning after regeneration succeeded without source changes.
- `cmake --install build-benchmarks --prefix /tmp/qmarkdown-013-benchmark-install` passed. Recursive installed-file and CMake-export checks found neither benchmark executable/target nor helper header; `nm -D --defined-only` confirmed the preparation helper is hidden. Both benchmark targets remain outside CTest and default OFF under the existing option.
- Python validator syntax passed. A valid synthetic preparation report passed; seven controlled faults (missing/duplicate workload, wrong median, output count, equivalence, link count and nonfinite durations) were rejected.

## Performance evidence

The unchanged Release benchmark was executed before rebuilding production, using `QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-benchmarks/qml" timeout 600 build-benchmarks/tests/qmarkdown-benchmark > /tmp/qmarkdown-013-before.json`; `scripts/verify-benchmark.py` passed. The after run used the same command with output `/tmp/qmarkdown-013-after.json`; both full reports passed `scripts/verify-benchmark.py`. Every environment/layout field was compared and matched. Builds/tests had completed before the after run; preparation benchmarking runs separately. Retained evidence: [before](benchmark-before-linux-qt6.12-release.json) and [after](benchmark-after-linux-qt6.12-release.json).

| Dense layout (10,240 source bytes) | Median ms | Maximum ms |
| --- | ---: | ---: |
| Previous preparation | 4326.618 | 5817.122 |
| Event sweep | 84.428 | 88.157 |

Median improved by approximately **51.25× (98.05% lower)**, satisfying dense-layout acceptance. Full workload/count samples and exact maxima are in the JSON records. `timeout 1800 build-benchmarks/tests/qmarkdown-inline-benchmark > /tmp/qmarkdown-013-inline.json` completed successfully; `python3 scripts/verify-inline-benchmark.py /tmp/qmarkdown-013-inline.json` passed. [Full preparation evidence](inline-benchmark-linux-qt6.12-release.json) retains all ten samples, maxima, environment, span/text/output counts, warmups and run counts for all 16 measurements. Workload construction and expected-output generation are outside timed sections; initial and every measured/warmup output matched the retained previous algorithm exactly. Link cases contain the same number of links as formatting spans, each covering up to three adjacent formats.

| Workload | Formatting spans | Previous median ms | Sweep median ms | Output intervals |
| --- | ---: | ---: | ---: | ---: |
| format-only | 1,024 | 73.623 | 0.155 | 1,024 |
| format-only | 2,048 | 296.143 | 0.320 | 2,048 |
| format-only | 4,096 | 1243.247 | 0.621 | 4,096 |
| format-only | 8,192 | 4971.676 | 1.827 | 8,192 |
| overlapping-links | 1,024 | 146.424 | 0.322 | 1,024 |
| overlapping-links | 2,048 | 587.940 | 0.677 | 2,048 |
| overlapping-links | 4,096 | 2368.728 | 1.396 | 4,096 |
| overlapping-links | 8,192 | 11945.320 | 3.094 | 8,192 |

For an 8× increase in formatting spans, sweep medians grew **11.78× / 9.62×** (format-only / overlapping links), versus **67.53× / 81.58×** for the old scan. This supports the O(S log S) preparation design, with substantial sample variance in the largest old workload; no timing threshold is imposed on CI. Storage consists of at most two events per valid span and one interval per distinct boundary pair, hence O(S).

Acceptance is met: equivalent valid-span outputs, independent invalid/UTF-16/overlap correctness checks, expected sweep scaling and lower full dense-layout median. Native font resolution, shaping, wrapping, format construction, ink-bound calculation and event/polish settling remain in the end-to-end 84 ms dense result. The isolated benchmark does not profile those costs or imply caching/virtualization improvements.


## CI and scope limits

No GitHub workflow was dispatched; Qt 6.8.0 / 6.11.3 compatibility remains pending CI execution as in Feature 012. Local Qt 6.12.0 verification does not establish those targets. Font resolution, native text shaping/layout, raster bounds and painting remain in the renderer; caching, virtualization, parser/image changes and link hit-testing optimization remain deferred. Package/module stay 0.7.0/0.7.
