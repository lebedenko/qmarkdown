# Install QMarkdown

<!-- current-release:start -->
QMarkdown 1.7.0 / QML 1.7 is available as source and an [Arch package](arch-install.md)
in the [1.7.0 downloads](https://github.com/lebedenko/qmarkdown/releases/tag/v1.7.0). Ubuntu binaries retain their
[historical installation instructions](ubuntu-install-historical.md).

## Source download and build

Download the archive and checksum from the release, then verify before extracting:

```sh
curl -fLO https://github.com/lebedenko/qmarkdown/releases/download/v1.7.0/qmarkdown-1.7.0.tar.gz
curl -fLO https://github.com/lebedenko/qmarkdown/releases/download/v1.7.0/qmarkdown-1.7.0.tar.gz.sha256
sha256sum -c qmarkdown-1.7.0.tar.gz.sha256
tar -xzf qmarkdown-1.7.0.tar.gz
cd qmarkdown-1.7.0
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build --parallel 2
cmake --install build
```

The assets are `qmarkdown-1.7.0.tar.gz` and
`qmarkdown-1.7.0-1-x86_64.pkg.tar.zst`, each with a `.sha256` companion.
They contain the library; the viewer remains in the development repository.
Follow the archive's `INSTALL.md`, also available as the
[standalone instructions](source-install.md). Requires Qt >=6.8, CMake >=3.21
and C99/C++17 compilers. Qt is not bundled.
<!-- current-release:end -->
