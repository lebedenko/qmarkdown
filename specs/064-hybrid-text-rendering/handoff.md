# Feature 064 handoff — QMarkdown continuation

## Starting point and authorization

Open `/home/andrii/Projects/pet/qmarkdown` as the next session's workspace. Read
[requirements](requirements.md), [design](design.md), [tasks](tasks.md),
[historical verification](verification.md) and the [development workflow](../../docs/development-workflow.md).
The owner authorized this transfer on 2026-10-21. Retain Feature 064; do not create another initiative.
Local branch: `feat/hybrid-text-rendering`. Base HEAD: `bbb0d0c1985567c4f1a7b71f82445c8fafd6ccf7`.
The candidate is reviewable and uncommitted; integration/release qualification remains pending.

Original candidate: `/home/andrii/Projects/pet/md-viewer/build/diagnostics/qmarkdown-hybrid-064`.
Original execution/evidence workspace: `/home/andrii/Projects/pet/md-viewer/build/diagnostics/hybrid-064`.
All historical runtime/build claims refer to those workspaces and their recorded isolated prefixes.
No build cache was copied or binary reused as a build of this checkout.

## Transferred content

The verified implementation patch transfers:

- `specs/README.md`
- `src/QMarkdown/private/formattedtext.cpp`
- `src/QMarkdown/private/formattedtext.h`
- `src/QMarkdown/private/private.qmltypes`
- `tests/tst_codeblocks.cpp`
- `tests/tst_math.cpp`
- `tests/tst_renderers.cpp`
- `tests/tst_styling.cpp`

`specs/064-hybrid-text-rendering/` was copied separately because it was untracked and absent from
the candidate Git diff. Preflight found that the packaged patch appends those four files; the
tracked-file portion was verified equal to the Git diff and applied separately, then the specs copied. Its four original Markdown files matched before intentional documentation additions.
`design.md` is unchanged. Transfer authorization was appended to requirements, transfer/pending
qualification tasks to tasks, and historical-location clarification to verification; this handoff is new.
Production/test files and `specs/README.md` remain byte-identical to the final candidate and archived snapshot.
Diagnostic instrumentation patches remain inside the ignored evidence directory, separate from production.

## Evidence inventory

Copies are under [build/diagnostics/hybrid-064-handoff](../../build/diagnostics/hybrid-064-handoff/).
Original archives and their adjacent checksum files remain in md-viewer's `build/diagnostics/`.
Each archive has its unchanged adjacent `.sha256` file in the new location:

| Archive | SHA-256 |
| --- | --- |
| `hybrid-064-report.tar.gz` | `6e2024fc82fe9a63db62edb45bf2a312a190f5ac40fa6b022a83bc030c1a580d` |
| `sdd0018-rendering-report-v9.tar.gz` | `07369b950ffaf8525789a4a0dfb0f1e0ddc134d6cf62fecf3e13650416a2f021` |
| `sdd0018-rendering-report-v10.tar.gz` | `34c2811ac37cad3cca8912bd42d4b78904511741d115cbc357329f6e5d135248` |

The Feature 064 report is extracted to
[hybrid-064-report](../../build/diagnostics/hybrid-064-handoff/hybrid-064-report/).
Its `SHA256SUMS` verifies 4782 payload files. Begin with its `README.md`, `EXCLUDED.md`,
`qualification/`, `measurements.json`, `backend-coverage.json`, raw logs and separate patches.
V9/V10 remain archives; all original checkouts, reports and measurements are preserved.
These local ignored artifacts are not durable Git storage: retain them before cleaning `build/`.

## Existing results and blockers

All results in this section are historical, not new transfer-checkout results:

- Qt 6.12 clean and diagnostic provider suites: 25/25; isolated viewer: 68/68; harness: 2/2;
  md-viewer `task check`: exit 0. Final code-block software/native suites: 39/39.
- Syntax-disabled Qt 6.12 shared/static packaging: 34/34 each, relocated consumers and library-only
  builds pass. Syntax-enabled shared/static gates: 33/34 each and **failed** on
  `playgroundReadingControls` with `QObject::installEventFilter` cross-thread warnings.
  Independent syntax-enabled relocated consumers/library-only builds pass; this does not waive failed gates.
- The warning appears during existing highlighting backend construction on its worker; matching V10
  sources/logs are preserved. Host Syntax Highlighting is 6.31.0; CI pins 6.10.0 in
  `scripts/ci-toolchain/kde.sh`. This dependency difference is observed, not an established cause.
  The warning's root cause remains unresolved. This transfer includes no highlighting fix.
- Exact Qt 6.8 shared/static, optional-feature and consumer qualification remains unverified:
  no local SDK or cached required CI image was found during historical qualification.
- All 28 fixed replay trials completed: 18 measured qualify, ten disabled controls remain unmeasured.
  Eligible text incurs zero replay raster/layout/glyph rebuild work. Residual padded-code raster cost
  remains. Timing preserves qualification and does not establish a speedup or perceptual fix.
- Native OpenGL/Vulkan checks pass at asserted actual DPR 1/1.25/1.5/2. Historical scale-multiplier
  labels are corrected in verification.md. Original wheel-control failures remain preserved alongside
  the later explicit-Mouse correction. Live monitor migration, untested transforms/scales,
  screen-reader sessions and owner visual acceptance remain unqualified. Shimmer remains unassessed.

Keep replay performance, feature compatibility and perceived shimmer separate. Read the full historical
verification for bounded memory/resource results and exclusions. Do not upgrade any result on transfer.

## Continuation workflow

Existing entry points are `task ci-minimum`, `task ci-development`, and PR-triggered hosted
`packaging (minimum)` / `packaging (development)` checks. Review the existing workflow before running
these; missing toolchains/images may require acquisition. Neither downloads nor a PR are authorized
by this transfer. Exact Qt 6.8 and the syntax-enabled packaging warning remain integration blockers.
No commit, push, merge, release, version change or md-viewer dependency adoption is included.

## Transfer verification

- Clean sibling/base and absent branch checked before mutation; `git apply --check` passed.
- The tracked-file portion of the packaged patch equals the final candidate's Git diff; all eight transferred patch files
  match the original candidate and archived final source bytes.
- Separate specifications matched before the explicitly listed documentation additions.
- All three copied archive/checksum pairs match originals; every extracted manifest entry passed.
- `python3 scripts/check-docs.py`: PASS (current 1.17.0 documentation and local links).
- Explicit local-link checks for all Feature 064 documents and updated md-viewer SDD/roadmap: PASS.
- `git diff --check` in both repositories and explicit whitespace checks for untracked specs: PASS.
- All 1158 before/after SHA-256 fingerprints match, recorded in the ignored evidence directory
  as `preservation-sha256.json`. They cover original candidate/spec/archive evidence, viewer
  production/dependency files and installed-provider/viewer files. The current installed provider
  is 1.17.0; older 1.14.0 observations remain historical. No package installation was performed.
- No missing CI matrix or native/runtime tests are run for this transfer. Historical results remain historical.

Preflight stopped without mutation on the packaged-patch/spec distinction and an obsolete historical
installed-library filename. Both were corrected from current read-only evidence before transfer.

## Subsequent authorization

The owner subsequently approved the qualification continuation, including the highlighting fix and
isolated toolchain acquisition. See [current actual results](continuation-verification.md); the
transfer-only boundaries above describe the historical transfer, not the new authorization.
