# Verification

Approved on 2026-10-09 by the user's explicit approval of requirements, design and T1–T4. Planned checks below are separate from actual observations and results.

## Planned checks

- Task command listing/dry runs select Qt 6.8.0 and 6.11.3 and the same wrapper used by GitHub.
- Python compilation and shell syntax checks; narrow argument/error-path checks; workflow inspection for matrix, pins, timeout and artifact paths.
- Inspect Docker invocation for the pinned image, amd64 platform, read-only source mount, dedicated writable outputs and absence of host Qt/environment inputs.
- Run `task ci-6.8` and `task ci-6.11`. Each must pass complete packaging and validated benchmark smoke execution inside the common Ubuntu container.
- Check retained environment/log/report evidence and invoking-user artifact ownership. Verify a deliberately failing container command returns nonzero and retains logs without weakening real checks.
- Run `git diff --check` and review final changes.

## Actual observations before implementation

The current workflow runs directly on a hosted Ubuntu 24.04 VM. Task and Docker are installed locally; act and Podman are not present in PATH. Docker daemon version 29.8.2 was confirmed through an approved read-only command; the sandbox itself denies daemon access. No container verification has run for this feature, and no production/task/workflow implementation has been changed.

## Implementation checks (2026-10-09)

- The user approved requirements, design and T1–T4 before implementation. The wrapper and both Task aliases now share the workflow's Docker verification path. The existing `/build*/` rule excludes all local outputs, so no ignore-file change was required.
- `docker buildx imagetools inspect ubuntu:24.04` resolved the official `linux/amd64` manifest to `sha256:f610ab94648195aa356059f5b41d6085c9d4d903c072430cdd1af7bdb646106b`. The wrapper pins that exact manifest, rather than the mutable tag or an approximate hosted-runner image.
- `python3 -m unittest discover -s tests -p test_ci_runner.py -v`: **five tests passed**. A temporary Git fixture verifies modified tracked files, nonignored untracked files and tracked deletions; ignored builds/configuration/credential fixtures stay out. A fake Docker CLI returns 17, exercising actual subprocess log capture and nonzero result persistence. Other checks reject old evidence and unsupported versions, exclude custom in-repository artifact directories from source inputs, and verify timeout status 124 and stopping only this invocation's named container. Tests assert read-only mounts for both source aliases.
- `bash -n scripts/ci-container.sh`, `python3 -m py_compile scripts/run-ci.py`, Task listing/dry runs and `git diff --check`: **passed**. YAML checks with available PyYAML confirmed identical Qt versions, shared wrapper invocation, triggers, permissions, 35-minute limit, full action pins and failure-artifact paths. Neither actionlint nor ShellCheck is installed; those checks were unavailable.

## Initial Ubuntu findings

The first complete container attempts passed shared CTest on both Qt versions and reached installed consumer configuration, where Ubuntu's CMake 3.28.3 exposed an existing package relocation defect. `find_dependency(Qt6)` replaced `PACKAGE_PREFIX_DIR`, causing `QMarkdown_QML_IMPORT_PATH` to point inside the SDK. The configuration template now resolves/checks its QML path before loading dependency configurations, preserving the original package contract without changing the library API or dependencies. This older-CMake behavior is documented by [CMakePackageConfigHelpers](https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html).

Initial failed evidence is retained under `build-ci/6.8.0/20261008T213558Z-aiw2m7rt/` and `build-ci/6.11.3/20261008T213608Z-_qhnerfz/`. Two subsequent pre-fix snapshots were stopped after this diagnosis rather than completing known-invalid runs. Final checks use fresh snapshots with the corrected package configuration and both source aliases mounted read-only.

## Final container results (2026-10-09 local date)

Both exact Task commands completed successfully in Docker on the Linux host. Artifact directory timestamps are UTC (2026-10-08); this record uses Europe/Kyiv's local date. The container environment was Ubuntu 24.04 amd64, CMake 3.28.3, GCC 13.3.0 and Python 3.12.3, with the official SDK matching the selected matrix version. Each invocation used the pinned image above, offscreen/Fusion rendering and UID/GID 1000.

| Command | Result | Elapsed seconds | Retained artifact directory |
| --- | --- | ---: | --- |
| `task ci-6.8` | Passed, exit 0 | 301.307 | `build-ci/6.8.0/20261008T215703Z-tcdqo95o/` |
| `task ci-6.11` | Passed, exit 0 | 296.654 | `build-ci/6.11.3/20261008T215746Z-hzrlhll_/` |

Each run passed all seven CTest entries for both shared and static variants, QML lint commands, both 140-symbol parser privacy checks, install/relocation metadata and notices, shared direct-link/plugin-only consumers, static consumers with `share/qml`, the tests/examples/benchmarks-disabled library-only build, and the Release benchmark smoke executable plus JSON validation. The Ubuntu CMake 3.28 installed-consumer checks now pass where the initial snapshots failed.

`environment.json`, `result.json`, `ci.log`, `tool-versions.txt`, Ubuntu and installer package manifests, aqt logs, packaging reports and benchmark JSON are retained. Final evidence assertions confirmed the two successful result records, two passing CTest suites and symbol checks per run, successful packaging/benchmark markers, read-only mount options for both `/source` and `/artifacts/source`, and ownership of every retained file/directory by the invoking UID/GID. Build artifacts are ignored by Git. Source/configuration/credential exclusions and custom output-directory isolation are covered by the five focused runner tests.

QML lint commands exit zero but Qt 6.8 emits native type-resolution warnings; no warning-free lint result is claimed. The Qt 6.11 SDK emits an optional Qt6TaskTree/QmlAssetDownloader discovery warning, and Vulkan headers are absent; configuration and all required checks still pass. Checkout/upload actions and a hosted GitHub execution of the changed workflow were not exercised. No commit, push, image publication or remote workflow dispatch was performed.
