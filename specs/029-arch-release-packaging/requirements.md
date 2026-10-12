# Requirements

Approved on 2026-10-12 by the explicit instruction to implement the supplied Arch Linux release package plan. Approval covers these requirements, design and mapped tasks; it excludes publication or modification of v1.1.0 assets.

- R1: Produce qmarkdown-<CMake version>-1-x86_64.pkg.tar.zst and SHA-256 from an exact Git source commit, using unprivileged makepkg in disposable, fully updated Arch containers.
- R2: Install only the shared CMake install payload, with system library/QML paths, lowercase qmarkdown license directory, runtime dependency lower bounds matching build Qt, and no installed RPATH or debug package.
- R3: Gate future stable uploads on Ubuntu and Arch verification; validate both artifact/checksum pairs and preserve identical-bytes reruns and refusal to overwrite differing assets.
- R4: Verify build tests, lint, parser symbols, package metadata/payload, namcap, actual pacman installation/ownership/integrity, installed consumers, higher-pkgrel upgrade and removal. Retain environment and failure evidence.
- R5: Document checksum, installation, inspection, ownership, removal and local building. Public APIs and QML versions stay unchanged. No host installation, viewer package, AUR, signing or repository hosting.
