# Install QMarkdown

<!-- current-release:start -->
QMarkdown 1.17.1 / QML 1.17 supports source and [Arch package](arch-install.md)
installation using the [1.17.1 downloads](https://github.com/lebedenko/qmarkdown/releases/tag/v1.17.1). Ubuntu binaries retain their
[historical installation instructions](ubuntu-install-historical.md).

## Source download and build

Download the archive and checksum from the release, then verify before extracting:

```sh
curl -fLO https://github.com/lebedenko/qmarkdown/releases/download/v1.17.1/qmarkdown-1.17.1.tar.gz
curl -fLO https://github.com/lebedenko/qmarkdown/releases/download/v1.17.1/qmarkdown-1.17.1.tar.gz.sha256
sha256sum -c qmarkdown-1.17.1.tar.gz.sha256
tar -xzf qmarkdown-1.17.1.tar.gz
cd qmarkdown-1.17.1
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build --parallel 2
cmake --install build
```

The assets are `qmarkdown-1.17.1.tar.gz` and
`qmarkdown-1.17.1-1-x86_64.pkg.tar.zst`, each with a `.sha256` companion.
They contain the library; the viewer remains in the development repository.
Follow the archive's `INSTALL.md`, also available as the
[standalone instructions](source-install.md). Requires Qt >=6.8, CMake >=3.21
and C99/C++17 compilers. Qt is not bundled.

Optional fenced-code syntax highlighting uses a private KSyntaxHighlighting backend.
It defaults off per view; source builds default the dependency off, while Arch
enables it and requires `syntax-highlighting>=6.10.0`. Hosts control a separate
Light/Dark token palette. See the [syntax guide](syntax-highlighting.md).

Code-block usability adds language-aware headers, view-wide wrapping, bounded
source viewports with internal scrolling and optional decorative source-line
numbers. Defaults retain wrapped natural-height source with numbers off. Hosts
own header composition, clipboard and scrollbar styling. See the
[code-block guide](code-blocks.md).
<!-- current-release:end -->
