# Verification

Approved on 2026-10-09 through the explicit user approval of Iteration 019. Approval covers requirements, design and tasks; scope is tests/documentation only. Production changes require revised approval.

## Planned checks

| Requirements | Evidence |
| --- | --- |
| R1 | Assertion-level coverage matrix, existing/new named native tests and recorded outcomes for every required block/inline family. |
| R2 | Focused settled-layout checks for widths, replacement and style transitions, followed by complete view suites in shared/static builds. |
| R3 | Focused resource/view checks using controlled local data and injected replies/decoder gates; display assertions separate from controller policy assertions. |
| R4 | `python3 scripts/verify-commonmark.py --strict`: 652 parser/model passes and zero unresolved failures/limits/losses. |
| R5 | Shared/static CTest; `task ci-6.8`; `task ci-6.11`; documentation consistency and `git diff --check`. Retain actual environment, counts, exit codes and artifact paths. |
| R6 | Final diff inspection confirms tests/documentation scope, unchanged production/public/version/dependency files and explicit unresolved defect/release records. |

## Actual results (2026-10-09)

Approval was recorded before implementation. Changes are limited to tests and documentation; no production defect was found, and no production fix was made.

### Focused host checks

Rebuilt `qmarkdown-view-test` in existing shared/static host builds. The focused shared command was:

```sh
QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/tests/qmarkdown-view-test retainedSemanticsPresentation mixedDocumentHiddenTransitions staleImageCompletionPresentation imageRowsAndPolicyLifecycle linkInteraction:autolink
```

Final result: 13 behavior/data rows plus init/cleanup, **15 passed, zero failed/skipped**, exit 0. The eight source-driven semantic presentation rows, hidden mixed-document transition, two held-decoder rows, enhanced image lifecycle and autolink interaction all pass. Initial test-authoring failures were corrected: the wide height needed a settled final-block boundary, restored unformatted descriptions use PlainText delegates, and an empty image description contributes no visible text. None required a production change.

`python3 scripts/verify-commonmark.py --strict --report /tmp/019-strict.json`: exit **0**, 652 parser passes and 652 model passes, zero mismatches/uncheckable examples or unresolved comparison limits/semantic losses.

`ctest --test-dir build-shared --output-on-failure`: **7/7 passed**, exit 0, 24.20 s. Static equivalent: **7/7 passed**, exit 0, 23.41 s. These execute every existing/new view/resource check named in coverage.md. Host uses Qt 6.12.0, GCC 16.2.1, existing Linux offscreen test configuration and bundled cmark 0.31.2.

### Pinned local CI

Initial sandbox attempts could not access `/var/run/docker.sock`: Task exit 201, runner exit 1 during image preparation. Failure evidence remains at `build-ci/6.8.0/20261009T172810Z-z6rcta7f/` and `build-ci/6.11.3/20261009T172810Z-qglrwviy/`. Approved escalated reruns use the existing cached toolchain images and fresh source/build snapshots:

| Task | Result | Shared/static CTest | Elapsed seconds | Evidence directory |
| --- | --- | --- | ---: | --- |
| `task ci-6.8` | Passed, exit 0; Qt 6.8.0 | 7/7 each, 23.21 s each | 126.290 | `build-ci/6.8.0/20261009T173052Z-mu1o8duh/` |
| `task ci-6.11` | Passed, exit 0; Qt 6.11.3 | 7/7 each, 23.27/23.16 s | 130.761 | `build-ci/6.11.3/20261009T173055Z-54ok0mgg/` |

Each native view suite records **48 passed**, each resource suite **20 passed**, with zero failures/skips; these totals include init/cleanup. All required matrix coverage rows pass. Both tasks also pass shared/static QML lint, 140-symbol parser privacy checks, installed/relocated direct-link/plugin-only/static consumers, license notices, library-only build and validated Release benchmark smoke output. Both reuse the same immutable images recorded in Feature 018; preparation took 0.014/0.017 s, verification 126.258/130.714 s. `environment.json`, `result.json`, `tool-versions.txt`, `ci.log`, `packaging/` and `benchmarks/` retain detailed evidence.

Environment: pinned Ubuntu 24.04 amd64, GCC 13.3.0, CMake 3.28.3, Python 3.12.3, bundled cmark 0.31.2, offscreen/Fusion, UID/GID 1000 and read-only source mounts. Existing Qt 6.8 QML native-type resolution warnings, Qt 6.11 optional Qt6TaskTree dependency discovery warnings and unused-import information remain; Vulkan headers are unavailable. All checks return success; warning-free builds are not claimed.

Final source checks confirm `tests/tst_view.cpp` and every production source byte-match both CI snapshots. All four shared/static corpus reports were independently loaded and validated against current schema/fixtures: 652 parser/model passes, zero mismatches/uncheckable/aggregate limits/losses and strict acceptance true. Snapshot documentation predates final result recording; executable tests and production sources match the final tree.

Final diff review confirms tests/documentation scope. `git diff --check` and local prose Markdown link/trailing-whitespace checks pass. All authorized tasks are complete. No unresolved native defect was found by these checks; the limits below remain.

### Scope and limits

Added/updated `tests/tst_view.cpp`, Feature 019 requirements/design/tasks/verification/coverage, README and specification index/standards/roadmap. Production sources, public interfaces, dependencies, build/CI implementation, fixtures and 0.7.0/0.7 versions are unchanged.

Evidence covers representative Linux offscreen native layout/interaction and complete pinned-corpus semantics separately. Other operating systems, GPU output, accessibility and hosted workflow/cache execution remain unverified. The existing blocking active-decoder destruction limitation remains documented. v1.0 support/release decisions and publication are outside this approval.
