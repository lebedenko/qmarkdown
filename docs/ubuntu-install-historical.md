# Historical Ubuntu binary packages

The archive targets Ubuntu 24.04 x86_64 and a compatible Qt 6.8 runtime. Qt is
not included. Provide Qt Core, Gui, Qml, Quick and Network and their Ubuntu
system dependencies. Python 3.9+ and the standard Ubuntu `readlink`, `sudo`
and `ldconfig` tools are required for installation. Development consumers also
need CMake >=3.21, a C++17 compiler and the matching Qt development SDK.
The same Qt 6.8.0-built archive is tested with exact Qt 6.8.0 and 6.12.0; this is specific
verification evidence, not a guarantee for every newer Qt or distribution.

For the existing 1.5.0 release, download the archive and its `.sha256` companion from the [v1.5.0 GitHub release](https://github.com/lebedenko/qmarkdown/releases/tag/v1.5.0).
Verify before extracting, then run from the enclosing package directory:

```sh
sha256sum -c qmarkdown-1.5.0-ubuntu24.04-x86_64-qt6.8.tar.gz.sha256
tar -xzf qmarkdown-1.5.0-ubuntu24.04-x86_64-qt6.8.tar.gz
cd qmarkdown-1.5.0-ubuntu24.04-x86_64-qt6.8
sudo ./install.sh
```

Installation uses `/usr/lib/x86_64-linux-gnu` and
`/usr/lib/x86_64-linux-gnu/qt6/qml/QMarkdown`, with licenses under
`/usr/share/licenses/QMarkdown`. Ensure your Qt runtime libraries are discoverable
by the system linker (for an external SDK, configure its library directory in
ld.so.conf and run ldconfig). Do not mix incompatible Qt libraries in one process.

CMake discovers `/usr/lib/x86_64-linux-gnu/cmake/QMarkdown` through its normal
system search. Use `find_package(QMarkdown 1.5 CONFIG REQUIRED)` and link
`QMarkdown::QMarkdown`. For an external Qt SDK, configure `CMAKE_PREFIX_PATH`
with its root. Import `QMarkdown 1.5` in QML for all three optional extensions. Existing
QML 1.0–1.4 imports and CMake 1.0–1.4 requests remain supported with Legacy defaults;
Exact requests for earlier 1.x versions require the historical package. If your Qt installation does not
search Ubuntu's QML directory, set:

```sh
export QML_IMPORT_PATH=/usr/lib/x86_64-linux-gnu/qt6/qml
```

This is a manually owned package, not a dpkg package. Do not install a competing
package over these files. The installer validates all payload hashes, permissions,
links and destinations before copying; its ownership record is
`/usr/share/qmarkdown/installed.json`. Keep the extracted package for removal:

```sh
sudo ./uninstall.sh
```

Repeat installation and upgrades require every previously owned file to match its
record. Unrelated conflicts, missing owned files and local modifications cause
installation to stop before changing anything. Upgrades remove obsolete unchanged
owned files. Uninstall preserves modified files with a diagnostic and retains their
record; it removes only unchanged files and never recursively removes directories.
Real operations refresh the linker cache. Back up local edits before resolving a
conflict. Do not manually remove the record while owned files remain installed.

For unprivileged staging, use `./install.sh --destdir /absolute/temporary/root`
and the corresponding uninstall command. The `/usr` layout remains inside that
root; staging does not update the host linker cache.

Alternatively, build from source using the repository README's CMake instructions.
Source builds support Qt >=6.8 and custom install prefixes; they are the preferred
option on other distributions or toolchains.

These instructions apply to existing Ubuntu assets, including the released 1.6.0 archive (substitute 1.6.0 in filenames). Future delivery tooling produces source and Arch packages. Historical releases, assets and tags are preserved.
