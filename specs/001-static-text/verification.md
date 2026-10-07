# 001: static text verification

**Status:** Bounded slice implemented and verified on 2026-10-07. User approval covers the existing requirements, design, and tasks, as requested by the implementation plan. Full CommonMark conformance and Qt 6.8 execution are not claimed.

## Planned implementation checks

These are bounded-slice checks, not CommonMark conformance evidence. Use Qt Test for private semantic results and QQmlEngine/QQuickWindow native behavior, waiting for completed layout/polish rather than arbitrary sleeps or immediate geometry assertions. Prefer text, block order, font/color properties, and geometry assertions over pixel snapshots or platform-dependent exact heights. Record concrete test names/commands when implemented.

| Check | Requirements / tasks | Planned automated behavior | Planned viewer inspection |
| --- | --- | --- | --- |
| V1 | R1 / T2, T6, T7 | LF/CRLF/lone-CR equivalence, blank space/tab separators, leading/trailing trimming, internal whitespace preservation, soft-break joining, NBSP nonseparator, Unicode combining marks/non-Latin/emoji, U+0000 replacement. | Multiline and Unicode prose. |
| V2 | R2 / T2, T3, T6, T7 | H1–H6, zero–three indentation columns versus four/tab indentation, space/tab/end separators, seven markers, `#title`, interruption, empty headings, optional closing markers with/without preceding whitespace and trailing suffix, literal backslashes. Assert empty heading has one font line at positive width. | Six levels and empty heading gaps. |
| V3 | R2–R3 / T2, T3, T6, T7 | Ordered paragraphs/headings/fallback, literal inline/HTML/escape/entity/Setext/list/quote/link/image/rule text. Backtick/tilde fences: indentation, info strings, invalid backtick info, longer/shorter/mismatched closers, suffix text, blank interior lines, preserved indentation/line endings/punctuation, adjacent paragraphs, empty and unclosed fences, protected headings. | Inspect literal fallback including blank lines. |
| V4 | R4 / T3–T6, T7 | Positive narrow/wide/restored widths, unchanged block text/order, long-word wrapping with no horizontal overflow at widths exceeding glyph/cluster width, plus documented extreme-width overhang, no overlapping blocks, height recomputation, exact gap count, zero/negative width, implicitWidth zero, implicitHeight equals contentHeight, explicit height independent of contentHeight. Use portable relational geometry assertions. | Resize and host scrolling/overflow integration. |
| V5 | R4–R5 / T4, T6, T7 | Defaults, independent H1–H6 fields, font subproperty/whole-font edits, colors, transparent colors, spacing including negative/nonfinite values, shared styles, replacement/disconnected old object, supplied-object destruction, null and QML reset restoring defaults, modifying returned default. Assert content unchanged and font-size/spacing edits update height after polish. | Switch two visibly different styles and reset. |
| V6 | R1, R4, R6 / T2, T5, T6, T7 | Mixed→shorter→identical→empty replacement, ASCII whitespace-only and all line-ending forms, no stale/duplicate blocks, zero height/no edge gaps; repeat with width zero then positive, and with nonzero spacing. NBSP-only content remains visible. | Replace and clear repeatedly. |
| V7 | R3, R7 / T2–T4, T6, T7 | Standalone QML host with only Qt/QMarkdown; PlainText items and no link handlers, resource-request spy on image/link/HTML-looking strings, private types unavailable as public constructible API, dependency/source review against prohibited renderers and host modules. | Literal HTML/images/links; no activation. |
| V8 | R5–R7 / T6, T7 | Extend installed consumer to construct MarkdownStyle and MarkdownView, set content/style, assert replacement and runtime font/color propagation. Run existing packaging script for shared/static builds and relocated prefixes, using only installed package; preserve static plugin registration and custom QML destination coverage. | Viewer is supplemental; packaging assertions are automated. |

Planned commands after implementation: configure/build the existing shared/static CMake variants, run their focused Qt Test binaries and `ctest --test-dir <build> --output-on-failure`, run `all_qmllint`, and run `python3 scripts/verify-packaging.py`. Offscreen geometry checks need no desktop; visible viewer inspection needs a graphical session. Qt 6.8/CMake 3.21 execution is a separate pending scaffold improvement, not claimed here. T7 must record actual commands and results for V1–V8. See [tasks](tasks.md) for complete requirement coverage and [standards](../standards.md) for the later full official suite.

## Actual documentation results — 2026-10-07

The design-resolution iteration changes specifications only. Reviewed existing module/test/consumer conventions and primary CommonMark, cmark licensing, and Qt Text references. Resolved parser, syntax/fallback, style, layout, and update choices; completed T0 only. Relative Markdown link targets, R1–R7/T0–T7/V1–V8 coverage, consistent draft/approval status, and documentation-only scope were checked. The local documentation checker was run with `python3 /tmp/check_specs.py`: PASS, 44 relative link targets; R1–R7 delivery/check mapping; T0–T7 and V1–V8 identifiers; all four feature records draft with approval not recorded. A stale-rule search using `rg` found no remaining column-zero/trailing-marker or unresolved-design wording. These are documentation results, not implementation tests. No upstream fixtures were copied and no dependency was added.

## Actual implementation results — 2026-10-07

Environment: Linux x86_64; Qt and Qt Test 6.11.2 (shared Qt runtime), GCC 16.2.1, CMake 4.4.4. Authored C++ uses C++17. No dependency or upstream fixture was added. The repository workspace has no `.git` metadata, so Git status/diff were unavailable; files were reviewed directly.

Executed commands:

```sh
cmake -S . -B build-shared -DCMAKE_INSTALL_PREFIX=/tmp/qmarkdown-install
cmake --build build-shared --parallel 2
build-shared/tests/qmarkdown-parser-test
QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/tests/qmarkdown-view-test
ctest --test-dir build-shared --output-on-failure
cmake --build build-shared --target all_qmllint
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software QML_IMPORT_PATH="$PWD/build-shared/qml" QMARKDOWN_CAPTURE_VIEWER=/tmp/qmarkdown-viewer.png build-shared/tests/qmarkdown-view-test
python3 scripts/verify-packaging.py
```

Final results: parser Qt Test totals **28 passed**, native/viewer totals **6 passed**, and import totals **3 passed**, including each suite's initialization/cleanup. Shared and static CTest each report **3/3 passed**, with no failures or skips. QML lint reports no warnings; the retained import-only example has its expected informational unused-import notice. An early native test found an inherited `state` name collision in delegates; renaming the adapter and binding component scope fixed it. A viewer test's assumed default overflow was corrected to test overflow at a reduced window height. Final runs pass with unexpected Qt warnings treated as failures. After tightening the empty-heading assertion to exactly one native font line and asserting default font weights, the native/viewer suite was rebuilt and rerun in both shared and static variants: 6/6 Qt Test results passed in each. A documentation check passed 47 relative link targets, approval records, and completed T0–T7 checkboxes.

| Check | Actual evidence |
| --- | --- |
| V1 | `ParserTest::normalization`: LF/CRLF/CR equivalence, ASCII trimming and separators, internal whitespace, combining marks, Japanese, emoji, NBSP, and NUL replacement. |
| V2 | `ParserTest::headings` has 18 cases for six levels, indentation/tab boundary, separators, seven markers, missing separators, empty headings, closing runs, suffixes, backslashes, and NBSP. `ViewTest::nativeLayoutAndReplacement` verifies empty-heading line geometry and exact inter-block gaps. |
| V3 | `literalAndOrder`, five `fences` cases, and `invalidFencesAndResume` cover source order, literal punctuation, protected headings, closed/unclosed fences, info restrictions, indentation, normalized preserved line endings, mismatched/shorter/longer closers, suffixes, and parsing resumption. Viewer capture shows literal inline and fenced presentation. |
| V4 | `nativeLayoutAndReplacement` verifies narrow/wide/restored width, long-word wrapping, item width, extreme narrow width, nonpositive width suppression, gap count, implicit sizing, explicit-height independence, and replacement. Event-loop retries wait for the requested final geometry. Viewer resizing and host-owned overflow also pass. |
| V5 | `styleLifecycle` verifies body/H1–H6 defaults and independence, QML font subproperty edits, whole-font edits, shared styles, transparent colors, negative/infinite/NaN spacing, replacement, old-style disconnection, supplied-style destruction, returned-default mutation, and null/undefined/property-reset behavior. Runtime edits update text properties and geometry. |
| V6 | `nativeLayoutAndReplacement` verifies mixed-to-shorter-to-identical-to-empty replacement, whitespace-only and NBSP-only content, zero-height/no stale blocks, and replacement while width is suppressed. Viewer clear/sample/edit cycles pass. |
| V7 | `plainTextAndPrivateApi` asserts PlainText, preserved HTML-looking punctuation, zero requests via a QQml network manager factory/request counter, and rejection of constructible ViewState/BlockModel/ModuleAnchor under the public URI. Source review confirms only Item/Column/Repeater/Text layout, no link handlers or resource-loading/rendering engines, and no host module dependencies. Private adapters register only under QMarkdown.Private; anonymous public metadata supplies no constructible API. |
| V8 | Packaging script passes shared and static builds, CTest, lint, install, prefix relocation, exported-metadata path checks, and three installed consumers: linked shared, plugin-only shared, and static. Consumers instantiate both public types and assert live font/color propagation, height changes, replacement, and clear. Static uses the custom share/qml destination and no QMarkdown filesystem import path. |

The successful packaging run retained `/tmp/qmarkdown-packaging-_blnjdso`; its console output is `/tmp/qmarkdown-packaging.log`. Both variants built the import-only example and editable viewer. The installed source fixture is copied before building consumers, so no source-tree module dependency can satisfy their imports.

Viewer inspection: `ViewTest::viewer` loads the actual viewer QML, exercises edit/sample/clear/style-switch/reset, resizes, and verifies scrolling overflow at reduced height. A 1000×700 offscreen software-rendered capture was saved to `/tmp/qmarkdown-viewer.png` and visually inspected: H1–H6, empty-heading gap, literal inline/fenced text, Unicode, and long-word wrapping display correctly. This is headless visual inspection; interactive desktop mouse/keyboard behavior and platform-specific rendering were not manually inspected.

Limits: Qt 6.8 and CMake 3.21 execution remain unverified; no minimum-version claim is made. Native glyph shaping, tab width, and indivisible-cluster overhang follow Qt. Tests cover the approved bounded slice, not the official CommonMark suite, full block precedence, semantic code blocks, inline semantics, resources, or streaming. Future conformance results remain separate in [standards](../standards.md).
