# qt-markdown

A standalone Qt/QML Markdown rendering library under development, independent of any other project. The library owns Markdown semantics and rendering behavior; applications supply typography and colors. Markdown source feeds a private parser, document model, and native Qt Quick block components, without document-level HTML, QTextDocument, or WebEngine rendering.

**Status:** version `0.7.0` / QML module `0.7` provides native paragraphs, headings, rules, code, ordered/unordered and nested lists, block quotes and mixed containers, with emphasis/strong/code and CommonMark escapes/entities. Feature 010 is approved on 2026-10-08. A single privately bundled cmark 0.31.2 parse supplies block and inline semantics. Links and autolinks report decoded destinations to the host; opt-in PNG/JPEG images use native rows with formatted description fallbacks and HTML stays literal. This iteration does not claim full CommonMark conformance.

## Build and check

With [Task](https://taskfile.dev/) installed, run these shortcuts from the repository:

```sh
task build
task test
task demo
```

`build` configures `build-shared` with tests and examples enabled and builds with two parallel jobs. `test` builds first, then runs CTest. `demo` builds first, then opens the editable playground with its QML import path configured, inheriting your display environment. CMake's generator and build-type defaults are preserved.

Requires CMake ≥3.21, C99 and C++17 compilers, and Qt ≥6.8 Core, Gui, Qml, Quick, and Network. Tests additionally require Qt Test and Python ≥3.9 (standard library only). Local verification used Qt 6.12.0 on Linux; Qt 6.8 compatibility has not yet been executed. Dependencies are not downloaded by this project. [cmark 0.31.2](third_party/cmark/PROVENANCE.md) is bundled privately with prefixed symbols and complete [license notices](third_party/cmark/COPYING); no external cmark package is needed.

```sh
cmake -S . -B build-shared -DCMAKE_INSTALL_PREFIX=/tmp/qmarkdown-install
cmake --build build-shared --parallel 2
ctest --test-dir build-shared --output-on-failure
cmake --build build-shared --target all_qmllint
QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/examples/import-only/qmarkdown-import-example
cmake --install build-shared
```

Tests use the offscreen platform automatically. For a headless example launch, add `QT_QPA_PLATFORM=offscreen`; the example runs until closed. The import-only example retains an expected informational unused-import lint message.

Shared libraries are the default. Use a separate static build directory:

```sh
cmake -S . -B build-static -DBUILD_SHARED_LIBS=OFF
cmake --build build-static --parallel 2
ctest --test-dir build-static --output-on-failure
```

`QMARKDOWN_BUILD_EXAMPLES` and `BUILD_TESTING` default on for standalone builds and off when included with `add_subdirectory`. Both can be overridden. Disable tests with `-DBUILD_TESTING=OFF` to avoid requiring Qt Test or Python. GNU install directories are supported. `QMARKDOWN_QML_INSTALL_DIR` defaults to `${CMAKE_INSTALL_LIBDIR}/qt6/qml`; for relocation, supply a relative path such as `-DQMARKDOWN_QML_INSTALL_DIR=share/qml`. Absolute destinations are supported but remain tied to that location.

## CommonMark evidence

Feature 011 adds a pinned offline baseline, without changing the 0.7 API or production behavior. After building tests, run:

```sh
python3 scripts/verify-commonmark.py
```

The runner checks all 652 CommonMark 0.31.2 examples through the privately bundled parser and the production native model separately, writing `build-shared/tests/commonmark-report.json`. The recorded baseline has 652 parser passes, 583 native projection passes and 69 uncheckable model examples. `--strict` currently exits with failure because comparison limits and semantic information loss remain; passing baseline CTest does not establish full conformance. See the [runner and gap inventory](tests/commonmark/README.md), [fixture provenance/license](tests/commonmark/PROVENANCE.md), and [Feature 011 verification](specs/011-commonmark-baseline/verification.md).

## Rendering and viewer

`MarkdownView` exposes writable `markdown` and `style`, and read-only `contentHeight`. Supply its width explicitly; `implicitWidth` is zero and `implicitHeight` follows content. Text wraps, including long words; an indivisible glyph may overhang an extremely narrow view. Nonpositive width suppresses layout and reports zero height. Hosts own scrolling, clipping, backgrounds, and navigation.

`MarkdownStyle` has independent body and H1–H6 fonts/colors, plus `inlineCodeFont`, `codeBlockFont`, `codeBlockColor`, `thematicBreakColor`, `thematicBreakThickness`, and `blockSpacing`. Defaults capture `QGuiApplication::font()` at style construction: body preserves its size and unit with normal weight; bold H1–H6 use 2/1.75/1.5/1.25/1.125/1 times that size. Point sizes retain fractions; pixel sizes round to a positive integer. Code blocks use the captured body size/unit and system fixed-font family with normal weight. Text defaults to `#202020`, with 8 logical pixels between blocks. Styles can be shared and edited live. Null or undefined resets the view's own style to construction defaults; deleting a supplied style restores that default. A supplied style remains host-owned. Negative spacing lays out as zero; nonfinite spacing uses 8. Rules default to `#202020` and thickness 1; finite thickness renders as `max(0, value)`, nonfinite as 1. Zero-height rules do not participate in Column spacing.

Paragraphs and ATX/Setext headings support emphasis, strong emphasis, inline code, escapes, and character references. Block and inline whitespace semantics come from the single cmark document parse. Emphasis enables italic and strong raises weight to at least bold. `inlineCodeFont` is an overlay on the containing block font: its default sets only the system fixed-font family, inheriting size, weight, and italic state. Surrounding emphasis/strong also apply to code. Code has no background or padding. Whole-font assignment and QML subproperty edits update the cached native layout. Body edits do not resize headings or code blocks. Reset restores construction snapshots; later application-font changes require explicit host updates or a new style.

Fonts support logical pixels (`pixelSize`) and points (`pointSize` in QML, `setPointSizeF` in C++). Spacing, indents and rule thickness always use logical pixels. Qt applies device scaling; do not multiply sizes by device pixel ratio. The playground converts with `px = pt × logical DPI / 72` using the preview window's screen, rounding pixels. Conversion is approximate and can lose precision. See [Qt's logical-DPI model](https://doc.qt.io/qt-6/qscreen.html#logicalDotsPerInch-prop). Native Qt Quick `Text` rounds point sizes to half points when a different font is assigned (an unchanged application font retains its initial precision). The private inline renderer matches this behavior while retaining the requested public font values.

Qt's QML font getters synthesize the other unit. Its subproperty setters also prefer an already explicit pixel size when both units are set, and may warn when changing a point font to pixels. Use whole-font assignments to select a different unit, or C++ `QFont::setPointSizeF`/`setPixelSize` on a copy to preserve all fields. For fractional QML edits, set `pointSize` on a point-based role (for example `style.bodyFont.pointSize = 12.5`). The playground handles both transitions through a local native helper without changing the library API.

Application-based defaults can change wrapping and content heights compared with earlier releases. To reproduce the previous typography, explicitly assign body/code-block fonts at 16 px and bold H1–H6 at 32/28/24/20/18/16 px; leave inline code family-only to inherit the block size. Current package/import versions are 0.7.0/0.7. See [Feature 008](specs/008-font-units/requirements.md).

Links display formatted labels; images retain formatted descriptions until an authorized PNG/JPEG is ready; autolinks display text. Links activate only by reporting destinations to the host; image resources are denied by default. Reference definitions disappear; unresolved references remain ordinary inline text. Inline HTML remains literal; HTML blocks use multiline plain body text. Soft breaks become spaces and hard breaks become newlines. LF/CRLF/CR are equivalent; NUL becomes U+FFFD. cmark owns container indentation, lazy continuation, interruption, tightness and precedence. Ordered lists count from the parsed start and retain `.` or `)`; bullets use `•`. Tight lists have zero interior/item gaps; loose lists and quote children use `blockSpacing`. Empty items/quotes reserve a body line. Gutters measure the widest marker plus 8 pixels, at least `listIndent`; quotes inset at least rule thickness plus 8 pixels. Indentation clamps to leave one content pixel at positive widths. Numeric style values render as nonnegative finite values or their defaults. The bundled parser caps opening fence lengths at 255; very long fence closers consequently follow that upstream limitation. Code whitespace remains literal and info strings stay private. Replacing Markdown disposes the complete child-model tree. Layout settles through Qt Quick polish; hosts own scrolling.

Launch the editable source/preview example:

```sh
QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/examples/viewer/qmarkdown-viewer
```

The playground offers eight bundled samples, live editing, independently scrollable panes with a draggable divider, Fit/240/480/720 preview widths, and collapsible role-based style controls with a px/pt selector, fractional point editing, and dedicated list/quote and thematic-break controls, Neutral/Alternate presets and reset. It starts with Overview; Reload sample restores the selected source and Clear empties it. Neutral and Reset restore application-based construction defaults; Alternate retains its explicit pixel sizes. Inline code displays the inherited body size until edited; selecting or reading it does not create an override, and heading code still inherits heading typography. Editing its size/unit creates an explicit override. All state stays in memory. Qt Quick Controls is required only when `QMARKDOWN_BUILD_EXAMPLES=ON`; library-only builds do not require it.

The playground supplies theme-aware styling: the preview surface and Neutral text follow the application palette, and Alternate accents adapt to light/dark surfaces. Manual color edits persist across theme changes until Neutral or Reset restores live theme colors. The library's default colors remain unchanged; host applications own theming.

## Consume an installed package

```cmake
find_package(QMarkdown 0.7 CONFIG REQUIRED)
target_link_libraries(myapp PRIVATE QMarkdown::QMarkdown)
```

```qml
import QtQuick
import QMarkdown 0.7

MarkdownView {
    width: 480
    markdown: "# Title\n\nBody with *emphasis*, **strong**, and `code`"
    style: MarkdownStyle {
        bodyFont: Qt.font({pointSize: 12})
        bodyColor: "#202020"
        h1Font: Qt.font({pixelSize: 32, bold: true})
        inlineCodeFont.family: "monospace"
        blockSpacing: 8
    }
}
```

Configure the host with `-DCMAKE_PREFIX_PATH=/path/to/install`. `QMarkdown_QML_IMPORT_PATH` exposes the installed QML import directory in CMake. For shared deployment, make that directory available to the QML engine using `QML_IMPORT_PATH` or `QQmlEngine::addImportPath`. The generated plugin loads the backing library through a relative loader path on Linux/macOS; applications linking the backing library also need it available to their platform loader. A Linux temporary-prefix launch can use:

```sh
QT_QPA_PLATFORM=offscreen \
QML_IMPORT_PATH=/path/to/install/lib/qt6/qml \
LD_LIBRARY_PATH=/path/to/install/lib \
./myapp
```

Adjust `lib` for your GNU library directory. Deploy the module metadata and plugin along with the backing library. Hosts own their Qt runtime deployment.

For a static QMarkdown build, explicitly link the installed plugin target and register it in one C++ translation unit:

```cmake
target_link_libraries(myapp PRIVATE
    QMarkdown::QMarkdown QMarkdown::QMarkdownPlugin)
```

```cpp
#include <QtQml/QQmlExtensionPlugin>
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
```

Static registration embeds the module's resource metadata; no QMarkdown source-tree or filesystem import path is required at runtime. Static QMarkdown does not require a static Qt build. Keep Qt-generated resource and plugin initialization objects installed with the archives; exported targets include them automatically.

Run reproducible shared/static install and relocation checks with:

```sh
python3 scripts/verify-packaging.py
```

The script builds both variants, runs tests and lint, installs into temporary prefixes, relocates them, checks CMake metadata for source/build paths, and builds/runs a copied [consumer fixture](tests/installed-consumer/CMakeLists.txt) using only the installed package. It retains its temporary directory and prints its location for inspection. Static checks use a custom relative QML destination and no QMarkdown import path.

See [core leaf block verification](specs/006-core-leaf-blocks/verification.md), [fenced code verification](specs/004-fenced-code-blocks/verification.md), [inline formatting verification](specs/003-inline-formatting/verification.md), [standards and conformance](specs/standards.md), [rendering verification](specs/001-static-text/verification.md) and [scaffold verification](specs/002-project-scaffold/verification.md), [specifications](specs/README.md), [overview](specs/overview.md), [roadmap](specs/roadmap.md), and [project instructions](AGENTS.md). Authored library code is licensed under the [MIT license](LICENSE), attributed to qt-markdown contributors. The bundled cmark sources retain their own [notices](third_party/cmark/COPYING), also installed with the package. No stable API or ABI is promised by this early release.

Container style defaults: `listIndent: 24`, `quoteIndent: 16`, `quoteRuleColor: "#808080"`, `quoteRuleThickness: 2`. All notify on changes and support shared styles and the established default-style reset/destruction lifecycle. See [Feature 007](specs/007-container-blocks/requirements.md) and its [verification](specs/007-container-blocks/verification.md).

### Host-controlled links

```qml
MarkdownView {
    markdown: "[Guide](../guide.md#intro)"
    style: MarkdownStyle { linkColor: "#0066cc"; linkUnderline: true }
    onLinkActivated: function(destination) {
        // Validate, resolve and navigate according to your application's policy.
        console.log(destination)
    }
}
```

`linkActivated(string destination)` reports the cmark-decoded string unchanged, including empty destinations, relative paths, fragments and custom schemes. Primary clicks and touch taps activate links; dragging to scroll cancels activation. The view never navigates links. Image access uses the separate opt-in resource policy. Links inside image descriptions are inert; an enclosing link includes the description. Keyboard link traversal, accessibility and visited states are deferred. See [Feature 009](specs/009-host-controlled-links/requirements.md).

### Images and host resource policy

```qml
MarkdownView {
    markdown: "Before ![description](photos/image.png) after"
    baseUrl: "file:///home/user/documents/readme.md"
    resourcePolicy: MarkdownResourcePolicy {
        allowedFileRoots: ["file:///home/user/documents/photos/"]
        allowedHttpsOrigins: ["https://images.example.org", "https://cdn.example.org:8443"]
        allowQrc: false
    }
}
```

All image permissions default to denied. Relative image destinations resolve only against explicit `baseUrl`; no working directory or component URL is inferred. Local roots must be existing absolute directory URLs with an empty or `localhost` authority. Canonical path boundaries reject traversal and symlink escapes. HTTPS entries must be origins only (no credentials, paths other than `/`, queries or fragments); every redirect needs independent authorization. `allowQrc: true` opts into application resources. Shared policies update views live; supplied objects remain host-owned, and null/reset/destruction restores view-owned deny-all defaults.

Ready PNG/JPEG images occupy separate native rows within their paragraph or heading, preserving text order and formatting. They keep aspect ratio, shrink to available width, never upscale, and use oriented pixel sizes as logical dimensions. This release does not compose images inline. Loading, denied, invalid or failed images retain formatted descriptions. Image-description links and nested description images remain inert; an enclosing link activates only over the displayed image rectangle and cancels when dragging to scroll. HTML remains literal and triggers no requests.

The isolated per-view loader permits four fetches and one decoder, with a 15-second total deadline, five manual redirects, an 8 MiB encoded limit, 16 megapixels per image, and 64 MiB retained decoded images admitted in document order. Network requests use normal TLS validation and no cookies, supplied credentials or disk cache. Unsupported formats (including SVG, data URLs and animation), missing decoders and exhausted budgets keep descriptions. Markdown, base URL and policy changes cancel pending work and discard cached images; failed loads do not retry automatically. Qt Network is a package dependency, including static consumers. The playground's Images sample authorizes bundled qrc PNG/JPEG assets only.
