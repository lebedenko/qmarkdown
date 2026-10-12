# Verification

Approved for local implementation and verification on 2026-10-13. Actual execution results are recorded separately from the planned checks below.

## Planned

- Independent semantic expectations for GFM section 5.3 examples, with source attribution, pinned acquisition/checksum and applicable fixture license before importing upstream examples. Add authored negatives for escaped/entity brackets, missing separator whitespace, malformed markers, later paragraphs, headings, code, HTML and reference interactions. Cover tabs/line endings and marker-only items against the specification's whitespace rules.
- Check default-off rendering and toggles in both directions with the same source. Compare retained model state and displayed text; assert original `markdown` stays intact and redundant option assignments do not reset models or restart resources.
- Check native geometry and accessibility for checked/unchecked, mixed, ordered and recursive lists at normal, narrow and nonpositive widths; live typography/color changes; pointer/focus inactivity; formatted labels, links, authorized images and existing code-copy callbacks.
- Run narrow parser/view/resource tests first, then the relevant shared/static CTest suites and `python3 scripts/verify-commonmark.py --strict`. Require 652 parser and 652 model passes with no unresolved losses/limits in default mode; extension evidence remains separate.
- Run `task ci-minimum` and `task ci-development` for final build-affecting inputs, including QML lint, symbol privacy, package/installed-consumer checks and existing required checks. Test historical imports and the new 1.3 fixture. Record exact source revision, commands, environments, counts, failures and limitations.
- A representative viewer review supplements offscreen assertions; it does not establish every-platform or full GFM conformance. No release publication or downloaded-release lifecycle execution is planned without publication authorization.

## Actual

Planning review inspected the roadmap, standards, current delivery guidance, iteration 034 record, production parser/inline adapters, ViewState, recursive list rendering, task runner and existing view tests. No production files were changed and no runtime verification is claimed. Draft document checks are recorded after execution below.

Planning checks on 2026-10-12: `git diff --check` passed for tracked edits; all four new draft documents were checked for local Markdown links, with the one local target resolving. Runtime tests are unnecessary for this prose-only planning change.

### Local implementation checks, 2026-10-13 (Europe/Kyiv)

Base revision: local main `8f1a5f3`; verification includes the uncommitted approved feature and preserved user edits, not a clean committed release revision. No commits, pushes, PRs, tags or publication were performed during these local checks. Commit, push and PR delivery were subsequently authorized on 2026-10-13; merge and publication remain unrequested.

Host environment: Linux x86_64, Qt 6.12.0, GCC 16.2.1. Configured and built shared/static variants with tests and examples enabled. Narrow parser checks (`taskMarkers`, `taskContextAndInlineSemantics`, `officialTaskListExamples`) passed 32 QtTest cases including initialization/cleanup; 28 marker rows each cover LF, CRLF and CR. The two upstream section 5.3 examples pass separate independently authored state/label expectations. Focused native view/image checks and playground checks passed.

Final host shared/static `ctest --test-dir build-{shared,static} --output-on-failure`: 12/12 checks pass for each variant. Strict `python3 scripts/verify-commonmark.py --strict`: 652 parser passes, 652 model passes, zero mismatches/uncheckable examples and no unresolved comparison limits or semantic losses. `cmake --build build-shared --target all_qmllint` passes with the existing informational unused import in the import-only example. `python3 scripts/verify-cmark-symbols.py build-shared` passes: 141 external definitions are prefixed, with no shared cmark exports. Changed Python packaging scripts compile.

Native assertions cover default-off behavior, source preservation, both toggle directions, replacement while enabled, no-op notifications/model resets, mixed/ordered/nested/loose lists, marker-only body height, link activation, accessibility checked/unchecked state and inert toggle action, pointer/keyboard inactivity, live typography/color, one-pixel/nonpositive widths, deny-by-default images and real authorized-image projection. The playground toggle/sample also passes. An offscreen software capture at `build-shared/035-task-lists.png` was inspected: markers, read-only indicators, nesting, numbering, label formatting and literal escaped syntax render correctly. This is representative offscreen evidence, not GPU, assistive-technology integration or broader-platform certification.

The initial test build encountered a moc raw-string parsing issue in the new QML fixture; replacing that fixture with an ordinary escaped C++ string resolved it. Initial sandboxed CI attempts failed at Docker socket access; approved escalated task executions use the same isolated runner. Existing consumer font-unit warnings and a pre-existing unused fgets return warning remain unrelated to this feature.

Minimum-Qt task `task ci-minimum` passed with exit 0, artifacts at `build-ci/6.8.0/20261012T212039Z-vf20t6mx`. Both shared/static suites pass 12/12 checks, relocated linked/plugin-only 1.3 task consumers and historical 1.0/1.1/1.2 consumers pass, version compatibility/EXACT checks pass, QML lint and symbol privacy pass, library-only build and benchmark smoke pass. The Ubuntu candidate archive and installer checks pass, including repeated installation, ownership, uninstall and all eight installed consumer configurations in a disposable root container. No host system installation was performed. All 139 build/test/package input files match the captured source snapshot.

Development-Qt task `task ci-development` passed with exit 0, artifacts at `build-ci/6.12.0/20261012T212057Z-fzq9ftjz`. Shared/static suites each pass 12/12 checks; both reports retain 652 parser/model passes with zero unresolved limits/losses. Relocated task and historical consumers, compatibility checks, lint, symbol privacy, library-only build, benchmark smoke, Ubuntu candidate archive and disposable-root installer/consumer lifecycle all pass. All 139 build/test/package inputs match both captured snapshots. The first development SDK provisioning took about 32 minutes with most time in the Qt Declarative installation step; verification then completed successfully. The SDK emits an optional Qt6TaskTree/QmlAssetDownloaderPrivate discovery warning; required Qt components, all builds and executed consumers pass without it.

[Machine-readable summary](verification-summary.json) records the baseline revision, build-input digest, host/corpus results, container identities/results, durations and verified archive checksums. Raw logs and reports remain in the local build directories. These are two separately built candidate archives; no same-archive cross-runtime or published-release claim is made.

### Changed files and delivery limits

Implementation changes are in the bundled cmark parser/provenance/symbol map; private document/ViewState/model metadata; MarkdownView.qml; package/module CMake versions; playground Main.qml/Samples.qml; parser, view and import tests; selected-GFM fixtures/provenance; installed task consumer; and Ubuntu/Arch packaging consumer matrices. Documentation adds the task-list guide, unreleased changelog entry, current spec indexes/status and this feature's approval/verification records. Existing user-owned workflow edits are retained.

Arch recipe regression checks pass and the changed Arch verification script compiles. A new Arch candidate lifecycle was not run: `build-arch-package.py` archives a committed revision, while this approval covers local uncommitted implementation and verification. Hosted required checks and PR delivery are now authorized. Fresh exact-commit Arch/release gates remain release-delivery work; publication has not been authorized; historical downloaded-release evidence is not reused as evidence for this new candidate.

Final prose-only checks after evidence recording: tracked edits pass `git diff --check`; all new specification files have clean trailing whitespace and final newlines; 314 local prose links resolve; the summary JSON parses and both container exit codes are zero; selected-GFM fixture JSON and its recorded SHA-256 match. Build/test/package inputs remain identical to both successful snapshots, so runtime checks were not repeated for evidence-only prose updates.
