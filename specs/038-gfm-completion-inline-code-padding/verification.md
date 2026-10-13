# Verification — planned and actual

Status: A1 and A2 approved on 2026-10-13; local implementation and verification complete. Protected commit/PR/merge and exact-commit release/publication/download acceptance remain separately authorized delivery work. Earlier milestones below retain their historical scope.

## Planned checks

| Requirements | Checks |
| --- | --- |
| R1 | 32 individual combinations × aggregate off/on; precedence/restoration; default-off, equal writes, both toggle directions, masked values, exact signals, source preservation, replacement and stale resource cancellation |
| R2–R3 | Pinned official table examples 198–205 and tag-filter example 657 with independent model expectations; escaped pipes/code, delimiter mismatch, missing/excess cells, termination/containers, case/name boundaries, raw spelling vs filtered semantics, disabled baseline and mixed extensions |
| R4–R5 | Native table widths/heights, wraps, left/center/right alignment, header bold, nested lists/quotes, native links and denied/authorized/cancelled images, live grouped styles, defaults/overrides/reset and narrow-width limitation |
| R6–R7 | Successful prototype before detailed design approval; every preset 4/2 defaults; negative/nonfinite metrics, precise resets/signals; zero-padding baseline; transparent surfaces; adjacent/wrapped code; UTF-16 mapping, graphemes, bidi, font/DPI changes and pointer/drag behavior |
| R8–R9 | Historical QML/package compatibility plus 1.6 consumers in shared-linked/plugin-only/static modes; strict unchanged 652-example CommonMark parser/model gate; shared/static CTest, QML lint, symbol privacy, benchmarks, configured Qt 6.8.0/6.12.0 and package checks |
| R10 | Authorized protected PR/merge; exact final Ubuntu/Arch bytes and SHA/checksum provenance; authorized tag/publication; independently downloaded-package acceptance and compact release evidence |

## Actual results

Initial planning stage, 2026-10-13: inspected repository workflow, feature 037 records, parser adapter, ViewState lifecycle, MarkdownView delegates, grouped styles, FormattedText shaping/hit geometry and version/check tooling. Initial working tree clean. Consulted official GFM and Qt documentation. Static investigation is recorded in investigation.md; prototype feasibility and performance remain unverified.

At the initial planning stage, only specification files and the specification index were changed. Verification passed: `git diff --check`; a Python local-link check resolved all 158 relative links across the proposal and specification index; all six new documents have clean trailing whitespace and final newlines. Scope review confirms pending approval, the prototype/detailed-design gate and separate publication authorization are explicit throughout. Runtime/build/package checks are not run for this prose-only proposal, following docs/development-workflow.md; planned features are not implemented and therefore cannot yet be verified. No remote workflow, release or publication action has been performed.

## Approved implementation milestone — 2026-10-13

A1 approved by the user's response "approved". Work is based on commit `d350d4e186c2376adffadcdc91dc13a6aa0e8978` plus uncommitted feature changes; this is not exact-commit release evidence. No commit, push, PR, merge, tag or publication was performed.

Implemented: private ParseOptions and six independent/aggregate ViewState/QML controls; private cmark table nodes/parser/alignment getter; owned table/cell and raw/filtered HTML semantics; recursive resource projection; native equal-column table delegates, alignment, header font and grouped style; version/compatibility matrices and installed GFM consumers; playground controls/sample and guides. Code renderer changes are limited to table alignment; production inline padding remains unimplemented pending A2.

| Actual check | Result |
| --- | --- |
| Bounded padding prototype, host Qt 6.12.0 | 144 geometry/source/bidi/wrapping/hit-region cases passed at normal and 1.25 application scale; reproducible source/results in prototype/; mixed-direction raster inspected |
| Focused parser tests | Official table examples 198–205, tag-filter example 657, independently authored expectations, all 32 combinations, nested/mixed semantics, delimiters/escapes/boundaries passed |
| Model-only semantic probe | All nine official table/tag-filter examples match independently authored structure/alignment/raw/filtered fixture expectations |
| Focused ViewTest | All 64 aggregate/individual configurations, restoration/no-op/signals/source preservation; native geometry/alignment/nesting/link activation/live style and nonfinite metrics; image permission/link/cancellation for each new switch; playground controls passed |
| Focused grouped table style tests | All five preset defaults, overrides/reset/notifications/inherited foreground passed |
| `ctest --test-dir build-shared --output-on-failure -R 'qmarkdown-(parser|commonmark|view|styling|resources)'` | 7/7 passed, including unchanged strict 652-example CommonMark gate and fractional styling |
| `python3 scripts/verify-packaging.py --work-dir build-packaging-038` | Passed shared/static relocation, 12/12 CTest in each variant, lint/privacy, 34 package requests, historical consumers and 1.6 GFM shared-linked/plugin-only/static consumers; examples/tests-disabled shared/static builds also passed |
| `cmake --build build-shared --target all_qmllint` | Passed with no new warnings; existing import-only unused-import info remains |
| `python3 scripts/verify-cmark-symbols.py build-shared` | 143 prefixed definitions; shared exports private |
| Python compilation for updated packaging scripts | Passed |
| `git diff --check` | Passed after removing a fixture provenance trailing blank line |

Package verification began before the last supplemental parser/playground/nonfinite assertions and model-only probe additions; corresponding focused checks were rebuilt/run afterward. Production behavior is covered by the packaging run; these later assertion/probe additions and formatting-only source changes are covered by focused checks. Do not describe the run as final release-package acceptance.

Remaining gates: A2 approval and production inline padding (T5); final style/default-layout documentation; final configured Qt 6.8.0/6.12.0 container/package checks after the complete implementation; explicitly authorized protected delivery and final Ubuntu/Arch exact-byte publication/download acceptance. Configured container/distribution checks have not been run at this milestone because the feature is incomplete behind its approved A2 gate. Existing CI triggers/protections are unchanged.

Final supplementary checks: `scripts/verify-commonmark.py` rerun after model-only probe additions retained all 652 parser/model passes, zero mismatches/uncheckable cases/information loss, and unchanged reviewed baseline. The advisory full benchmark (18 measurements) and inline preparation benchmark (16 measurements) both passed their existing JSON validators; compact summaries are retained in [benchmark-summary.json](benchmark-summary.json) and [inline-benchmark-summary.json](inline-benchmark-summary.json). Timings are advisory, not universal performance guarantees or prototype-to-production padding measurements. A link check excluding illustrative code examples resolved 302 local Markdown links; both new fixture hashes match provenance. An initial regex-only link scan incorrectly treated an illustrative `../guide.md` destination in a fenced example as a documentation link; the corrected check excludes code examples. Final whitespace check passed. [Compact milestone evidence](verification-summary.json) records actual results and pending gates.

## Final approved local implementation — 2026-10-13

The user explicitly approved the prototype-backed detailed design (A2). Production padding is integrated: each wrapped visual code fragment reserves horizontal space and increases line advance; backgrounds and link hit regions share that geometry. Every preset defaults to 4/2 logical pixels, including transparent Legacy/Minimal surfaces. Semantic text, code identities and UTF-16 positions survive parsing, model projection and image text slices. Zero metrics retain the original geometry/draw path. Playground padding controls, public guides, compatibility wording and draft release notes document the intentional layout change.

Final input identity is retained in [source-inputs.json](source-inputs.json): base commit `d350d4e186c2376adffadcdc91dc13a6aa0e8978` plus uncommitted changes, 148 build/test input files, SHA-256 `03a232ff08ce6925bcb28cd77964dd51afff1ae10747b222089bbf8953832093`. Both final configured source snapshots match that digest. Documentation/evidence changes are excluded from the digest. These are source-candidate checks, not exact-commit publication evidence.

| Final actual check | Result |
| --- | --- |
| Focused parser/model checks | All official table/tag-filter examples and mixed/independent extension fixtures passed; model-only probe independently checked all nine examples; semantic code identity/UTF-16 assertions passed |
| Focused production padding checks | Defaults/reset/notifications, zero geometry, adjacent/partial linked spans, whitespace, wrapped fragments, Hebrew/Latin, combining/emoji, widths 1/32/80/220, font edits, foreground/background stability and native pointer/drag passed |
| Fractional pointer checks | `QT_SCALE_FACTOR=1.25` ViewTest padding/table/link/font checks: 7 passed, 0 failed; complete styling suites also run at normal and fractional scale |
| Host Qt 6.12 shared | `ctest --test-dir build-shared --output-on-failure`: 12/12 passed |
| Host Qt 6.12 static | Rebuilt final source in `build-packaging-038-final/build-static`; CTest 12/12 passed |
| Strict core gate | Final `scripts/verify-commonmark.py` report: 652/652 parser and model passes, zero mismatches/uncheckable cases/semantic loss, unchanged reviewed baseline |
| QML lint and symbol privacy | `all_qmllint` passed; existing import-only unused-import info only. 143 prefixed bundled definitions, zero public cmark exports |
| Installed/package matrix | Shared/static relocation, 34 version requests, historical consumers and 1.6 GFM/padding consumers in linked/plugin-only/static modes passed; tests/examples-disabled builds passed |
| Configured minimum Qt 6.8.0 | `scripts/run-ci.py --qt-role minimum --artifact-dir build-ci-038-minimum-verified`: passed; shared/static/release-package suites each 12/12, lint/privacy, matrix/consumers, smoke benchmark and Ubuntu candidate installer acceptance |
| Configured development Qt 6.12.0 | `scripts/run-ci.py --qt-role development --artifact-dir build-ci-038-development-verified`: same complete checks passed |
| Ubuntu candidate archives | Both retained hashes match exact checked archive bytes. Staged and real disposable-container `/usr` install/reinstall/uninstall and installed consumers passed; no host installation or publication |
| Final full advisory benchmarks | Final Release render/parse JSON: 18 measurements; interval-preparation JSON: 16 measurements. Both validators passed; compact final summaries retained |
| Documentation/scripts/fixtures | Relative links checked outside examples; updated Python scripts compile; official fixture hashes match provenance; whitespace check passed |

[Minimum configured summary](configured-minimum-summary.json) and [development configured summary](configured-development-summary.json) retain immutable image IDs, recipe fingerprints, candidate archive SHA-256 values and check outcomes. Detailed local logs/source snapshots remain in their named ignored build directories. [Final benchmark](benchmark-summary.json), [interval benchmark](inline-benchmark-summary.json) and [overall verification](verification-summary.json) summaries retain compact evidence.

Investigation attempts are distinct from final results. Initial Docker runs could not access the sandboxed socket and were rerun with explicit escalation. The first final minimum-version run exposed a QML anonymous-group revision restriction; matching the existing unrevisioned grouped-field convention fixed QML 1.6 assignments. Glyph-string index types differ between Qt 6.8 and 6.12; inferred container types support both. Cursor-cell reuse removed redundant preparation, while a trial ink cache did not help and was discarded. Final configured runs listed above supersede earlier failed/incomplete attempts without overwriting their evidence.

The focused host preparation measured 18.1457/32.0357 ms (zero/default padding, 1.77×) on 15,000 characters/1,000 spans. The final minimum-runtime observation was 15.4217/31.9342 ms (2.07×), slightly above the advisory 2× target after the recorded investigation. This small runtime-dependent overrun is retained rather than treated as a portable performance guarantee or machine-dependent test failure. See [investigation](investigation.md) for comparisons and costs.

Offscreen checks cover normal/fractional application scale, font changes and source inspection of the existing screen/DPI invalidation lifecycle. A physical live screen/DPI switch was not exercised: these headless runs have no controlled multi-screen compositor/hardware configuration. They do not claim physical display acceptance.

Delivery remains D1–D3. No commit, push, PR, merge, tag or publication occurred. Exact final Arch package acceptance was not run: `scripts/build-arch-package.py` consumes a Git commit archive, while the approved work remains uncommitted; archiving HEAD would test the historical source instead of this feature. Arch recipe checks passed, and final exact-commit Ubuntu/Arch bytes, protected delivery, publication and independent downloaded-package acceptance remain the separately authorized release stage. Candidate Ubuntu archives retained here must not be described as published or final tagged release bytes.

## Release delivery authorization — 2026-10-13

The supplied release plan explicitly authorizes D1–D3: commit the feature on a branch, open one PR, squash merge after required packaging checks and resolved review threads, run the release workflow against the exact merged commit, tag v1.6.0, publish stable/latest Linux x86_64 Ubuntu/Arch packages, independently accept downloaded packages and attach compact evidence. Preserve existing releases, assets and protections. No evidence-only PR or automatic post-release upkeep commit. Earlier pending-authorization statements retain their historical scope; actual delivery results will be attached to the release.
