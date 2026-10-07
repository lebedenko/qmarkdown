# 004: native fenced code block verification

**Status:** Implemented and verified for the bounded Feature 004 slice, version 0.3.0 / QML module 0.3. The user explicitly approved requirements R1–R6, design, and tasks T1–T7 on 2026-10-07.

## Planned checks

| ID | Checks |
| --- | --- |
| V1 | Matching/mismatched markers, shorter/longer closers, whitespace-only suffixes, indentation boundaries, runs exceeding 255 markers, invalid backtick info, empty/unclosed fences and adjacent paragraphs/headings. |
| V2 | Space indentation, tabs/partial tabs, preserved whitespace/blank lines, final-newline variants without phantom lines, Unicode, CRLF/CR, NUL, literal inline/resource syntax, full info escape/entity decoding and conservative adaptation fallback. |
| V3 | Hidden delimiters, PlainText/no ranges, code font/color, empty and blank-line heights, long-line wrapping, width changes, zero-width recovery, exact block gaps, replacement and clear after completed polish. |
| V4 | Whole-font and subproperty notifications, shared styles, replacement/old-style disconnection, destruction/default reset, body/inline-code independence and zero resource requests/activation. |
| V5 | Focused parser/view tests first; shared/static builds and CTest, QML lint, private API/metadata and bundled symbol checks, existing relocated shared/plugin-only/static packaging consumers with code content/live styling. |
| V6 | Capture and inspect updated viewer in both styles; verify current version/documentation consistency, unchanged historical records and explicit conformance limits. |

## Actual results

Specification preparation only: inspected document splitter, private block model/view state, MarkdownStyle, MarkdownView, CMake/test setup, and Feature 003 records. Consulted [CommonMark 0.31.2](https://spec.commonmark.org/0.31.2/#fenced-code-blocks). No source code changed and no runtime tests run at this approval stage. Git status could not run because the supplied workspace has no Git repository metadata.

After approval, record exact commands, toolchain versions, outcomes/counts, capture paths and inspection findings, failures and environmental limitations here. Full CommonMark conformance remains unverified. Claim Qt 6.8/minimum-toolchain execution only if actually performed.

## Implementation and actual verification — 2026-10-07

Implemented private code blocks and info strings, full-length fence matching/content extraction, column-aware indentation removal, validated public-cmark info decoding with deterministic private ownership, independent code font/color, private render-kind role, and native PlainText code delegation. The semantic LF remains in the private model; only display removes one terminal LF. Updated package/module/consumer metadata to 0.3.0/0.3, editable examples, regression tests and current documentation. No dependency added and historical Feature 001/003 records were not edited.

Environment: Linux x86_64, Qt/Qt Test 6.11.2, GCC 16.2.1, CMake 4.4.4. Qt 6.8 and CMake 3.21 execution remain unverified.

Commands (focused checks preceded broader checks):

```sh
cmake -S . -B build-shared -DCMAKE_INSTALL_PREFIX=/tmp/qmarkdown-install
cmake --build build-shared --parallel 2
build-shared/tests/qmarkdown-parser-test
QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/tests/qmarkdown-view-test fencedLayoutAndStyle
ctest --test-dir build-shared --output-on-failure
cmake --build build-shared --target all_qmllint
python3 scripts/verify-cmark-symbols.py build-shared
python3 scripts/verify-packaging.py > /tmp/qmarkdown-fences-packaging.log 2>&1
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software QML_IMPORT_PATH="$PWD/build-shared/qml" QMARKDOWN_CAPTURE_VIEWER=/tmp/qmarkdown-fences-viewer.png build-shared/tests/qmarkdown-view-test viewer
```

Final parser result: **77 passed**, zero failures/skips. Focused code layout/style: **3 passed** including initialization/cleanup. Complete view suite: **12 passed** including initialization/cleanup; import suite remains **3 passed**. Shared and static CTest each passed **3/3**. Lint passed with the pre-existing informational unused QMarkdown import in the import-only example; no warnings. Symbol checks passed for **140** prefixed external definitions with no cmark symbols/types in shared exports.

The packaging script passed and retained `/tmp/qmarkdown-packaging-bc78gjdd`; full commands/output are in `/tmp/qmarkdown-fences-packaging.log`. It built both variants, ran CTest/lint/symbol checks, installed and relocated prefixes, checked metadata/notices and ran linked shared, plugin-only shared and static consumers. Consumers import 0.3, render both fence markers with whitespace and long lines, and check native PlainText code font/color after live subproperty edits. Static deployment uses the existing custom relative QML destination with no QMarkdown filesystem import path. No external cmark dependency is needed.

After strengthening font independence, idempotent setter, wrapping bounds and negative-width assertions, rebuilt the view test and reran CTest in both retained packaging builds:

```sh
cmake --build /tmp/qmarkdown-packaging-bc78gjdd/build-shared --target qmarkdown-view-test --parallel 2
ctest --test-dir /tmp/qmarkdown-packaging-bc78gjdd/build-shared --output-on-failure
cmake --build /tmp/qmarkdown-packaging-bc78gjdd/build-static --target qmarkdown-view-test --parallel 2
ctest --test-dir /tmp/qmarkdown-packaging-bc78gjdd/build-static --output-on-failure
```

Both passed **3/3** again. Only tests/documentation changed after the full packaging run.

| Check | Actual evidence |
| --- | --- |
| V1 | Authored fences rows cover both markers, mismatches, shorter/longer closers, invalid suffixes, indentation and >255-marker runs; invalidFencesAndResume and headingAndFenceFormatting cover invalid info/openers, neighbors and resumed paragraphs/headings. |
| V2 | Fixtures cover partial/residual tabs, spaces, blank lines, empty/unclosed/opener-only blocks, final LF versus unterminated content, CRLF/CR, Unicode, NUL, literal Markdown and resources, full escape/entity decoding, unknown entities and tilde-looking info. Decoder validates tree shape defensively; failure injection into that small private helper was not performed. |
| V3 | fencedLayoutAndStyle observes hidden fences/metadata, literal PlainText content without painted formatting, empty-line metrics and two intentional blank lines, wrapping/width recovery, exact gaps, replacement and clear. Viewer and installed consumers additionally exercise an unbroken long line. |
| V4 | Tests verify defaults, notifying QML font/color edits, whole-font assignment and idempotence, two shared views, replacement and old-style isolation, destruction/reset, independent body/inline-code edits, zero/negative widths, zero network requests and no link activation after a click. Existing style and private-API tests also pass. |
| V5 | Shared/static CTest, lint, symbol privacy, metadata/install/relocation and all three consumers pass. |
| V6 | Inspected both 1000×700 software-rendered viewer captures. Neutral shows hidden delimiters/info, literal HTML/entities/Markdown, preserved whitespace/blank lines, wrapped code and an empty-block gap. Alternate shows independent larger blue code, green body and purple headings with wrapping; host scrolling clips overflow as intended. Current versions and documentation updated, historical records preserved. |

Captures: `/tmp/qmarkdown-fences-viewer.png` and `/tmp/qmarkdown-fences-viewer.png.alternate.png`. These are headless software captures, not manual desktop validation or every rendering backend.

Issues corrected during verification: obsolete protected-fence expectations first prevented test compilation; native multiline Text height rounds differently from FontMetrics, so blank-line geometry uses a rounding tolerance after polish rather than Text.lineCount; std::unique_ptr template instantiations exposed cmark type names in dynamic symbols, corrected using the established private local owner pattern. Final checks pass.

Limitations: full CommonMark conformance and its official suite remain unverified; this is bounded top-level fence handling, with existing unsupported container/block fallback unchanged. Minimum toolchains, platform-specific fonts/DPI, other graphics backends and interactive desktop behavior were not comprehensively tested. Conservative info adaptation failure is defensive and unforced in tests. No highlighting, copy actions, selection, custom renderers or public language metadata is delivered. Git status/diff could not run because this workspace has no Git metadata.
