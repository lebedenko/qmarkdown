# Build and install QMarkdown from source

Requires CMake >=3.21, C99 and C++17 compilers, and Qt >=6.8 development
components Core, Gui, Qml, Quick and Network. The private cmark parser and its
licenses/provenance are bundled; no downloads or external cmark package are needed.

Verify the archive's SHA-256 companion before extracting. From the extracted
`qmarkdown-X.Y.Z` directory:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build --parallel 2
cmake --install build
```

For an external Qt SDK add `-DCMAKE_PREFIX_PATH=/path/to/Qt/gcc_64` when configuring.
The archive builds only the library. Tests, benchmarks and the example viewer
are available in the development repository. Use `-DBUILD_SHARED_LIBS=OFF` for
a static library. No public C++ headers or ABI are provided.

CMake consumers use `find_package(QMarkdown 1.12 CONFIG REQUIRED)` and link
`QMarkdown::QMarkdown`. Static consumers also link `QMarkdown::QMarkdownPlugin`.
Set `CMAKE_PREFIX_PATH` to your installation prefix. Import `QMarkdown 1.12` in QML.
The default QML installation directory is `lib/qt6/qml` (GNUInstallDirs may choose
another library directory). Add that directory under your prefix to
`QML_IMPORT_PATH` if your Qt installation does not search it. The platform linker
must also find the installed shared libraries. A relative
`-DQMARKDOWN_QML_INSTALL_DIR=share/qml` supports custom relocatable layouts.

Licenses install under `share/licenses/QMarkdown`. CMake does not track file
ownership or provide uninstall; use a dedicated prefix or your system package
manager for managed installation and removal.

Optional syntax highlighting requires KF6SyntaxHighlighting >=6.10.0 and
`-DQMARKDOWN_WITH_KSYNTAXHIGHLIGHTING=ON`. It defaults OFF; disabled builds do not
search for KDE. Enabled static consumers need the dependency CMake package.
KSyntaxHighlighting remains external with its own SPDX license notices and syntax
resources; no definitions are bundled in QMarkdown. See the upstream
[6.10.0 sources](https://invent.kde.org/frameworks/syntax-highlighting/-/tree/v6.10.0).
