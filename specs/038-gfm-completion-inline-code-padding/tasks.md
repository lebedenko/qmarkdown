# Tasks — approval and delivery gates

Status: staged tasks approved on 2026-10-13 by the user response "approved"; unchecked items remain planned work, not completed evidence.

- [x] T0 (R1–R10): Inspect architecture, lifecycle, style conventions, workflow and existing evidence; prepare requirements, staged design, tasks, investigation and verification records.
- [x] A1: Obtain explicit approval of requirements, staged design and mapped tasks. Record date and scope in these documents. This gate authorizes feature work and a bounded inline prototype; it does not authorize commit, PR, merge or publication.
- [x] T1 (R6–R7): Prototype Qt-shaped padding alternatives; measure feasibility, complexity and cost; demonstrate wrapping, bidi and linked-padding hit testing using investigation acceptance cases.
- [x] A2 (R6–R7): Record successful prototype evidence, finalize detailed renderer design and obtain explicit approval before production inline-padding integration. If gates fail, revise proposal for approval.
- [x] T2 (R1): Consolidate private parse configuration; add ViewState properties, QML aliases/metadata and aggregate precedence. Test all 64 aggregate/individual configurations, toggles, masked values, equal assignments, precise notifications, source preservation, replacement and resource cancellation.
- [x] T3 (R2–R3, R9): Add private table parser/nodes/alignment and filtered HTML semantics. Pin all official examples with license, attribution, acquisition date and checksums. Author independent table and raw/filtered semantic expectations; add syntax boundaries and mixed-extension regressions.
- [x] T4 (R4–R5): Add table model/delegates/grouped style and reuse formatted/image resource presentation. Verify equal widths, wrapping, tallest-cell heights, alignment, lists/quotes, header bold, links, permitted/denied/cancelled images and live style/reset behavior.
- [x] T5 (R6–R7): Integrate approved padding algorithm and grouped metrics; verify all preset defaults, overrides/reset/notifications, zero-padding compatibility, transparent backgrounds, wrapped/adjacent spans, Unicode/bidi, narrow widths, font/DPI changes and pointer/drag interaction.
- [x] T6 (R8): Advance package/module to 1.6.0/1.6; extend historical/exact/future package matrices and installed consumers in all configurations. Update playground, guides, changelog, standards, roadmap and release notes with intentional default layout change and bounded conformance claims.
- [x] T7 (R1–R9): Run focused checks, strict unchanged 652-example CommonMark gate, shared/static tests, QML lint, symbol privacy, benchmarks and configured Qt 6.8.0/6.12.0 packaging checks. Record commands, versions, source SHA, results and unavailable checks separately from plans.
- [ ] D1 (R10): When separately authorized, deliver one focused PR per meaningful change through existing required checks and squash merge; retain implementation/tests/docs/evidence together.
- [ ] D2 (R10): When release delivery/publication is explicitly authorized, verify final exact-source Ubuntu/Arch package artifacts, provenance and checksums; tag v1.6.0 and publish verified bytes under existing protections.
- [ ] D3 (R10): Independently download published artifacts, compare hashes and repeat Ubuntu consumers and Arch ownership/consumer/upgrade/removal acceptance. Retain compact release evidence linking detailed logs.

T2–T4 can proceed after A1; production inline work depends on A2. Release gates remain separate from implementation approval.

T2–T4 completed and verified on host Qt 6.12.0. T6 compatibility, installed consumers, controls and GFM/table documentation are implemented; inline-padding guidance and intentional layout-change wording are complete after A2/T5. T7 shared/static relocation, strict corpus, lint and privacy checks passed for the current GFM implementation; final configured Qt 6.8.0/6.12.0 shared/static packaging and Ubuntu candidate acceptance have passed. Exact-commit Arch/package release delivery remains D2. See verification.md.

## Release delivery authorization — 2026-10-13

The supplied release plan explicitly authorizes D1–D3: commit the feature on a branch, open one PR, squash merge after required packaging checks and resolved review threads, run the release workflow against the exact merged commit, tag v1.6.0, publish stable/latest Linux x86_64 Ubuntu/Arch packages, independently accept downloaded packages and attach compact evidence. Preserve existing releases, assets and protections. No evidence-only PR or automatic post-release upkeep commit. Earlier pending-authorization statements retain their historical scope; actual delivery results will be attached to the release.

## Strikeout continuity correction — user plan approved 2026-10-15

- [x] Retain visual padding boundaries and bordering source cells; connect effective
  shaped strikeout endpoints with interpolated heights and code-owned styling.
- [x] Include cached connector pen extents in raster bounds without changing layout.
- [x] Add image regressions for both gutters, native font heights, inherited strikeout,
  linked colors, adjacent code styling and unconnected endpoints; strengthen existing
  wrapped/aligned/bidi image checks with per-column gutter coverage.
- [x] Run focused decoration, styling/view/selection, both configured Qt shared/static
  matrices and native Wayland wide/wrapping GFM off/on acceptance; record actual results.

Approval covers implementation and verification of the supplied plan, with no public
API, parser, dependency or version changes. It does not request publication.
