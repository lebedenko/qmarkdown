# Verification — 1.9 reading and navigation

Status: **approved on 2026-10-15** by the user response “approved”.
Approval covers requirements R1–R9, design and tasks T1–T6 implementation and
verification. Merge, tagging and publication require separate authorization.

## Planned checks

- V1: Private search tests for inactive/empty/no-match state, both case modes, Unicode and surrogate pairs, repeated non-overlapping matches, forward/backward wrapping, query/case/source changes and effective parser-option resets.
- V2: View/render tests covering formatted spans, links, tables, code, image descriptions and projection boundaries; verify match colors/current distinction, unchanged layout and selected/copy text, selection overlap and keyboard selection behavior.
- V3: Renderer lifecycle and geometry tests: Find source fallback/restoration with selection on/off, action payload stability, width/font changes, image arrival, zero-width/deferred targets, view coordinate mapping and stale-target cancellation.
- V4: Outline/fragment tests for H1–H6 order, list/quote nesting, decoded titles, Unicode punctuation/whitespace/symbols, empty IDs, duplicates, literal numeric suffix collisions, exact IDs, single percent decoding, encoded Unicode, plus signs and missing/invalid targets.
- V5: Verify successful heading/search requests emit target rectangles without library scrolling; missing targets emit none. Check explicit fragment routing and unchanged local/external linkActivated payloads.
- V6: Exercise playground Ctrl+F, Enter/Shift+Enter, Escape, count/case controls, outline collapse and host scroll clamping. Record actual manual/platform results separately.
- V7: Extend installed shared/plugin-only/static/relocated consumers for new QML API and old imports. Run focused checks first, then relevant complete shared/static matrix on both configured Qt targets, packaging and existing CommonMark/GFM regressions.
- V8: Validate coordinated 1.9.0 package / 1.9 module versions, compatibility requests, current release summaries, API documentation, release notes and changelog with `python3 scripts/check-docs.py --release` after implementation.

## Actual results

### Focused local checks — 2026-10-15

- Host Qt 6.12.0 shared build: `qmarkdown-reading-test` passed all eight QtTest cases (including init/cleanup), covering literal/case/Unicode/wrapping search, selection and copy independence, recursive outline/fragment IDs, geometry, loaded-image/deferred navigation, actual highlight pixels and reset behavior.
- Playground `playgroundReadingControls` and `viewerTheme` passed: Ctrl+F focuses Find, Enter/Shift+Enter move through results, Escape closes Find, local links resolve through the host, external destinations retain their payload and host scrolling clamps.
- Existing renderer and selection tests passed after the image overlay retained the previous selection child ordering. A first focused run also caught premature target geometry; polishing pending layout before mapping corrected it.
- `all_qmllint` passed on the host, with the existing import-only example's unused-import informational message.
- `python3 scripts/check-docs.py --release` and `git diff --check` passed after the coordinated 1.9.0/1.9 documentation update.

### Configured Qt matrices

Final runs use `python3 scripts/run-ci.py --qt-role ROLE --artifact-dir DIRECTORY`.
Each snapshots its inputs and checks shared/static builds, full CTest suites,
QML lint, parser symbol isolation, relocated linked/plugin-only/static installed
consumers, package-version compatibility and benchmark smoke.

The initial minimum run at `build-ci-045-minimum` could not access the Docker
socket under sandbox restrictions. An escalated run at
`build-ci-045-minimum-verified` failed on the old playground assertion expecting
`Last activated: #heading`; the new host explicitly reports a missing local
heading. The assertion was corrected and the final matrices supersede that run.

Qt 6.8.0: **passed**, exit 0, in `build-ci-045-minimum-final/ci.log`.
Both shared/static suites passed 18/18 tests. The CommonMark report records
652/652 parser passes and 652/652 model projection passes, with no mismatches,
losses or example-level limits. Installed reader consumers passed linked,
plugin-only and static imports after relocation. Compatibility accepts 1.0–1.9
and EXACT 1.9.0, and rejects exact earlier versions, 0.7, 1.10 and 2.0.
Symbol isolation and advisory benchmark JSON checks passed.

Qt 6.12.0: **passed**, exit 0, in `build-ci-045-development-final/ci.log`.
Both shared/static suites passed 18/18 tests, including the reader and playground
regressions. Relocated installed linked/plugin-only/static reader consumers,
package/import compatibility, parser symbol isolation, QML lint and benchmark
smoke/JSON validation passed. The corresponding CommonMark reports also retain
652/652 parser and model projection passes.

Only release wording and specification/evidence prose changed after these input
snapshots; runtime, build, packaging and test inputs are unchanged. The release
documentation gate and whitespace checks were repeated against the final prose.
T1–T6 implementation and local verification are complete. Minimum Qt lint retains
the pre-existing unresolved native-type metadata warnings also seen in iteration
044; these are diagnostics, not failed runtime or lint commands. Host lint resolves
the native metadata.

No interactive desktop session, hosted PR gate, merge, tag or publication is
claimed. Playground keyboard and scroll behavior is exercised automatically with
the offscreen Qt platform. Merge, tagging and publication remain separately
authorized actions; required hosted checks still apply before delivery.

## Release authorization — 2026-10-15

The user instruction “release it” additionally authorizes the release PR, protected
squash merge, tagging and publication of 1.9.0 after required hosted checks and
distribution checks pass. This supersedes the earlier delivery exclusions above.
Publication completion requires downloaded-asset hash verification. Final hosted
verification and release outcomes are retained in GitHub Actions and the release.
