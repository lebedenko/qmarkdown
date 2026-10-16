# Actual verification — syntax highlighting

Approval and scope are recorded in requirements.md, design.md and tasks.md.
Merge, tagging and publication are not authorized by this implementation task.

## Local results (2026-10-16)

- Host Qt 6.12.0, shared, backend ON: build and full CTest passed, 21/21, 85.27 seconds before the additional lifecycle/layout/performance checks. Updated focused syntax/backend and rendering checks passed after those additions (2/2, 8.47 seconds). With examples enabled, view and syntax checks passed (3/3, 21.88 seconds), including the corrected viewer layout.
- `all_qmllint` passed for the library; strict `python3 scripts/check-docs.py --release` and `git diff --check` passed.
- Focused tests cover token roles/languages, multiline and embedded source, incomplete code, UTF-16 boundaries, resource fallbacks, delayed fake backend results, GUI responsiveness during blocked work, cancellation/replacement/destruction, deduplication, reentrant delivery, entry/byte LRU eviction and live palette reuse.
- Rendering checks inspect actual painted token pixels and cover palette replacement/default restoration, selection/Find, nested fences, exact document-copy source including tabs/trailing newlines, alignment/layout stability, empty/unknown/indented source and custom renderer precedence.

## Configured matrix and delivery

The local working-snapshot matrix passed all eight 30-test suites and all eight
QML lint runs, with examples enabled:

| Qt | Shared OFF | Static OFF | Shared ON | Static ON |
| --- | --- | --- | --- | --- |
| 6.8.0 | 30/30 | 30/30 | 30/30 | 30/30 |
| 6.12.0 | 30/30 | 30/30 | 30/30 | 30/30 |

Relocated consumers passed for both linkage modes and both backend states on each
SDK, including historical imports and compatible/exact package requests. Disabled
build caches contained no KDE discovery; shared consumers configured with KDE
package discovery explicitly disabled. Enabled static consumers resolved the
private transitive dependency. Both SDKs completed benchmark smoke. The final committed-source matrix is reported
through [PR #34 checks](https://github.com/lebedenko/qmarkdown/pull/34/checks).
Local matrix artifacts are retained in ignored `build-ci-049-minimum-complete`
and `build-ci-049-development-complete` directories.

Final review added a post-normalization budget check and avoided allocating a
foreground map when no syntax ranges exist. The affected host view, styling,
selection and syntax suites passed 7/7 (67.34 seconds). The explicit build-mode
availability assertion then passed the syntax-view suite (1/1). The final PR
matrix repeats the full configured verification for these committed changes.

The final deterministic source archive passed extracted build/install checks with
backend OFF and ON on both SDKs (four builds), in
`build-source-check-049-final-{6.8.0,6.12.0}`. Source SHA-256:
`fc0a9bc938374acdbe1308edb3c915639d1665e0e0a960023b9ec25139b60dbb`.

Arch package checks passed against that archive: dependency lower bounds including
`syntax-highlighting>=6.10.0`, payload/ownership, namcap, linked/plugin-only
consumers, explicit enabled-highlighting availability and token ranges, and
pacman install/upgrade/removal. Final package SHA-256:
`61bf9f81632f8acdfc0cff6c6c189780f995591829c3be2f918f00ee266db635`.
Artifacts/logs are in ignored `build-arch-049-availability`; these bytes have not
been published. Source and package inventories contain no vendored KDE definitions.

Strict release docs, whitespace checks and host Qt synchronization (already
6.12.0) passed. One release PR contains implementation, specifications, tests,
documentation and packaging. Final-head CI status lives on that PR; no merge,
tagging, release dispatch or publication is part of this authorization.

The first configured runs exposed an example import pinned to 1.10; it was corrected to 1.11. KDE 6.10's indexer also conflicts with Qt 6.12's new `_sv` literal. CI guards and records a local indexer-only literal rename by original/modified file hashes; runtime library code and syntax resources remain pinned upstream. Initial failed runs are retained under ignored build-ci-049-* directories and do not count as passing evidence.

Pinned archives downloaded from KDE over HTTPS and hashed locally:

- Extra CMake Modules 6.10.0: `506989a0d400913403e669c1912238db053cd6b38dff74b17e2e6f879c79cca0`.
- KSyntaxHighlighting 6.10.0: `b5b5e343ff27bc5c95be0051d5606dfcb3295f835830e7fc6dac8d2863891699`.

Private cooperative limits cannot forcibly interrupt an individual backend call. Performance measurements are advisory; the deterministic responsiveness test blocks the fake worker while a GUI event executes and asserts stale work cannot alter replacement source.

Advisory host Qt 6.12 measurements: 40 blocks to first result 62 ms; 4,000-line
block initial layout 18 ms; 100 replacements 73 ms; 100 alternating palette
assignments 3 ms. No throughput thresholds are asserted. Explicit backend ON with
`CMAKE_DISABLE_FIND_PACKAGE_KF6SyntaxHighlighting=TRUE` failed configuration as required.
