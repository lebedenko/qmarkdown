# Qualification continuation — actual results

Date: 2026-10-21. Workspace: `/home/andrii/Projects/pet/qmarkdown`, branch
`feat/hybrid-text-rendering`, base `bbb0d0c1985567c4f1a7b71f82445c8fafd6ccf7`.
This record covers fresh builds of the transferred checkout. [Earlier results](verification.md)
remain historical. The owner approved this continuation, including the warning fix and isolated
acquisition, in the implementation request; no commit, publication or dependency adoption is included.

Status: the approved automated qualification is complete and passes. The change is reviewable
as uncommitted local work. Perceived shimmer and owner visual acceptance remain unassessed;
no integration, release or viewer adoption was performed.

## Implementation and reproduced failure

Fresh Release configuration:

```sh
cmake -S . -B build/qualification-064/host -DBUILD_TESTING=ON \
  -DQMARKDOWN_BUILD_EXAMPLES=ON -DQMARKDOWN_WITH_KSYNTAXHIGHLIGHTING=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/qualification-064/host --parallel 2
```

Host: Qt/QtTest 6.12.0, KDE Syntax Highlighting 6.31.0-1. Full host tool versions are
recorded in `build/qualification-064/host-versions.json`; each CI run retains `tool-versions.txt`. The initial
`qmarkdown-view-test playgroundReadingControls` failed (2 passed, 1 failed) with
`QObject::installEventFilter(): Cannot filter events for objects in a different thread.`
The initial real asynchronous syntax test emitted the same warning. Runs used
`QT_QPA_PLATFORM=offscreen`, software rendering and the freshly built QML import path.
Logs: `build/qualification-064/playground-before.log`, `syntax-before.log`, `build-before.log`.

Installed-library disassembly (`kde-constructor.txt`) confirms `Repository::Repository()`
loads `QCoreApplication::self` and calls `QObject::installEventFilter(QObject*)` with the new
repository. This independently confirms the historical worker-construction backtrace.

The scheduler now lazily prepares its real backend on the application thread before the first
queued job. The private factory constructs the repository, removes its application event filter,
and moves it to the worker. Token roles need no translation-change notification. Queued processing
and backend destruction remain on the worker, with no worker-to-GUI blocking call. Token generation,
limits, cancellation, caching and delivery are unchanged; fake backend construction stays on the
worker. No public API, package version or dependency pin changed.

`SyntaxTest::realBackendLifecycle` treats warnings as failures across two scheduler lifetimes,
checks asynchronous real highlighting, submits work before cancellation and joins worker teardown.
After rebuilding, this regression and `playgroundReadingControls` each pass (3/3).
Logs: `real-backend.log`, `playground-after.log`, `build-after.log`.

## Fresh host correctness and consumers

All logs below are under `build/qualification-064/`.

| Check | Actual result | Evidence |
| --- | --- | --- |
| `QT_QUICK_BACKEND=software QT_QUICK_CONTROLS_STYLE=Fusion ctest --test-dir build/qualification-064/host --output-on-failure` | 34/34 passed | `software-tests.log` |
| Native Wayland OpenGL and Vulkan, requested DPR 1/1.25/1.5/2 | All eight runs passed; runtime-reported DPR equals each request; QRhi logs confirm requested backend | `native.py`, `native-summary.log`, `native-exact-dpr/results.json` and per-run logs |
| Native selection, keyboard, reading/Find, syntax-view, codeblocks, math, footnotes, renderers | All eight suites passed | `native-features.py`, `native-features/results.json` and per-suite logs |
| Fresh `md-viewer` Debug build against isolated installed candidate | 68/68 passed | `viewer-configure.log`, `viewer-build.log`, `viewer-tests.log` |
| Final working-byte source archive, host build/install and executed consumer | Passed | `distribution-check.log`, `distribution-consumer-{configure,build,confirmed}.log` and `distribution-consumer-result.json` |

Native checks include stock-text comparison, fractional placement, overlay foreground restoration,
surface transitions, window recreation and scene-graph lifecycle. `native.py` derives scale factors
from this display's native DPR 1.5 and rejects any runtime DPR mismatch. These results do not imply
coverage of live monitor migration or arbitrary transformations. Native logs retain incidental
Wayland text-input focus warnings and a point-size/pixel-size warning; these suites do not globally
fail on warnings. The dedicated real-backend regression and playground test do, and no
`installEventFilter` warning remains in the fresh post-fix results.

Viewer commands use `cmake -S ../md-viewer -B build/qualification-064/viewer -G Ninja`, Debug,
`BUILD_TESTING=ON`, and both `CMAKE_PREFIX_PATH` and `QMarkdown_DIR` pointing at
`build/qualification-064/candidate-prefix`. Build: `cmake --build ... --parallel 4`;
test: `QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software ctest --test-dir ... --output-on-failure`.
The viewer source, lock/pin and installed packages are unchanged. QMarkdown owns the renderer.

The archive was produced using `build_archive(Path.cwd(), output)` from
`scripts/build-source-archive.py`, with no commit argument, so it contains working-tree bytes.
Archive: `distribution/qmarkdown-1.17.0.tar.gz`;
SHA-256: `236a40263f0b543dfc6138794d7ebf241473ee9bf7426885f41c2fc2d3503af4`.
This isolated candidate must not replace the published archive of the same version.
`verify-source-archive.py` used `--qt-root /usr --syntax-highlighting ON --kde-prefix /usr`.
The consumer was built from `tests/installed-consumer` with `QMARKDOWN_PACKAGE_PREFIX` restricted
to `distribution-check/install`, then its executable was run directly with `QML_IMPORT_PATH` set to that prefix’s
`lib/qt6/qml` and exited 0. Import tracing confirms candidate QML resolution. The first host
run did not explicitly set the QML path and is not used as isolated-consumer evidence.

## Exact toolchain acquisition and matrix

`task sync-qt` reports 6.12.0 and leaves the configured exact Qt targets unchanged.
Initial `task ci-minimum` and `task ci-development` attempts failed before provisioning:
Docker lacked buildx, so its legacy builder rejected `--progress`. Sandbox Docker access also
required escalation. These failures remain in `ci-*-launch.log` and
`ci-minimum-unrestricted.log`; they are not matrix results.

Acquired official Docker buildx v0.38.1 in `build/qualification-064/docker/cli-plugins/`, verified
against the release checksum file, then reran the unchanged existing tasks using
`DOCKER_CONFIG="$PWD/build/qualification-064/docker"`. No installed package or user Docker
configuration was changed. Binary SHA-256:
`e25d5cd675092bb5d1d03632fdc46cdf3fede7b514dc5f023a105e00d3ce5ad4`.
Metadata, checksums and binary are retained in the ignored qualification directory.

| Task | Isolated run directory | Result |
| --- | --- | --- |
| `task ci-minimum` | `build-ci/6.8.0/20261021T192052Z-o16igp8g` | Passed, exit 0 |
| `task ci-development` | `build-ci/6.12.0/20261021T192104Z-y1lke4s9` | Passed, exit 0 |

The existing provisioning recipe acquires exact Qt 6.8.0/6.12.0 and pinned KDE 6.10.0 with
recorded provenance. Both tasks completed with `status: passed`, `exitCode: 0` in their retained `result.json`.
Each SDK passes four full 34-test suites (syntax OFF/ON × shared/static), relocated installed
consumers, both syntax variants' library-only builds, math-disabled shared/static tests and
benchmark smoke. All four syntax-enabled full-suite logs explicitly pass `realBackendLifecycle`
and `playgroundReadingControls`, with no `QObject::installEventFilter` warning.

All 446 build/test/tooling inputs in each retained snapshot match the final working tree
(`final-ci-inputs.json`). Final documentation-only additions are checked separately below.
`environment.json` records the exact image identity and command; image preparation, Qt/KDE
provenance, `ci.log` and `result.json` remain in each isolated run directory. The launch logs are
`ci-minimum-final.log` and `ci-development-final.log` under the host qualification directory.
Additional final-byte source-distribution builds on both exact SDKs pass. The first installed
consumer executions failed because the ad hoc harness omitted the QML import path (module not
installed). After setting `QML_IMPORT_PATH=/artifacts/archive/install/lib/qt6/qml`, both existing
consumer executables exit 0, with import tracing confirming the candidate prefix. No build inputs
changed or rebuild was needed. `source-containers.py` and `source-consumers.py` retain the commands;
`source-qt-{6.8.0,6.12.0}/run.log`, `result.json`, `consumer-isolated.log` and `consumer-result.json`
preserve failed attempts and successful corrections separately.

## Reused measurement evidence and preservation

No renderer, fixture or measurement input changed. `source-identities.json` compares all 53
production files against the archived candidate: only `syntaxbackend.cpp`, `syntaxhighlighter.cpp`
and `syntaxhighlighter.h` differ. Renderer identities remain:

- `formattedtext.cpp`: `1dbb64e36930163cf3677bd95afd0d4854b10f5fee05b06626a91e259958f4e1`
- `formattedtext.h`: `b796909a3b00b807efcd64dd726f62b11b7b2713e8d8faf36ff0feefd35cf09b`

The fixed 28-trial replay uses the direct host in `run.py`/`Compare.qml`, without enabling syntax
highlighting; the 24-cycle memory harness likewise uses default highlighting-disabled options.
`ViewState::m_syntaxEnabled` remains false by default. The changed backend is therefore not exercised
by these measurements. Historical replay/lifetime evidence is reused without timing reruns, with
its original scope and limitations; it is not presented as a measurement of newly built binaries.
The identity record also hashes the retained harness, fixtures, results and measured-binary manifest.

All 4,782 archived manifest entries pass SHA-256 verification (`historical-integrity.log`). All
1,158 preservation fingerprints still match (`preservation-check.json`), including original
candidate/evidence, viewer production/dependency files and installed-provider/viewer files.

## Remaining limits

Performance qualification does not establish a speedup or resolve perceived shimmer. Owner visual
acceptance, live monitor migration, exotic scales/transforms and screen-reader sessions remain
unassessed. Bounded historical process-memory and node-lifetime evidence does not measure individual
GPU allocations or prove absence of every leak. There are no missing required automated
qualification environments after this continuation.

Final `python3 scripts/check-docs.py`: passed (1.17.0 documentation and local links).
Final `git diff --check` and explicit whitespace checks of all untracked Feature 064 Markdown:
passed. Evidence: `build/qualification-064/docs-final.log` and `final-checks.json`.
All 297 source-archive payload files match final working-tree bytes (`archive-final-bytes.json`).

## Changed files

Transferred renderer and regressions remain unchanged from the archived candidate:
`src/QMarkdown/private/formattedtext.{cpp,h}`, `private.qmltypes`,
`tests/tst_{codeblocks,math,renderers,styling}.cpp`.
This continuation changes `src/QMarkdown/private/syntaxbackend.cpp`,
`syntaxhighlighter.{cpp,h}`, `tests/tst_syntax.cpp`, and Feature 064's requirements, design,
tasks, historical-result navigation and handoff, plus `specs/README.md` navigation; this
actual-results record is new.
All changes remain uncommitted local work.

## Subsequent local commit

After the verification recorded above, the owner explicitly authorized a local commit. The
uncommitted-work statements above describe the completed qualification snapshot. Only this
authorization note and its requirements record were added before committing; implementation and
test inputs remain unchanged. Documentation and whitespace were checked again.
