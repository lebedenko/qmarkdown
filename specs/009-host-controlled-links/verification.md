# 009: Verification

**Approval:** On 2026-10-08 the user explicitly requested implementation of the supplied Feature 009 plan, approving these requirements, design and tasks for package 0.6.0 / module 0.6 before implementation.

## Planned
V1: Parser links, decoded destinations, unresolved refs, adjacency, formatting, Unicode and image nesting.
V2: Wrapped/bidi/heading/container hits, font composition, precise blank/separator boundaries, live layout/style changes.
V3: Primary mouse/touch, cancellation/scroll/replacement/clear, hover and zero resource requests.
V4: Playground controls and plain-text reporting; style lifecycle; installed linked/plugin-only/static consumers.
V5: Full CTest, QML lint, symbol privacy, shared/static relocation packaging, light/dark inspection.

## Actual
Executed on 2026-10-08 on Linux with Qt 6.12.0:

- T1 / R1–R5: Approval recorded in requirements, design and tasks before implementation; specification index and roadmap updated.
- T2 / R1,R4 / V1: `build-shared/tests/qmarkdown-parser-test linkDestinations linkSpansAndImages` passed (11 including initialization/cleanup). Covers inline, full/collapsed/shortcut reference, URI/email autolinks, decoded backslash/entity destinations, relative/fragment/custom/empty destinations, formatted labels, distinct adjacent same-destination spans, UTF-16 emoji/Japanese offsets, image nesting, unresolved references, code/HTML and a heading in recursive containers.
- T3 / R2,R3 / V2,V3: Focused `linkInteraction linkLayout` passed (9 including initialization/cleanup); additional `inactiveLinksAndScrolling linkStyleLifecycle viewerTheme viewer containerLayoutAndStyle` passed (7 including initialization/cleanup). Final `linkInteraction linkHitBoundariesAndLiveHover` passed (8 including initialization/cleanup). Pixel/fractional-point layouts compose strong/emphasis/code, wrap mixed bidi/emoji text, retain adjacent identities, reject hard-break separators/outside cells/blank tails, and update link color/underline/width. Hover uses scene coordinates mapped to current paint geometry, including a stationary pointer during font resizing; pointing-hand/arrow cursor transitions pass.
- V3: One exact view signal per primary click and synthetic touch tap across paragraph, heading, nested quote/list, empty destination and enclosing-image links. Secondary clicks, canceled mouse drags, blank text and replacement/clear during a press emit no activation. An actual Flickable contentY change verifies scrolling with no activation. Ordinary text, image-description links, inline/indented code and literal HTML stay inert. The instrumented QML network manager records zero resource requests.
- V4: Shared/replaced/destroyed styles, defaults and notification deduplication pass. Playground click reports `Last activated: #heading` through a PlainText label; palette link binding, valid/invalid manual color edits, Alternate/Reset color and underline pass. Installed 0.6 consumers exercise decoded destination spans, real mouse-event activation through the public signal, link defaults and live style changes.
- V5: `ctest --test-dir build-shared --output-on-failure` passed 3/3 after final code changes. `cmake --build build-shared --target all_qmllint` passed without warnings; only the established import-only unused-import informational message remains.
- Scaling: `QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH=build-shared/qml QT_SCALE_FACTOR=2 build-shared/tests/qmarkdown-view-test linkInteraction linkLayout linkHitBoundariesAndLiveHover inactiveLinksAndScrolling` passed (11 including initialization/cleanup).
- Final `python3 scripts/verify-packaging.py` passed: fresh shared/static builds, 3/3 CTest in each, QML lint, 140 prefixed bundled symbols and shared-export privacy, install/relocation metadata/notices, linked and plugin-only shared consumers, and static consumer. Final log: `/tmp/qmarkdown-links-packaging-final.log`; artifacts: `/tmp/qmarkdown-packaging-0e32r9u4`. An earlier complete packaging run also passed before the final hover-coordinate correction; the final run supersedes it.
- Visual inspection: `QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software QML_IMPORT_PATH=build-shared/qml QMARKDOWN_CAPTURE_THEME=/tmp/qmarkdown-links build-shared/tests/qmarkdown-view-test viewerTheme` passed. Captured four light/dark Neutral/Alternate views; inspected light Neutral and dark Alternate images. The linked heading, formatting, wrapped/nested links, enclosing image description, inert inner image link, underline switch and plain-text activation display render as intended. Capture paths: `/tmp/qmarkdown-links.{light,dark}.{neutral,alternate}.png`.
- `git diff --check` passed. Package/module/private metadata is 0.6.0/0.6; no added dependencies, cmark-source changes or generated dependency lockfile edits.

## Environmental limits

Touch events are synthesized by QtTest; physical touchscreen input was not inspected. Visual inspection used offscreen software rendering, not physical display/screens with mixed DPI. Available Qt is 6.12.0; Qt 6.8 compatibility was not executed. Full CommonMark conformance, keyboard traversal/accessibility/selection/tooltips/visited states/resource rendering and automatic navigation remain outside this approved scope.
