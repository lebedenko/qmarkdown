# 002: project scaffold verification

**Status:** Approved on 2026-10-07; implemented and locally verified on the same date. Approval scope is feature 002 requirements, design, and tasks only.

## Planned checks

| Check | Requirements / tasks | Procedure |
| --- | --- | --- |
| V1 | R1–R3, R5 / T2, T4 | Configure and build separate shared/static directories; run headless Qt Test imports and basic Qt Quick object creation in both. |
| V2 | R2–R4, R7 / T3, T4 | Install each variant into a temporary prefix; relocate the prefix and build/run an independent consumer using only the installed package. Verify static plugin registration and inspect exports for source/build paths. |
| V3 | R4 / T3 | Check installed libraries, plugin, qmldir, type information, and required artifacts; exercise a custom relative QML install destination. |
| V4 | R1, R5 / T2 | Configure subdirectory inclusion and verify tests/examples default off; configure without testing and confirm no Qt Test requirement. |
| V5 | R5, R7 / T4 | Build the import-only example and run generated QML lint checks; record warnings and errors. |
| V6 | R1, R6 / T5 | Review source/build dependencies for private Qt and application-specific dependencies; check documentation links, license attribution, status, and requirement/task/check coverage. |

## Actual records

### Task shortcuts maintenance (2026-10-07)

The user approved the Taskfile maintenance plan for the existing scaffold workflow: configure/build `build-shared` with tests/examples enabled, run CTest after building, and launch the editable viewer after building with an absolute QML import path. No generator or build type is selected by the shortcuts.

- `task --list` discovers all three described tasks; dry runs confirm configure → build → test/viewer ordering.
- A dry run with `--dir /tmp --taskfile /home/andrii/Projects/pet/qt-markdown/Taskfile.yml` confirms viewer paths remain anchored to the Taskfile directory.
- `task build` passed; `task test` passed all three CTest tests (3/3).
- `QT_QPA_PLATFORM=offscreen timeout 5s task demo` built and launched the viewer without startup diagnostics. The running GUI was stopped by the timeout (exit 124; Task reported child termination 143), as expected. This checks startup only, not visual appearance or interactive behavior.

Executed on 2026-10-07 on Linux with Qt **6.11.2**, CMake **4.4.4**, GNU C++ **16.2.1**. Qt 6.8 and CMake 3.21 execution remain unverified; minimum versions are declared requirements, not results from those versions. Windows/macOS and a fully static Qt runtime were not tested.

| Check | Actual result |
| --- | --- |
| V1 | PASS: separate shared and static builds, including examples and tests. Each variant's CTest import check passed (1/1), creating a QQuickItem with the expected width/height. |
| V2 | PASS: both packages installed, then moved to new prefixes before consumer configuration. Consumer sources were copied into a temporary directory. Both consumers built and ran with only the installed package and system Qt; generated CMake metadata contained no repository, build-directory, or original-prefix paths. The static consumer ran with no QMarkdown filesystem import path and explicit plugin registration. An additional shared consumer linked only Qt and dynamically loaded the installed plugin with LD_LIBRARY_PATH unset. |
| V3 | PASS: shared install includes versioned backing library, generated plugin, qmldir, type information, package metadata, and license. Static install additionally includes generated resource/plugin initialization objects and plugin archive. Shared uses `lib/qt6/qml`; static uses custom relative `share/qml`. ELF inspection confirmed shared plugin RUNPATH `$ORIGIN/../../../`. |
| V4 | PASS: a parent project using add_subdirectory configured and built with examples/tests off by default and no Qt6::Test target. A standalone build with testing and examples disabled also configured and built; disabling Qt6Test discovery was unused, confirming no search was requested. |
| V5 | PASS: generated all_qmllint targets exited successfully for both variants. The intentional `import QMarkdown` in the example produces one informational unused-import diagnostic; no lint warnings/errors. Both examples launched with QT_QPA_PLATFORM=offscreen, remained running for two seconds, and produced no stderr before termination. No visual inspection is claimed. |
| V6 | PASS: repository Markdown relative links resolve. Direct build/source dependencies contain no Qt private targets/headers, application-specific types, QTextDocument, or WebEngine. MIT attribution and feature approval/status boundaries reviewed. No rendering requirements are claimed as verified. |

### Reproducible packaging command

```sh
python3 scripts/verify-packaging.py
```

Final run exited 0 and printed `PASS: shared and static relocated package consumers`. Artifacts are retained at `/tmp/qmarkdown-packaging-dzrr5fh0`; full command/output log from this session is `/tmp/qmarkdown-scaffold-packaging-final.log`. The script records exact configure/build/test/lint/install/consumer commands as it runs, including explicit shared/static options and install destinations.

### Additional local commands

```sh
cmake -S . -B build-shared -G Ninja -DCMAKE_INSTALL_PREFIX=/tmp/qmarkdown-scaffold-shared
cmake -S . -B build-static -G Ninja -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=/tmp/qmarkdown-scaffold-static -DQMARKDOWN_QML_INSTALL_DIR=share/qml
cmake --build build-shared
ctest --test-dir build-shared --output-on-failure
cmake --build build-shared --target all_qmllint
cmake --build build-static
ctest --test-dir build-static --output-on-failure
cmake --build build-static --target all_qmllint
cmake -S /tmp/qmarkdown-scaffold-parent -B /tmp/qmarkdown-scaffold-parent-build -G Ninja
cmake --build /tmp/qmarkdown-scaffold-parent-build
cmake -S . -B build-no-tests -G Ninja -DBUILD_TESTING=OFF -DQMARKDOWN_BUILD_EXAMPLES=OFF -DCMAKE_DISABLE_FIND_PACKAGE_Qt6Test=TRUE
cmake --build build-no-tests
```

The temporary parent CMakeLists uses add_subdirectory on this repository and fails configuration if BUILD_TESTING, QMARKDOWN_BUILD_EXAMPLES, or Qt6::Test is enabled. Headless example launches were checked through Python subprocesses with a two-second timeout; processes were then terminated. Python also checked relative Markdown links and direct dependency boundaries.

### Fixes found during verification

Initial checks caught an incompatible `final` anonymous QObject anchor, Qt's module generator needing an explicit STATIC/SHARED argument, cache PATH normalization turning a custom relative QML destination absolute, and missing exported static resource/plugin initialization objects. These were corrected within approved scope and the final packaging run passed. The initial misplaced install artifacts were moved out of the repository to `/tmp/qmarkdown-scaffold-misplaced-artifacts`; no generated installation artifacts remain in source directories.

The shared plugin loader path was made explicitly relative to the installed backing library, including custom QML destinations. Feature 001 still needs its own approved rendering design and verification.
