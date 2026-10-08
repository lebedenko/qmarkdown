# 006: Verification

**Approval:** The user explicitly instructed implementation of the supplied Feature 006 plan on 2026-10-08. Approval covers R1–R5, the design below, tasks T1–T6, and delivery as package 0.4.0 / QML module 0.4.

## Planned checks

- **V1 / R1,R2,R5:** Setext levels, multiline/formatting, malformed underlines, indentation, precedence; all thematic markers, paragraph interruption, adjacent blocks; retain fence regressions.
- **V2 / R3,R5:** Four columns/tabs, paragraph non-interruption, literal resources/Markdown, whitespace/blank buffering, EOF and normalization.
- **V3 / R1–R4:** Native hidden delimiters, heading/code styles, rule geometry/live edits, exact gaps, wrapping/height, replacement/clear and nonpositive-width recovery after polish.
- **V4 / R4,R5:** Notifications, NaN equality, reset/sharing/replacement/destruction; playground light/dark palettes, presets and overrides; zero resource activity.
- **V5 / R5:** Focused parser/view first; shared/static CTest, lint, symbol privacy, relocated linked/plugin-only/static consumers.
- **V6 / R4:** Capture/inspect light/dark playground leaf samples; record environmental limits and preserve historical records.

## Actual results

Implemented and verified on 2026-10-08 for the approved bounded slice. Package 0.4.0 / module 0.4. No cmark change or dependency added. Feature 001–005 historical verification records remain unchanged.

Environment: Linux x86_64, Qt/Qt Test 6.12.0, GCC 16.2.1, CMake 4.4.4. Minimum Qt 6.8 and CMake 3.21 execution remains unverified.

Commands (focused checks preceded broad verification):

```sh
cmake -S . -B build-shared -DCMAKE_INSTALL_PREFIX=/tmp/qmarkdown-install
cmake --build build-shared --parallel 2
build-shared/tests/qmarkdown-parser-test
QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/tests/qmarkdown-view-test leafLayoutAndStyle
ctest --test-dir build-shared --output-on-failure
cmake --build build-shared --target all_qmllint
python3 scripts/verify-cmark-symbols.py build-shared
python3 scripts/verify-packaging.py > /tmp/qmarkdown-leaf-packaging.log 2>&1
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software QML_IMPORT_PATH="$PWD/build-shared/qml" QMARKDOWN_CAPTURE_THEME=/tmp/qmarkdown-leaf build-shared/tests/qmarkdown-view-test viewerTheme
git diff --check
```

Results:

- V1/V2: parser **109 passed**, zero failures/skips, including existing fences >255 markers, info decoding and inline regressions. Authored new fixtures cover both Setext levels, malformed/indented underlines, multiline/formatted content, all rule markers, precedence/interruption, adjacent transitions, code whitespace/blank handling, EOF, CRLF/CR/NUL/Unicode and literal resource syntax. Superseded four-column heading/fence fallback and Setext paragraph expectations were deliberately updated; ordinary normalization now begins within three columns.
- V3/V4: focused leaf layout/style **3 passed** including init/cleanup. Full view suite **14 passed**, import suite **3 passed**. Native checks cover hidden delimiters, heading/code fonts, literal resources and zero requests/activation, full-width rules, fractional/negative/zero/infinite/NaN thickness, idempotent notifications, shared styles, replacement/destruction/default reset, wrapping, exact participating-block gaps, content height, replacement/clear and width recovery after polish. Playground checks cover rule palette tracking, invalid/manual colors, alternate light/dark accents, thickness presets/overrides and reset. Existing style/fence regressions pass.
- Initial focused leaf test and the first shared CTest run failed because the new test assumed zero-height items participate in Column positioning. Qt Column skips them, including their spacing. The test now verifies that established behavior (normal 8-pixel gaps between participating blocks); documentation explicitly records it. No renderer layout change was needed. Final focused and shared/static suites pass.
- V5: shared CTest **3/3** and static CTest **3/3** passed. Lint passed for both variants with the existing informational unused QMarkdown import in import-only; no warnings. Symbol checks passed for **140** prefixed external definitions with no public cmark exports. Relocated shared linked, shared plugin-only and static consumers passed, including Setext/indented text, rule geometry and live color/thickness edits. Static consumers use the custom relative `share/qml` destination without a source import path. Packaging artifacts: `/tmp/qmarkdown-packaging-r89pm3of`; commands and complete output: `/tmp/qmarkdown-leaf-packaging.log`.
- V6: captured and visually inspected `/tmp/qmarkdown-leaf.light.neutral.png`, `/tmp/qmarkdown-leaf.light.alternate.png`, `/tmp/qmarkdown-leaf.dark.neutral.png`, `/tmp/qmarkdown-leaf.dark.alternate.png`. Heading underlines are hidden, formatted H1/H2 use inherited heading styles, rules span preview width at 1/3 pixels, indented code retains literal syntax/internal blank line/residual indentation and wraps in the larger alternate code font. Controls show rule colors/thickness and fit at 1000×700. Alternate content below the viewport remains scrollable. The simulated light palette leaves some existing platform button backgrounds dark, giving those button labels low contrast; native controls/platform-theme switching beyond the supplied palette roles was not changed or fully validated.
- Final `git diff --check` passed. Current version/import/private metadata and documentation were reviewed; historical records and third-party source are untouched.

Limits: authored bounded fixtures are not the official full CommonMark suite. Full container precedence, lists/quotes, links/images/resource policies and full conformance remain deferred. Captures use offscreen software rendering; other platforms, real desktop theme transitions and the minimum toolchain were not executed.
