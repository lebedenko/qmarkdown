# 002: project scaffold requirements

**Status:** Approved for implementation. User approval recorded on 2026-10-07: requirements, design, and tasks for feature 002 only; feature 001 was draft at that approval and has since been separately approved (see its records).

## Scope

Create a buildable, installable Qt/QML library skeleton. This feature provides packaging and import integration only; feature 001 was draft at that approval and has since been separately approved (see its records).

| ID | Requirement and acceptance criterion |
| --- | --- |
| R1 | Build project version `0.1.0` with CMake ≥3.21, C++17, and Qt ≥6.8. Use only public Qt Core, Gui, Qml, and Quick APIs; require Qt Test only with testing enabled. |
| R2 | Support `import QMarkdown`, `find_package(QMarkdown CONFIG REQUIRED)`, and target `QMarkdown::QMarkdown`. Expose no public rendering types. |
| R3 | Default to shared libraries and support `BUILD_SHARED_LIBS=OFF`. Export the static plugin as `QMarkdown::QMarkdownPlugin` and document explicit linking and plugin registration. |
| R4 | Install relocatable CMake metadata, libraries, plugin, QML metadata, and required module artifacts using GNU install directories. Provide a configurable QML install directory defaulting to `${CMAKE_INSTALL_LIBDIR}/qt6/qml`. |
| R5 | Provide an optional Qt Quick example that imports the module and displays scaffold status. Examples and tests default on for standalone builds and off for subdirectory inclusion. |
| R6 | Add MIT licensing attributed to “qt-markdown contributors,” build-directory ignore rules, and build/install/consumer documentation. Clearly distinguish scaffold availability from draft rendering. |
| R7 | Verify shared/static builds, headless imports, QML lint, and separately built installed-package consumers, including static plugin registration without source-tree paths. Record actual results and limitations. |

## Boundaries

No parser, AST, `MarkdownView`, styling API, streaming, resource policies, extensions, Git initialization, CI setup, or dependency downloads. The temporary briefs are design input only. This scaffold does not grant feature 001 implementation approval.
