# Arch Linux release package

<!-- current-release:start -->
QMarkdown 1.12.0 / QML 1.12 uses the package and checksum below.

[QMarkdown 1.12.0](https://github.com/lebedenko/qmarkdown/releases/tag/v1.12.0)
provides `qmarkdown-1.12.0-1-x86_64.pkg.tar.zst` with a SHA-256 companion.
Pacman enforces Qt lower bounds matching the package's build version. Qt and
the development viewer are not bundled.

Download the package and checksum from the release, then verify and install:

```sh
curl -fLO https://github.com/lebedenko/qmarkdown/releases/download/v1.12.0/qmarkdown-1.12.0-1-x86_64.pkg.tar.zst
curl -fLO https://github.com/lebedenko/qmarkdown/releases/download/v1.12.0/qmarkdown-1.12.0-1-x86_64.pkg.tar.zst.sha256
sha256sum -c qmarkdown-1.12.0-1-x86_64.pkg.tar.zst.sha256
sudo pacman -U ./qmarkdown-1.12.0-1-x86_64.pkg.tar.zst
pacman -Qi qmarkdown
pacman -Qo /usr/lib/libQMarkdown.so
```

Pacman owns the shared library, QML plugin/metadata, CMake configuration and
licenses, and tracks runtime dependencies. Locations are `/usr/lib`,
`/usr/lib/qt6/qml/QMarkdown`, `/usr/lib/cmake/QMarkdown` and
`/usr/share/licenses/qmarkdown`. Arch's Qt discovers these paths normally.
Development consumers need CMake and a C++17 compiler. Use
`find_package(QMarkdown 1.12 CONFIG REQUIRED)` and `import QMarkdown 1.12`.
Install a later package with `pacman -U`; remove with `sudo pacman -R qmarkdown`.


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

## Development distribution building

From a development checkout with Python 3, Docker and daemon access:

```sh
python3 scripts/build-source-archive.py --source-commit HEAD --work-dir /tmp/qmarkdown-source
python3 scripts/build-arch-package.py --source-archive /tmp/qmarkdown-source/qmarkdown-1.7.0.tar.gz --work-dir /tmp/qmarkdown-arch
```

Use new output directories and substitute the version being built. The source
archive is library-only and comes from the selected commit's explicit allowlist.
The Arch runner consumes those exact source bytes and their checksum. Packaging
and installed-consumer tooling comes from the development checkout.

Disposable x86_64 Arch containers update rolling repositories. makepkg runs
unprivileged, builds and installs the library without rerunning the full test
suite. Focused checks cover metadata, dependencies, payload, namcap, root/pacman
ownership, integrity, linked/plugin-only consumers and removal once. Logs remain
in the output directory and Actions; only package/checksum files are published.
Recorded container identity does not promise reproducible rolling-Arch binaries.
No AUR submission, repository hosting or signing infrastructure is included.
