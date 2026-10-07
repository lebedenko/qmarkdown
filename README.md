# qt-markdown

A standalone Qt/QML Markdown rendering library under development, independent of any other project. The library owns Markdown semantics and rendering behavior; applications supply typography and colors. Markdown source feeds a private parser, document model, and native Qt Quick block components, without document-level HTML, QTextDocument, or WebEngine rendering.

**Status:** version `0.3.0` provides `MarkdownView` and `MarkdownStyle`, paragraphs and ATX H1–H6 with emphasis, strong emphasis, inline code, and CommonMark escapes/entities, an editable viewer, and shared/static packaging. Links, images, autolinks, and raw HTML remain visible source text; nothing loads or activates. Backtick/tilde fences render as native literal code blocks with hidden delimiters, preserved whitespace and wrapped long lines. This bounded slice is not a CommonMark-conformant renderer. [Feature 004](specs/004-fenced-code-blocks/requirements.md), approved on 2026-10-07, supersedes protected-fence fallback. [Feature 003](specs/003-inline-formatting/requirements.md) supersedes Feature 001's literal inline behavior for paragraphs/headings; both were approved on 2026-10-07.

## Build and check

With [Task](https://taskfile.dev/) installed, run these shortcuts from the repository:

```sh
task build
task test
task demo
```

`build` configures `build-shared` with tests and examples enabled and builds with two parallel jobs. `test` builds first, then runs CTest. `demo` builds first, then opens the editable playground with its QML import path configured, inheriting your display environment. CMake's generator and build-type defaults are preserved.

Requires CMake ≥3.21, C99 and C++17 compilers, and Qt ≥6.8 Core, Gui, Qml, and Quick. Tests additionally require Qt Test. Local verification used Qt 6.11.2 on Linux; Qt 6.8 compatibility has not yet been executed. Dependencies are not downloaded by this project. [cmark 0.31.2](third_party/cmark/PROVENANCE.md) is bundled privately with prefixed symbols and complete [license notices](third_party/cmark/COPYING); no external cmark package is needed.

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

`QMARKDOWN_BUILD_EXAMPLES` and `BUILD_TESTING` default on for standalone builds and off when included with `add_subdirectory`. Both can be overridden. Disable tests with `-DBUILD_TESTING=OFF` to avoid requiring Qt Test. GNU install directories are supported. `QMARKDOWN_QML_INSTALL_DIR` defaults to `${CMAKE_INSTALL_LIBDIR}/qt6/qml`; for relocation, supply a relative path such as `-DQMARKDOWN_QML_INSTALL_DIR=share/qml`. Absolute destinations are supported but remain tied to that location.

## Rendering and viewer

`MarkdownView` exposes writable `markdown` and `style`, and read-only `contentHeight`. Supply its width explicitly; `implicitWidth` is zero and `implicitHeight` follows content. Text wraps, including long words; an indivisible glyph may overhang an extremely narrow view. Nonpositive width suppresses layout and reports zero height. Hosts own scrolling, clipping, backgrounds, and navigation.

`MarkdownStyle` has independent body and H1–H6 fonts/colors, plus `inlineCodeFont`, `codeBlockFont`, `codeBlockColor`, and `blockSpacing`. Defaults use the application font snapshot at pixel sizes 16 for body and 32/28/24/20/18/16 for bold headings, all colored `#202020`, with 8 pixels between blocks. Styles can be shared and edited live. Null or undefined resets the view's own style to construction defaults; deleting a supplied style restores that default. A supplied style remains host-owned. Negative spacing lays out as zero; nonfinite spacing uses 8.

Paragraphs and ATX headings support emphasis, strong emphasis, inline code, escapes, and character references. Ordinary lines are trimmed at ASCII spaces/tabs and joined with spaces before inline parsing. Emphasis enables italic and strong raises weight to at least bold. `inlineCodeFont` is an overlay on the containing block font: its default sets only the system fixed-font family, inheriting size, weight, and italic state. Surrounding emphasis/strong also apply to code. Code has no background or padding. Whole-font assignment and QML subproperty edits update the cached native layout.

Recognized HTML, links, images, and autolinks retain source spelling without rendering children or loading/activating resources. Unresolved references remain ordinary inline text; reference definitions are not collected. Unsupported block syntax stays in normalized paragraph fallback and can contain supported inline formatting. Hard-break semantics remain outside this slice. LF/CRLF/CR are equivalent and NUL becomes U+FFFD. Fenced code preserves line breaks and applies CommonMark opening indentation removal, including partial tabs. Code blocks have independent fixed-family, 16-pixel normal-weight font and `#202020` color defaults, with no background or padding. Info strings stay private and undisplayed. Empty blocks occupy one line. Replacing markdown replaces the whole document. Layout settles through Qt Quick polish; no synchronous geometry or stable block identity is promised.

Launch the editable source/preview example:

```sh
QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/examples/viewer/qmarkdown-viewer
```

The playground offers six bundled samples, live editing, independently scrollable panes with a draggable divider, Fit/240/480/720 preview widths, and collapsible role-based style controls with Neutral/Alternate presets and reset. It starts with Overview; Reload sample restores the selected source and Clear empties it. All state stays in memory. Qt Quick Controls is required only when `QMARKDOWN_BUILD_EXAMPLES=ON`; library-only builds do not require it.

## Consume an installed package

```cmake
find_package(QMarkdown 0.3 CONFIG REQUIRED)
target_link_libraries(myapp PRIVATE QMarkdown::QMarkdown)
```

```qml
import QtQuick
import QMarkdown 0.3

MarkdownView {
    width: 480
    markdown: "# Title\n\nBody with *emphasis*, **strong**, and `code`"
    style: MarkdownStyle {
        bodyFont.pixelSize: 16
        bodyColor: "#202020"
        h1Font.pixelSize: 32
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

See [fenced code verification](specs/004-fenced-code-blocks/verification.md), [inline formatting verification](specs/003-inline-formatting/verification.md), [standards and conformance](specs/standards.md), [rendering verification](specs/001-static-text/verification.md) and [scaffold verification](specs/002-project-scaffold/verification.md), [specifications](specs/README.md), [overview](specs/overview.md), [roadmap](specs/roadmap.md), and [project instructions](AGENTS.md). Authored library code is licensed under the [MIT license](LICENSE), attributed to qt-markdown contributors. The bundled cmark sources retain their own [notices](third_party/cmark/COPYING), also installed with the package. No stable API or ABI is promised by this early release.
