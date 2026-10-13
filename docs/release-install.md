# Install QMarkdown

QMarkdown 1.6.0 / QML 1.6 is released. Build from its source tag or use its
[Arch package](arch-install.md). Existing Ubuntu binaries retain their
[historical installation instructions](ubuntu-install-historical.md).

## Source build

For the existing release, obtain the source checkout at tag `v1.6.0`:

```sh
git clone --branch v1.6.0 --depth 1 https://github.com/lebedenko/qmarkdown.git
cd qmarkdown
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DQMARKDOWN_BUILD_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build --parallel 2
cmake --install build
```

Future release assets are `qmarkdown-X.Y.Z.tar.gz` and
`qmarkdown-X.Y.Z-1-x86_64.pkg.tar.zst`, each with a `.sha256` companion.
They contain the library; the viewer remains in the development repository.
Verify downloaded assets before use:

```sh
sha256sum -c qmarkdown-X.Y.Z.tar.gz.sha256
tar -xzf qmarkdown-X.Y.Z.tar.gz
```

Follow the archive's `INSTALL.md`, also available as the
[standalone build/install instructions](source-install.md). Source builds require
Qt >=6.8, CMake >=3.21 and C99/C++17 compilers. Qt is not bundled.
No historical release assets are replaced by this workflow change.
