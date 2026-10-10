# QMarkdown 1.0.0

First stable release of the independent native Qt Quick Markdown library. Markdown is parsed privately with bundled cmark 0.31.2 and rendered through native blocks and inline layout. Host applications own typography, colors, scrolling, navigation and resource authorization. All 652 pinned CommonMark examples pass parser and model semantic checks; native presentation has separate representative verification.

## Supported environments and compatibility

Source support: Linux amd64, Qt >=6.8, C++17, CMake >=3.21. Verification is pinned to Qt 6.8.0 and 6.11.3. Documented MarkdownView, MarkdownStyle, MarkdownResourcePolicy QML properties, signals and behavior, and CMake integration remain compatible throughout 1.x. Implementation classes and QMarkdown.Private are private; no public C++ headers or C++ ABI guarantee.

The binary archive targets Ubuntu 24.04 x86_64, is built with Qt 6.8.0, and is tested with the same binary on Qt 6.11.3. Qt is not bundled; this is specific tested compatibility, not a guarantee for other distributions or runtimes.

## Installation

Download `qmarkdown-1.0.0-ubuntu24.04-x86_64-qt6.8.tar.gz` and its `.sha256` companion from this release. Assets appear after the publication workflow completes.

```sh
sha256sum -c qmarkdown-1.0.0-ubuntu24.04-x86_64-qt6.8.tar.gz.sha256
tar -xzf qmarkdown-1.0.0-ubuntu24.04-x86_64-qt6.8.tar.gz
cd qmarkdown-1.0.0-ubuntu24.04-x86_64-qt6.8
sudo ./install.sh
# Remove using the retained extracted package:
sudo ./uninstall.sh
```

Provide compatible Qt Core, Gui, Qml, Quick and Network libraries and Ubuntu dependencies, Python 3.9+, readlink, sudo and ldconfig. Development requires the matching Qt SDK, CMake and a C++17 compiler. The installer tracks its own files and refuses conflicting or modified files before installation; uninstall preserves modified files. This is a manually owned package.

See [complete binary installation instructions](https://github.com/lebedenko/qmarkdown/blob/v1.0.0/docs/release-install.md) for runtime discovery, ownership and staging. Alternatively, build shared or static libraries using the [README](https://github.com/lebedenko/qmarkdown/blob/v1.0.0/README.md).

## Migration from 0.7

Change `import QMarkdown 0.7` to `import QMarkdown 1.0` (or an unversioned import), and `find_package(QMarkdown 0.7 CONFIG REQUIRED)` to `find_package(QMarkdown 1.0 CONFIG REQUIRED)`. Link `QMarkdown::QMarkdown`. No legacy 0.x aliases remain. 1.0.0 accepts CMake requests for 1.0 and exact 1.0.0; it rejects 0.7, 1.1 and 2.0.

## Accepted limitations

Images occupy separate rows and support only authorized PNG/JPEG resources; resources are denied by default. HTML remains literal. Markdown changes replace the full document model. Indivisible glyphs can overhang extremely narrow widths. Codecs cannot be interrupted; two occupied decoder workers can delay fresh work, and application shutdown can wait for codecs. GFM extensions, streaming, selection/copy, renderer extensions, further optimization and broader platform support remain deferred.
