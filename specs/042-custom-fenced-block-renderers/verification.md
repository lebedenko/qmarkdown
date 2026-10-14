# Verification — custom fenced-block renderers

Status: **approved on 2026-10-14** by the user response “approved”. Requirements
R1–R8, design and tasks T1–T5 are authorized for implementation and verification.
Package 1.7.0 / QML 1.7 is retained; no version bump, tag or publication is authorized.
See [requirements](requirements.md), [design](design.md), [tasks](tasks.md) and
[verification](verification.md).

## Planned implementation checks

- V1: Default empty map, exact case-sensitive first-token matching, leading/mixed
  whitespace, empty/unmatched tokens, full decoded entity/escape metadata, unchanged
  code whitespace/trailing newlines, whole-map updates and inherited-key exclusion.
- V2: Fenced versus indented code (including empty fences and apparent language
  text in indented code), recursive lists/quotes and mixed documents. Confirm metadata
  survives semantic/presentation projection without altering source-copy offsets.
- V3: Null entries, non-Components, Component errors, missing required properties,
  non-Item roots and construction failure: diagnostic plus ordinary fallback. Observe
  instance destruction on document/component replacement, selection toggling and
  teardown; no stale visual, partial-instance leak or repeated retry loop.
- V4: Width changes, narrow widths, empty code, finite/negative/nonfinite implicitHeight,
  height notifications, live style changes/replacement and nested geometry. Assert
  surface padding/decorations and action headers retain existing placement.
- V5: Selection enabled with/without selected text suppresses custom content;
  toggling restores eligible visuals safely. Exact code-action copy payloads and
  document selection text remain unchanged. Exercise native playground example,
  map reassignment and host clipboard/scroll ownership.
- V6: Focused checks first, existing view/style/selection/resource and GFM regressions,
  strict 652-example CommonMark semantics, QML lint, shared/static builds and installed
  and relocated consumers (including plugin-only shared import). Run configured
  minimum/development Qt targets, currently 6.8.0/6.12.0, through repository tooling;
  confirm parser symbol privacy, source archive completeness and no public C++ headers.
  Required PR packaging jobs verify the final committed head before protected delivery.
- V7: Documentation checker, specification links, whitespace and eventual API/example
  consistency. Record actual runtime commands, versions, outcomes and limits separately.

## Actual documentation results

Documentation reconciliation and draft specifications only; no renderer code, package
version, tag or publication changes. At the initial documentation stage, runtime checks V1–V6 were not run because
the feature was unimplemented and awaiting approval. Subsequent implementation
results are recorded separately below.

2026-10-14 actual results:

- `python3 scripts/check-docs.py`: PASS, current 1.7.0 documentation and local links.
- Explicit local-link validation over the four reconciled summaries and all four
  Feature 042 records: PASS, 279 local links across eight files. Code fences and
  external URLs were excluded; each relative target was resolved from its document.
- `git diff --check`: PASS for tracked changes. Explicit whitespace validation of
  all four new specification files: PASS (no trailing whitespace, final newline present).
- Manual consistency review: local 1.7 versus published 1.6, implemented GFM/desktop
  selection/maintainability, preserved historical records, draft approval boundary
  and requirement/task/check mapping agree. No runtime tests were needed for this
  documentation-only change. No push, merge, tag or publication was performed.

## Actual implementation results

2026-10-14, after explicit approval:

- Implemented private fence-origin getter and semantic/presentation/model metadata,
  exact map dispatch, host Component creation/validation/fallback, instance lifetime,
  content geometry/style updates, source selection and unchanged copy actions.
  Added native playground controls/sample, public guide/API reference and relocated
  shared, plugin-only shared and static renderer consumers. No Qt private API used.
- Host Qt synchronization: `python3 scripts/ci-toolchain/qt_versions.py --sync-host qmake6`
  reported 6.12.0; configured targets were already aligned and unchanged.
- Focused renderer checks: 21 passed, zero failed/skipped. Cover decoded info,
  fenced/indented distinction, nested blocks, empty code, exact matching/fallback,
  invalid roots/property types/Components and creation failure, width/height/style,
  document/component replacement/destruction, construction-time replacement,
  selection toggles and exact code/document copy payloads. Playground toggle/sample
  check and existing action/fenced geometry regressions pass.
- `cmake --build build-shared --parallel 2`, full host `ctest --test-dir build-shared
  --output-on-failure`: PASS, 17/17 targets. `all_qmllint`: PASS, only the existing
  import-only example's unused-import informational message.
- Final configured minimum Qt 6.8.0 and development Qt 6.12.0 matrices: PASS, exit 0.
  Commands: `python3 scripts/run-ci.py --qt-role minimum --artifact-dir
  build-ci-042-minimum-final` and the equivalent development command with
  `build-ci-042-development-final`. Each ran shared/static 17-target suites (68/68
  total), strict 652-example CommonMark checks, QML lint, parser symbol privacy,
  version compatibility, relocated linked/plugin-only/static consumers (including
  renderers), library-only builds and advisory benchmark smoke/JSON validation.
  Actual runner results are retained in each artifact directory's `result.json`,
  `ci.log`, `packaging/` and `benchmarks/`.
- Working-tree source archive from the existing allowlist: built, extracted,
  configured, built and installed on host Qt 6.12.0 via `verify-source-archive.py`:
  PASS. Artifacts: `build-archive-042/`; log: `/tmp/qm042-archive.log`. No new source
  file required an allowlist addition; the private getter remains hidden/uninstalled.
- Early checks exposed unsupported QML required-flag introspection and test-window
  polish assumptions; both were corrected. Broader host/minimum checks exposed an
  ordinary Loader-height regression; reserving custom height on the surface restored
  ordinary text sizing. Final runs above cover the correction. Earlier failures are
  retained in `build-ci-042-*-verified` and are not passing evidence. Sandbox Docker
  failures are retained separately; approved Docker access enabled final matrices.
- Documentation checker, affected specification local links and whitespace checks
  pass. Package/module remain 1.7.0/1.7. Historical Feature 040/041 local records
  remain intact; GitHub confirms their protected merges in PRs 19/20.

## Protected delivery

One feature PR contains implementation, tests, documentation and this actual evidence.
The required minimum/development hosted packaging jobs verify the final committed
head before protected squash delivery. Hosted logs remain in Actions. Tagging and
publication are excluded from this authorization.
