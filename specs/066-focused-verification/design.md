# Design

Approved by the user implementation request on 2026-10-23, before implementation.

A shared path classifier combines complete diff inputs, with renames expanded and unknown paths conservative. Hosted initial checks run Qt-independent tools once. Required packaging jobs retain names, skip Qt for Light, and select Core/Full through the shared container runner. Compiler cache identity includes recipe, Qt and architecture; correctness never depends on a hit. Ninja configurations retain two build workers and serial GUI tests.

Installed consumers share a configuration per relocated package but retain separate processes and linked/plugin loading modes. Exact current package and QML versions are derived from project metadata. The SameMajorVersion package policy remains unchanged.

Git hooks export immutable Git snapshots into a locked managed cache, preserving unchanged timestamps and build outputs. Classification uses each pushed commit against locally available origin/main; absent ancestry selects conservative coverage. Release evidence explicitly identifies schema and Full profile alongside existing final-head/tree/Qt/version guards. Image preparation and verification have independent deadlines; log collection runs unconditionally when artifacts exist.

## Execution details

`check-scope.py` returns additive profile/syntax/math/shipping fields. Specifications are standalone inputs even when they contain prototype CMake/C++ files. Python tooling changes select hosted Full but remain Qt-independent for local pre-push. The initial hosted job validates changed evidence and all tooling tests; CTest retains only Qt runtime and parser/CommonMark integration checks.

`verify-packaging.py` installs/relocates each selected package once. A single consumer CMake configuration creates distinct fixture executables for every retained loading mode and CTest runs them serially. Package version requests use the exact root project version; copied fixtures use the module's current QML version. The Arch verifier uses the same grouped infrastructure for its focused math/syntax/GFM processes and retains the ELF plugin-only linkage assertion and package lifecycle checks.

Full reuses the same shared OFF path as Core, adds static OFF and shared/static ON, and never schedules duplicate Core builds. Optional Core syntax coverage uses shared ON; optional math checks use the same disabled-math loop as Full. Full benchmark smoke is limited to development Qt. Compiler launchers alone use ccache; linking/test execution always occurs. The cache root can be set with `QMARKDOWN_CCACHE_DIR`; the recipe fingerprint incorporates Qt and platform.

Release evidence carries `schema=qmarkdown-ci-v2` and `profile=full`; publication rejects absent/older schemas and other profiles even if the legacy full flag is true. The release-gate job depends on the complete successful packaging matrix. Existing source archive extraction, Arch payload/ownership/removal and downloaded-byte hash validation remain publication requirements.
