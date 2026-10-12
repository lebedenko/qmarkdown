# Design

Approved with the supplied plan on 2026-10-12; see requirements.md.

A Python host runner resolves a commit and derives its version from that commit's CMakeLists.txt, then uses git archive with an enclosing source directory and computes its checksum. The runner pulls the current archlinux:base image, records its immutable identity, and uses that identity for both disposable x86_64 containers. Each container fully updates its repositories. A run-local download cache is shared between the containers; installed roots remain fresh. Packaging helpers are snapshotted before provisioning so working-tree edits cannot change an active run. No host package operations occur.

The build container installs tools, queries distribution Qt, renders a PKGBUILD template and runs makepkg as an unprivileged account. Release/shared CMake configuration enables tests, disables examples/benchmarks, installs under /usr/lib and /usr/lib/qt6/qml, and suppresses installed RPATH. The package function moves only CMake-installed licenses to /usr/share/licenses/qmarkdown. Runtime dependencies are glibc, gcc-libs, qt6-base and qt6-declarative with build Qt lower bounds. CMake/Ninja are build dependencies and Python, Git and Noto fonts are check dependencies. !debug produces one package.

The fresh verification container installs the artifact through pacman, inspects metadata/payload, validates ownership and integrity, ELF dependencies and installed paths, and builds all four existing installed-consumer variants with system discovery and no custom runtime search paths. A temporary higher-pkgrel copy is repackaged and installed for upgrade/removal checks; only the original revision is exported. namcap errors fail verification; warnings remain evidence.

The release workflow retains Arch artifacts/evidence on manual runs and releases. Upload depends on both distribution gates and validates checksums for exactly one archive of each format before any remote mutation. Existing published bytes remain immutable. Rolling repositories are intentionally current, not byte-reproducible; image and installed-package evidence describe each run.
