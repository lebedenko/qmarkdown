# 002: project scaffold design

**Status:** Approved for implementation. User approval recorded on 2026-10-07: requirements, design, and tasks for feature 002 only; feature 001 was draft at that approval and has since been separately approved (see its records).

## Build and module

Use a root CMake project named `QMarkdown`, version `0.1.0`, with GNUInstallDirs and CMakePackageConfigHelpers. Set C++17 and require Qt 6.8 Core, Gui, Qml, and Quick. Respect `BUILD_SHARED_LIBS`, defaulting it to on only when unspecified. Define `QMARKDOWN_BUILD_EXAMPLES` and `BUILD_TESTING` with defaults determined by standalone versus subdirectory configuration; find Qt Test only when tests are enabled.

Use `qt_add_qml_module` for the `QMarkdown` backing target and its generated `QMarkdownPlugin` plugin, URI `QMarkdown`, module version `0.1`. Keep a minimal private C++ module anchor so generated registration has a backing library; expose no public C++ headers or rendering QML types. Use Qt's generated plugin tooling rather than a custom plugin implementation. The anchor may be an internal, uncreatable QObject type if required by Qt's registration tooling.

Export the backing library as `QMarkdown::QMarkdown`. For static builds, export the generated plugin as `QMarkdown::QMarkdownPlugin`; consumers link both targets and explicitly register the generated plugin with `Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)`. Validate the actual generated class name during implementation. The shared plugin resides in the installed QML module directory and loads the backing library through the platform loader; document runtime library and QML import path setup for temporary prefixes.

## Installation and consumers

Install backing libraries under GNU library/runtime directories, CMake package files under `${CMAKE_INSTALL_LIBDIR}/cmake/QMarkdown`, and the plugin plus generated `qmldir`, type information, and any required module artifacts under `QMARKDOWN_QML_INSTALL_DIR/QMarkdown`. Default `QMARKDOWN_QML_INSTALL_DIR` to `${CMAKE_INSTALL_LIBDIR}/qt6/qml`; support relative destinations for relocation. Package config uses `find_dependency` for public Qt dependencies and imports exported targets without source/build paths. Apply library version `0.1.0` and major SOVERSION `0`; no stable rendering ABI is promised.

Use `examples/import-only/` for a small Qt Quick application with a status message and module import. Use `tests/` for a Qt Test case creating a basic Qt Quick object from QML that imports QMarkdown, and an independent consumer fixture using only installed package targets. Run headless with `QT_QPA_PLATFORM=offscreen`. Static hosts explicitly register the plugin; shared hosts locate the installed QML module and runtime libraries through documented paths.

## Documentation

Add the MIT license and build ignores. Update README, specification index, overview, roadmap, and feature 001 references as necessary to acknowledge scaffold decisions while preserving feature 001's draft status and unresolved parser/rendering contracts. After approval, update the documentation-only initialization statement in AGENTS.md to distinguish the approved scaffold from unapproved rendering.

This uses standard Qt/CMake mechanisms to avoid an additional packaging abstraction. No third-party dependencies or rendering framework are introduced.

## Implemented details

The private anchor uses `QML_ANONYMOUS`, so no named public type is exported. Module linkage type is passed explicitly to Qt's module generator. The QML installation option is a cache string to preserve command-line relative paths. Static installation includes Qt-generated resource and plugin initialization object targets, with their installed objects exported alongside the archives. Shared plugins use a loader-relative path to the backing library on Linux/macOS. These are implementation details within the approved packaging scope.

`scripts/verify-packaging.py` retains temporary artifacts and tests a copied installed consumer fixture; shared checks also run a host linked only to Qt to exercise dynamic plugin loading. Local execution is Linux-only; other platforms have not been verified.
