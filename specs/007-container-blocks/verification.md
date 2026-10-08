# 007: Verification

**Approval:** On 2026-10-08 the user explicitly requested implementation of the supplied Feature 007 plan. Approval covers these requirements, design, tasks, and package 0.5.0 / module 0.5.

## Planned checks

- V1 / R1,R2: Container markers, starts/delimiters, nesting, tight/loose, lazy continuation, tabs/interruption/precedence and contained leaves; retain leaf/normalization/fence checks; breaks, references, labels and HTML.
- V2 / R3,R4: Numbering, gutters, geometry, wrapping, empties/narrow widths, exact spacing/height, live styles, clear/replacement and width recovery; inert resources.
- V3 / R5: Playground controls/reset/presets and light/dark captures inspected.
- V4 / R5: Shared/static CTest, lint, symbol privacy and relocated linked/plugin-only/static consumers.
- V5: Diff review; no bundled cmark changes; actual results recorded below.

## Actual results

Implemented and verified on 2026-10-08 for package 0.5.0 / QML module 0.5. No dependencies added; third_party/cmark and historical feature records are unchanged.

Environment: Linux x86_64, Qt/Qt Test 6.12.0, GCC 16.2.1, CMake 4.4.4. Minimum Qt 6.8 and other platforms remain unexecuted.

Executed commands:

```sh
cmake --build build-shared --parallel 2
build-shared/tests/qmarkdown-parser-test
QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/tests/qmarkdown-view-test containerLayoutAndStyle leafLayoutAndStyle
ctest --test-dir build-shared --output-on-failure
cmake --build build-shared --target all_qmllint
python3 scripts/verify-cmark-symbols.py build-shared
python3 scripts/verify-packaging.py
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software QML_IMPORT_PATH="$PWD/build-shared/qml" QMARKDOWN_CAPTURE_THEME=/tmp/qmarkdown007 build-shared/tests/qmarkdown-view-test viewerTheme
python3 /tmp/qmarkdown007-refresh-packages.py
git diff --check
```

Results:

- V1: **108 parser checks passed**, zero failures/skips. Authored cases cover all bullet markers and both ordered delimiters, non-one starts, changed markers, empty items/quotes, tight/loose lists (including a loose inner list inside a tight outer list), interruption, tabs, thematic precedence, lazy continuation, nested mixed containers and contained headings/rules/fenced/indented code. Existing leaf, normalization, long-fence and info regressions pass. Inert-label fixtures preserve Unicode range offsets, label emphasis/strong, escapes, URL/email autolinks, literal inline HTML and comments. Reference definitions disappear, missing references stay text, and soft/hard breaks follow the approved migration. Old source-span/conservative-tree tests were removed with the superseded adapter contract.
- V2: Focused container/leaf checks passed. Full suites report **15 view checks**, **3 import checks** and **108 parser checks** for both shared and static variants. Geometry checks cover 9)/10) numbering, widest-marker measurement and live body-font updates, marker top alignment, exact tight/loose and nested-loose item gaps, quote child spacing/inset/rule extent/color, empty container line reservation, narrow-width clamping, suppression/recovery, formatted hard-break layout, sharing/reset/destruction and replacement/clear. Numeric notification idempotence, negative thickness, NaN indent and nonfinite thickness are exercised. Network factory counts stay zero; existing inert activation checks pass. FormattedText already mapped LF to Qt line separators and needed no code change.
- V3: Playground quote color validation, manual override persistence, light/dark palette tracking, preset indent/thickness values and reset pass. Captured and visually inspected `/tmp/qmarkdown007.light.neutral.png`, `.light.alternate.png`, `.dark.neutral.png` and `.dark.alternate.png`. Ordered starts, mixed nesting, wrapped ordered content, italic quote text, hard line breaks, quote rules and body/heading styling render correctly. Controls fit at 1000×700; content below the viewport remains host-scrollable. Simulated light palettes retain the previously recorded platform-button low contrast; desktop/platform theme behavior remains unverified.
- V4: Shared/static CTest **3/3 each**; final lint has no warnings (only the established informational unused import in import-only). Symbol verification confirms **140** prefixed bundled external definitions and no exported cmark symbols/types. Relocated shared linked, shared plugin-only and static consumers pass, including native list markers, nested quote/list text and live quote-rule style edits. Static consumers use relative `share/qml` and no source import path. Packaging artifacts: `/tmp/qmarkdown-packaging-qy1rw0n0`; full initial log `/tmp/qmarkdown007-packaging-final.log`. After the final marker-font fix, `/tmp/qmarkdown007-refresh-packages.py` rebuilt both variants, reran CTest/lint/privacy, refreshed the relocated installations and rebuilt/executed all consumers. Final log: `/tmp/qmarkdown007-packaging-refresh.log`.
- V5: `git diff --check` passed. Public MarkdownView properties remain unchanged; notifying container properties are added only to MarkdownStyle. Package/import/private metadata and current documentation are updated to 0.5.0/0.5. Requirements/design/tasks approval was recorded before implementation.

Intermediate findings resolved during verification: Loader height initially formed a zero-size cycle; letting Loader derive height restored leaf sizing. QML lint required a typed inline BlockSequence and typed Loader access; parenthesized cast assignment triggered a runtime binding error, fixed by a local typed reference. A public std::unique_ptr specialization exposed a cmark type through weak dynamic symbols; private local RAII owners fixed privacy. Final review corrected loose inner-list gaps independently of tight parent gaps and explicitly tracked the font dependency of marker measurements. Regressions pass on the final source.

Intentional upstream limitation: cmark 0.31.2 caps stored opening fence lengths at 255. The retained 300-marker fixture consequently closes on a 299-marker line and creates a final empty fence. Its expectation now records cmark behavior; the bundle was not modified. This prevents a full CommonMark-conformance claim.

Limits: these are authored bounded fixtures, not the official complete CommonMark suite. Resource rendering/navigation, task lists, selection, streaming and extensions remain deferred. Captures use offscreen software rendering; real desktop transitions, other platforms and minimum-toolchain execution were not tested.
