# Verification — QML responsibilities and private contracts

Status: approved on 2026-10-15; implementation and local verification complete.

## Planned checks

- V1: Range queries: boundary exclusion, clipping, cross-block matches, UTF-16
  surrogate offsets, empty ranges, image descriptions, current-result changes with
  unchanged counts and query/source resets. Assert returned ranges and flags.
- V2: Registration metadata updates, item/coordinator replacement, helper/item/
  coordinator destruction, and exclusion-only table/action registrations. Check
  observable selection/link/navigation behavior and absence of stale entries.
- V3: Nested lists, quotes, tables and images; heading decoration suppression,
  style replacement, zero/positive width transitions, selection/search toggles and
  selection painted above search. Preserve text geometry and copied projection.
- V4: Existing view, styling, selection, reading and renderer regressions plus
  QML lint; check native Text fast path and recursive loading.
- V5: Existing renderer/action lifecycle tests including synchronous replacement,
  stale callbacks, invalid renderer fallback, creation-context bindings, source
  changes, selection/search fallback and language extraction compatibility.
- V6: Advisory activation/next-result benchmarks varying block and match counts;
  retain pre-change and post-change results/environment with no timing test gates.
- V7: Narrow checks first, then relevant complete shared/static suites on configured
  Qt 6.8.0 and 6.12.0 using repository CI tooling; validate relocated installed
  linked/plugin-only/static consumers without source-tree imports, source archives,
  payload inventories and parser/conformance regressions.
- V8: Documentation links/consistency and whitespace; review unchanged public API,
  package/QML versions and dependencies. Required hosted checks remain a PR gate.

## Actual results

### Focused and host checks — 2026-10-15

- Approval of requirements R1–R5, design and T1–T7 was recorded before runtime edits.
- Added bounded-range cases for half-open boundaries, clipping, cross-block
  matches, UTF-16 emoji offsets, empty/negative/overflowing queries, image
  descriptions and current-result revision changes with unchanged result counts.
- Added registration completion, metadata mutation, dependency replacement,
  helper/item/coordinator destruction and table/action exclusion regressions.
- Renderer projection tests cover ECMAScript fence whitespace including BOM,
  NBSP and line separators. Existing synchronous replacement, stale-copy,
  renderer fallback, style/geometry and creation-context cases pass on the host.
- Existing view geometry assertions now find block boundaries independently of
  text wrapper/Loader depth. The full host Qt 6.12.0 suite passed 18/18 tests
  (`ctest --test-dir build-shared --output-on-failure`, 53.05 seconds). The added
  incompatible-table-sequence test passed separately after this complete run.
- Host `all_qmllint` passed with only the existing import-only example's unused
  import informational message. Documentation validation and whitespace checks
  passed on final evidence prose.
- The pre-existing root CMakeLists.txt QTP0004 policy change was preserved. Local
  verification snapshots include it; it is not part of this feature's edits.

Initial focused checks caught prohibited self-instantiation of the extracted
BlockSequence, unsettled source identity in copy actions, stale pressed links and
geometry tests coupled to old parent depth. These were corrected. A new plain
text/HTML Find-mode regression reproduced an existing DelegateChooser role-change
limitation; stable render-kind dispatch with explicit text-mode switching makes
both paths searchable on supported Qt versions. Qt 6.8 additionally elided an
unused local revision read; using the revision in the binding result fixed stale
text/image ranges. The focused Qt 6.8 reading test then passed before full matrices
were repeated.

### Configured packaging matrices

Final runs use `python3 scripts/run-ci.py --qt-role ROLE --artifact-dir DIRECTORY`.
Both final `result.json` files record `status: passed`, `exitCode: 0`:

| Qt | Evidence directory | Shared suite | Static suite |
| --- | --- | --- | --- |
| 6.8.0 | `build-ci-046-minimum-validated` | 18/18 passed | 18/18 passed |
| 6.12.0 | `build-ci-046-development-validated` | 18/18 passed | 18/18 passed |

Both matrices passed QML lint, parser symbol isolation, relocated linked/plugin-only/
static installed consumers without source-tree imports, complete QML payload checks,
package-version compatibility and advisory benchmark smoke/JSON validation. Each
CommonMark report records 652/652 parser and model projection passes, with zero
mismatches, losses or example-level limits. Existing GFM, styling, rendering,
selection, renderer lifecycle and reading regressions passed in the full suites.
Minimum Qt lint retains existing unresolved native-type metadata diagnostics;
commands completed successfully. Development configuration retains the existing
optional Qt6TaskTree dependency warning from the SDK.

Intermediate `*-verified`, `*-final` and minimum `*-complete` runs caught the
regressions described above; they are superseded by the final validated runs.
Development `*-complete` passed before the last revision-binding fix, and both
matrices were repeated after that runtime input changed. The first minimum attempt
could not access Docker in the sandbox; Docker access was approved through tool
escalation. Only specification/evidence content changed after final snapshots.

### Source archive

The source archive API `build_archive(Path.cwd(), output)` was used in working-tree
mode to include uncommitted new files. The final archive built and installed with
host Qt 6.12.0, including all new private QML payload checks, in
`build-archive-046-validated/verification` (log `/tmp/046-archive-validated.log`).
An earlier working-tree archive also passed its installed core consumer with only
its installed QML import path (`/tmp/046-archive-consumer.log`).

An initial CLI invocation defaulted to HEAD and checked the previous release
archive; its missing-new-payload failure is superseded by the working-tree checks.
No publication artifact or interactive desktop test is claimed.

### Advisory search benchmark

The benchmark's `--quick` comparison varies 25/100 formatted blocks and one/four
matches per block, uses one warmup and three samples, and reports median activation
and next-result costs (three next actions per sample). Its default workload also
supports 100/500 blocks and one/eight matches with five samples and ten actions.
It validates searchable delegate and result counts, processes deferred
layout/navigation and uses an offscreen window. Formatted blocks ensure the
pre-change workload includes actual highlight bindings; the initial plain-text
trial exposed the mode-transition limitation above and was discarded. The large
baseline was stopped after proving impractically slow; the smaller comparison uses
identical workloads/harness for both revisions. Timing is advisory with no gates.

Baseline uses commit `0aabcd1` extracted under `/tmp`, the same benchmark harness,
host Qt 6.12.0 and the same build configuration as the local post-change run.
Completed data is retained in [search-before.json](search-before.json) and
[search-after.json](search-after.json). Both use unspecified CMake build type,
GCC 16.2.1, x86_64 and Qt 6.12.0 on the same host; runs were sequential. Ordinary
host/test activity can affect these advisory timings.

| Blocks | Matches | Activation before → after (ms) | Next result before → after (ms) |
| --- | --- | --- | --- |
| 25 | 25 | 9.16 → 6.04 | 9.71 → 5.98 |
| 25 | 100 | 23.22 → 9.06 | 23.62 → 8.83 |
| 100 | 100 | 93.05 → 24.55 | 93.45 → 24.26 |
| 100 | 400 | 303.65 → 38.32 | 310.84 → 37.27 |

### Changed files and review scope

- `src/QMarkdown/MarkdownView.qml` retains the facade and gestures. New private
  `BlockSequence.qml`, `TextBlock.qml`, `SelectableText.qml`, `ImageBlock.qml`,
  `ListBlock.qml` and `HeadingSeparator.qml` own presentation. Existing
  `CodeSurface.qml`, `CodeActionHost.qml` and `TableBlock.qml` use explicit contracts.
- Private `viewstate.h/.cpp`, `presentation.h/.cpp`, new
  `selectionregistration.h/.cpp` and `private.qmltypes` implement bounded search,
  fence-language projection, typed lifetime/selection metadata and tooling.
- `src/QMarkdown/CMakeLists.txt`, `scripts/release/source-files.txt`,
  `scripts/verify-packaging.py` and `scripts/verify-source-archive.py` cover resources,
  installation, source inventory and payload checks together.
- `tests/tst_reading.cpp`, `tst_selection.cpp`, `tst_renderers.cpp`, `tst_view.cpp`,
  `installed-consumer/main.cpp`, new `searchbenchmark.cpp` and `tests/CMakeLists.txt`
  cover the new contracts and observable behavior across the extracted hierarchy.
- `specs/README.md` and feature 046 requirements, design, tasks, this verification
  record and benchmark JSON retain approval, scope and actual evidence.

Public APIs, package/QML versions, dependencies, Layout.js and viewer code are
unchanged. The existing root CMakeLists.txt policy edit remains separate. Changes
are prepared as one focused review unit. Merge and publication require separate
authorization; required hosted checks remain a delivery gate.
