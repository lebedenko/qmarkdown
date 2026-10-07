# 003: inline formatting verification

**Status:** Implemented and verified for the approved bounded Feature 003 slice, version 0.2.0 / QML module 0.2. User approval recorded 2026-10-07; actual results follow.

## Planned checks

| ID | Checks |
| --- | --- |
| V1 | Check pinned archive SHA-256/provenance, complete applicable notices, offline configure/build, C99/PIC target privacy, and symbol prefixing/hidden visibility through symbol inspection. |
| V2 | Authored parser fixtures for nested/mixed emphasis, underscore boundaries, unmatched delimiters, matching backtick runs, code whitespace normalization, escapes, valid/invalid named/numeric entities, and Unicode. |
| V3 | Sentinel removal and block-looking input, preserved links/images/autolinks/HTML with preceding multibyte Unicode, unresolved references, protected fences, unchanged block order/replacement, and injected invalid spans/unexpected-tree conservative fallback. |
| V4 | Native resolved font ranges, body/heading code-size inheritance, overlay precedence with nested emphasis/strong, wrapping across mixed fonts, bidi/combining text, empty headings, resizing, spacing, nonpositive widths, ink overhang, and cached layout invalidation after completed polish. |
| V5 | Whole-font and QML subproperty edits, reset/shared/replaced/destroyed styles after completed polish; zero resource requests and absent link activation; implementation types unavailable under public QMarkdown URI. |
| V6 | Focused tests first, then shared/static CTest and QML lint; relocated shared, plugin-only shared, and static installed consumers render formatted text and apply live code-font edits with no external cmark dependency. |
| V7 | Capture and inspect editable viewer with nested formatting, escapes/entities, code and literal resource syntax in both styles; review documentation/version claims and retain Feature 001 history. |

## Initial partial implementation — historical results

User approval of requirements, design, and tasks recorded on 2026-10-07. Implemented the independent inlineCodeFont API in markdownstyle.h/.cpp and added ViewTest::inlineCodeFontOverlay. Formatting parsing/rendering is not implemented yet; the package remains at version 0.1.0 until that integration is complete.

Environment: Qt/Qt Test 6.11.2, GCC 16.2.1, CMake 4.4.4. Qt 6.8 execution is unverified.

Commands and results:

```sh
cmake --build build-shared --parallel 2
QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/tests/qmarkdown-view-test inlineCodeFontOverlay
ctest --test-dir build-shared --output-on-failure
cmake --build build-shared --target all_qmllint
cmake --build build-static --parallel 2
ctest --test-dir build-static --output-on-failure
```

Shared build passed. Focused font test: 3 passed including initialization/cleanup. Shared and static CTest: each 3/3 passed; both builds passed. QML lint passed with the import-only example's existing informational unused-import notice. The new test checks system fixed-family default, inherited heading size/weight/italic, notifying QML subproperty edits, whole-font assignment, and default reset including resolve mask. An initial test tried to assign a QObject-typed null to a typed QML alias; replacing that with QML `style = null` exercised the intended public operation and passed.

Source acquisition blocker: curl downloads from both github.com and codeload.github.com failed with `Could not resolve host`. Searches of workspace, /tmp, project directories, caches, and system include/share paths found cmark 0.31.2 headers/binaries and the binary package cache, but no upstream library source archive. These cannot meet approved private source bundling/provenance/offline-build requirements. No substitute parser, binary vendoring, or external package linkage was introduced. T2/T3/T5–T8 remain pending, as does the model-role portion of T4. Full packaging and viewer capture are not claimed for Feature 003.

Record exact commands, toolchain versions, test counts, captured artifact location and inspection findings, plus failures/limitations when checks run. Do not claim Qt 6.8 execution unless that toolchain is available and tested. Official CommonMark suite import and minimum-version infrastructure are outside this iteration.

## Source acquisition retry — 2026-10-07

After the user reported that connectivity should be fixed, retried the pinned archive at both https://codeload.github.com/commonmark/cmark/tar.gz/refs/tags/0.31.2 and https://github.com/commonmark/cmark/archive/refs/tags/0.31.2.tar.gz using curl --fail. Also checked raw.githubusercontent.com. All three hosts still failed with curl exit code 6 (`Could not resolve host`) in the execution environment. No archive was created or found in the workspace or readable /tmp paths. The source-acquisition blocker persists; no implementation files changed during this retry and tests were not rerun.

## Completed implementation and verification — 2026-10-07

The user supplied `/home/andrii/Downloads/cmark-0.31.2.tar.gz`, resolving the earlier source-acquisition blocker. SHA-256: `f9bc5ca38bcb0b727f0056100fac4d743e768872e3bacec7746de28f5700d697`. Compared every copied upstream source/table byte-for-byte against the extracted archive, except the two deliberately locally configured headers, and compared COPYING in full: passed. Acquisition URL/version/checksum and header/symbol-map changes are in [provenance](../../third_party/cmark/PROVENANCE.md). No conformance fixtures were imported.

Implemented the private C99/PIC bundle, inline adapter and model ranges, polished/cached QTextLayout delegate, font overlay, version updates, viewer content and alternate style, installed-consumer live code-font edits, and notice/symbol packaging checks. Source parsing uses only cmark's public document/node API. Protected fences and the original block splitter remain unchanged. Authored implementation is C++17; no dependency is fetched at configure/build time.

Environment: Linux x86_64, Qt/Qt Test 6.11.2, GCC 16.2.1, CMake 4.4.4. Qt 6.8 execution remains unverified. The compatibility review corrected DelegateChooser's import to `Qt.labs.qmlmodels`, which is documented in Qt 6.8; its `QtQml.Models` import begins at Qt 6.9. See [Qt 6.8 documentation](https://doc.qt.io/qt-6.8/qml-qt-labs-qmlmodels-delegatechooser.html).

Executed commands (focused tests preceded full checks):

```sh
sha256sum /home/andrii/Downloads/cmark-0.31.2.tar.gz
cmake -S . -B build-shared -DCMAKE_INSTALL_PREFIX=/tmp/qmarkdown-install
cmake --build build-shared --parallel 2
build-shared/tests/qmarkdown-parser-test
QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/tests/qmarkdown-view-test inlineCodeFontOverlay formattedFontAndLayout
QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/tests/qmarkdown-view-test codeInEveryHeading formattedFontAndLayout decodedLineSeparators
QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/tests/qmarkdown-view-test plainTextAndPrivateApi
QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/tests/qmarkdown-view-test
ctest --test-dir build-shared --output-on-failure
cmake --build build-shared --target all_qmllint
python3 scripts/verify-cmark-symbols.py build-shared
readelf -d build-shared/src/QMarkdown/libQMarkdown.so.0.2.0
python3 scripts/verify-packaging.py > /tmp/qmarkdown-inline-packaging-final.log 2>&1
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software QML_IMPORT_PATH="$PWD/build-shared/qml" QMARKDOWN_CAPTURE_VIEWER=/tmp/qmarkdown-inline-viewer.png build-shared/tests/qmarkdown-view-test viewer
```

Final results: parser **65 passed**, native/viewer **11 passed**, import **3 passed**, including initialization/cleanup. Shared and static CTest each report **3/3 passed**, no failures or skips. QML lint has no warnings; the import-only example retains its informational unused-import notice. All **140** externally defined bundled symbols match the prefix map, and no cmark symbol/type is dynamically exported. ELF NEEDED entries contain Qt/system libraries and no external cmark library.

The final packaging script retained `/tmp/qmarkdown-packaging-6ukez0il` and logged commands/output to `/tmp/qmarkdown-inline-packaging-final.log`. It built and tested both variants, linted, verified symbol maps, installed and relocated prefixes, checked metadata privacy/relocatability and complete notices, and built/ran the copied linked shared, plugin-only shared, and static consumers. Consumers import QMarkdown 0.2, render formatted heading/body/code content, and apply a QML code-font size edit that increases layout height. Static uses a custom `share/qml` destination and no QMarkdown filesystem import path. Public consumers require no cmark headers, targets, or external package.

After strengthening the click-inactivity assertion in the existing resource test, rebuilt only the view test and reran CTest in both retained final packaging builds:

```sh
cmake --build /tmp/qmarkdown-packaging-6ukez0il/build-shared --target qmarkdown-view-test --parallel 2
ctest --test-dir /tmp/qmarkdown-packaging-6ukez0il/build-shared --output-on-failure
cmake --build /tmp/qmarkdown-packaging-6ukez0il/build-static --target qmarkdown-view-test --parallel 2
ctest --test-dir /tmp/qmarkdown-packaging-6ukez0il/build-static --output-on-failure
```

Both again passed **3/3**. Parallel GNU Make runs emitted an incidental jobserver FIFO-exists message; both builds completed successfully. No implementation changed after the packaging run.

| Check | Actual evidence |
| --- | --- |
| V1 | Archive/source/notice comparisons passed; private object target compiles with C99, PIC, hidden visibility, and the checked-in complete prefix map. Symbol checker passed for both shared and static builds; shared exports contain no cmark symbols/types. Installed metadata excludes the object target and source include paths. |
| V2 | Authored inlineSyntax, mixedRanges, and existing normalization/headings fixtures cover nested/mixed emphasis, underscores, unmatched delimiters/backtick runs, code whitespace/tabs and non-decoding, escapes, named/numeric/invalid entities, Unicode, and coalesced nonoverlapping UTF-16 flags. |
| V3 | preservedSource tests links/images/autolinks/HTML after multibyte/combining Unicode and decoded entities/escapes, and inside strong emphasis without adapting children. Block-looking input and definitions remain ordinary inline text through sentinel parsing. Fence/order tests pass. conservativeFallback checks invalid/out-of-bounds/incomplete UTF-8 spans and unexpected multiline trees. invalidAdapterTree mutates a borrowed public cmark document to verify complete literal fallback after valid formatted content for invalid source positions, unexpected nodes/siblings, and a missing sentinel. |
| V4 | formattedFontAndLayout checks resolved nested font flags, weight ≥ bold, inherited sizing, mixed-font narrow/wide wrapping, RTL glyph runs, combining-cluster cursor boundaries, ink/indivisible-glyph raster bounds separate from one-pixel logical width, nonpositive widths, spacing, empty headings, and replacement. codeInEveryHeading checks H1–H6 code-font size/weight inheritance and a live independent H3 edit. decodedLineSeparators verifies named newline-entity geometry agrees with native PlainText layout. |
| V5 | inlineCodeFontOverlay covers defaults, whole-font assignment, notifying subproperty edits, resolve-mask changes, and reset. formattedStyleLifecycle checks shared edits, replacement/old-style disconnection, destruction, and reset after polish. plainTextAndPrivateApi verifies zero requests in formatted and unformatted paths, absence of a link-activation signal on the painted item, zero PlainText activation signals after a mouse click, and rejection of FormattedText and other implementation types under QMarkdown. |
| V6 | Shared/static CTest and lint pass. All three relocated consumer variants pass formatted rendering and live code-font edits. Installed complete COPYING/provenance are verified; no private cmark header or object target is installed/exported. |
| V7 | Captured and visually inspected neutral and alternate viewer images; reviewed documentation/status/version/approval claims and retained Feature 001 historical results. |

Viewer artifacts: `/tmp/qmarkdown-inline-viewer.png` and `/tmp/qmarkdown-inline-viewer.png.alternate.png`, both 1000×700. Neutral capture shows italic/strong/nested code, visible code-span backticks, decoded escapes/entities, literal resource-looking syntax, and protected fences. Alternate capture shows larger serif inline code, green body typography, independent purple headings, and natural wrapping. Host scrolling clips the bottom of the longer alternate preview as expected; automated viewer tests exercise sample/edit/clear/reset/style changes and overflow. These are headless software-rendered captures, not manual desktop interaction or verification of every rendering backend.

Issues caught and corrected during verification: an authored fixture needed escaped quotes; moc treated a multiline raw QML string containing `#` as preprocessing input, so fixtures now use adjacent ordinary C++ strings; UTF-8 validation needed eagerly evaluated, stateless QStringDecoder output to reject incomplete sequences; private adapter ownership/template symbols and its cmark-bearing signature required isolation; decoded LF characters required Qt line separators in the layout string to match PlainText geometry. Final checks pass with unexpected Qt warnings treated as test failures.

Limits: Qt 6.8 and CMake 3.21 execution remain unverified. Adapter nesting deeper than 512 conservatively falls back to the entire normalized literal block. The authored tests verify the approved bounded slice, not the official CommonMark suite, full block semantics, hard breaks, reference collection, semantic fences, resource policies, streaming, or selection. Platform-specific fonts, DPI, interactive desktop behavior, and other graphics backends were not comprehensively verified. Feature 001's recorded results remain unchanged; this record establishes Feature 003's superseding behavior.

Final documentation checks: passed 58 relative link targets across current documentation and Feature 003 records, R1–R7/V1–V7 traceability, completed T0–T8 tasks, and authored-source/documentation whitespace. A stale-version/status search found no 0.1 package/module claims or draft/unimplemented status in current README/build/examples/consumer files or Feature 003 requirements/design/tasks. The workspace still has no Git metadata; review and checks used files directly.
