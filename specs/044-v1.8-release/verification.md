# Verification — 1.8 desktop reading improvements

## Draft lookup recovery and repair — 2026-10-15

The user explicitly approved recovery of the existing draft from retained checked
artifacts before advancing main, followed by one lookup-repair PR, fresh full Qt
CI/release gate and squash merge. This is an approved manual recovery exception
to workflow-driven publication; no rebuild or repeat distribution tests, version
bump, runtime API change, release-note change or further 1.8.0 dispatch.

Publication run [38702659450](https://github.com/lebedenko/qmarkdown/actions/runs/38702659450)
remains **failed**: `/releases/tags/v1.8.0` could not resolve the newly created
draft. Manual recovery completed separately:

- An isolated checkout of `48e37ef13ca691771ad250383ce8bddd0cd8cab1`
  passed existing PR #26 eligibility and strict release documentation validation.
  Main and tag matched that commit; no publication run was active.
- Only source artifact `11957390689` and Arch artifact `11957146525` from the
  failed run were downloaded into a fresh directory. Strict four-file selection,
  archive checksum validation and all four approved hashes passed.
- Paginated listing resolved the sole exact match to draft `413555024`, with
  `prerelease=false`; fresh ID lookup confirmed state. Existing byte comparison
  accepted the empty draft; only missing files were uploaded via its ID URL,
  without overwrite or automatic retries. All four draft downloads matched.
- Eligibility and absence of active publication runs were checked again before
  REST publication by ID with `draft=false`, `prerelease=false`, `make_latest=true`.
  All four published downloads matched; latest ID and unchanged tag were confirmed.
- Published release: [QMarkdown 1.8.0](https://github.com/lebedenko/qmarkdown/releases/tag/v1.8.0).
  Recovery evidence is also recorded on
  [PR #26](https://github.com/lebedenko/qmarkdown/pull/26#issuecomment-6188272653).

| Accepted file | SHA-256 of retained and published downloaded bytes |
| --- | --- |
| `qmarkdown-1.8.0.tar.gz` | `91b9c62b5688c44ecd5207ea525adcca5a504c7576529ba29e7560cf56b37929` |
| `qmarkdown-1.8.0.tar.gz.sha256` | `a42821836906e1efdd4020082fa5225c5e69606fa078d7235f76b0aaf7f9594c` |
| `qmarkdown-1.8.0-1-x86_64.pkg.tar.zst` | `6dc23354691da128956badefea18dfdd46077bf58bdeefa0a7fcef5f2b5d1965` |
| `qmarkdown-1.8.0-1-x86_64.pkg.tar.zst.sha256` | `965edcd20702634c32d18d92a02dee94868a60f1ccb05894da547b9d6781295b` |

Permanent repair resolves exactly one tag through paginated listings and refreshes
state by release ID, preserving prerelease rejection, bounded upload retries,
byte comparison and published-release safeguards. Both service fakes reject tag
lookup, reproducing the original failure before the fix. Focused uploader and
publisher suites pass (14 and 8 tests), covering draft/published state, pagination,
missing/ambiguous tags, listing/ID API errors, identity changes, prereleases,
partial uploads, differing bytes and published download failures. Host Qt sync
confirmed 6.12.0 without configuration changes. Final-head hosted verification
and merge results are recorded on the repair PR to avoid invalidating its head.

## Release artifact selection repair — 2026-10-15

The user approved the artifact-selection repair, one repair PR, fresh required
full Qt CI, squash merge and publication of existing 1.8.0 using that merged PR.
Existing metadata and release notes remain authoritative; no version/API change.

- Publication [run 38697428521](https://github.com/lebedenko/qmarkdown/actions/runs/38697428521)
  passed distribution checks but stopped at artifact validation before tag/release
  writes: the Arch wildcard also selected the package's `.namcap.log`.
- Added a regression that reads the workflow's configured Arch upload patterns
  and selects from a temporary package/checksum/namcap-log directory. It failed
  before the repair because the log was included.
- Replaced the broad wildcard with explicit package and checksum patterns. The
  publisher's strict four-file validation remains unchanged.
- Local checks passed: focused selection regression; all 34 offline release
  tests; 5 Arch packaging tests; strict release documentation/local links; and
  `git diff --check`. Host Qt sync confirmed configured development Qt 6.12.0.
- Final hosted verification and publication results are recorded on the repair
  PR and Actions run, avoiding an evidence-only commit that invalidates its head.

Status: **implementation and local verification complete**. Final-head hosted
verification is recorded in the associated feature PR.

## Planned checks

- V1: Unicode words, whitespace runs, punctuation graphemes, combining characters,
  bidi text and wrapped paragraphs; directional whole-word extension/reversal.
- V2: Headings, nested lists/quotes, image-split semantic paragraphs and atomic
  loaded images; empty table cells; code/HTML lines with and without a terminating
  newline; whole triple-click unit extension; no synthetic prefixes/separators.
- V3: Platform repeated-click timing/distance, immediate repeated-click selection,
  whole-unit dragging, Shift anchor extension, delayed single-link activation and
  suppression/cancellation; action exclusions and touch/selection-disabled links.
- V4: Upward/downward scrolling with stationary window pointer, edge speed and
  elapsed-time caps, host clamping/document bounds, no viewport, viewport
  destruction/removal, release/cancellation/deactivation/disable/source reset.
- V5: Installed-consumer new public interface and 1.x compatibility/future rejection;
  selection/copy, renderer lifecycle/fallback, resource-policy and conformance
  regressions; playground integration and relevant shared/static Qt matrix.
- V6: Ordinary and strict release documentation checks, links and whitespace;
  existing release tooling regressions; required final-head minimum/development
  PR gates using configured SDKs and inputs.

## Actual results

Approval was explicitly granted on 2026-10-14 for requirements, design and T1–T5.
Runtime implementation, playground integration, 1.8 package/QML metadata,
installed-consumer coverage and current-release documentation are implemented.
Tagging and publication have not occurred and remain separately authorized.

- `python3 scripts/ci-toolchain/qt_versions.py --sync-host qmake6`: host is Qt
  6.12.0; configured development target already matches, no configuration change.
- `python3 scripts/check-docs.py --release`: passed current 1.8.0 documentation
  and local links. `git diff --check` passed for tracked changes.
- Host focused CTest checks (`import`, `selection`, `selection-fractional`,
  `release-publish`, `release-delivery`): 5/5 passed. Selection checks cover Unicode,
  semantic units, loaded-image paragraphs, directional unit dragging, platform
  timing/distance, delayed links, host scrolling/clamping, null/removed/destroyed
  viewports, Escape, source replacement, disabling and window deactivation.
- Initial broad host and configured Qt shared checks found stale version fixtures
  in import-error assertions and offline release-documentation setup. These were
  corrected; focused import and release regressions now pass. Initial Qt evidence
  is in `build-ci-044-{minimum,development}-verified`; those runs are failures and
  are not final verification. The first sandboxed Docker attempt failed because
  the sandbox denies Docker socket access; approved escalated execution runs it.
- Final configured Qt matrices passed (exit 0) in
  `build-ci-044-minimum-final` and `build-ci-044-development-final`:

  | Qt | Shared CTest | Static CTest | Packaging/consumers | Benchmark smoke |
  | --- | --- | --- | --- | --- |
  | 6.8.0 | 17/17 passed | 17/17 passed | Passed | Passed |
  | 6.12.0 | 17/17 passed | 17/17 passed | Passed | Passed |

  Each matrix includes QML lint, private-symbol checks, relocation, library-only
  builds, 1.x package requests/future rejection, installed style/action/GFM/selection
  and renderer consumers, resources, renderer lifecycle and CommonMark evidence.
  The selection suite has 64 passing cases at ordinary and fractional scaling
  in both library variants and both Qt targets. Installed selection consumers
  verify viewport assignment/destruction and the scroll-request signal.
- At commit preparation, final-head hosted gates have not run. Their actual
  outcome and run link are maintained in the associated PR description/checks,
  avoiding evidence-only commits that would invalidate the verified head.
- No protected merge, tagging, distribution publication or downloaded-asset
  acceptance has been performed for 1.8.0.

## Selection interaction repair — 2026-10-14

The user authorized the repair plan within approved Features 040/044, including
all documented selection shortcuts and verification in PR #25. No public API or
host clipboard ownership change was needed.

- Reproduced the full-playground mouse-focus failure on native Wayland before
  the fix. Qt pointer diagnostics showed `MouseButtonPress` with a `TouchPad`
  device and `Finger` pointer type. Mouse-only selection handlers ignored the
  press, so the preview did not acquire focus and its existing key handler never
  received Copy/Select All/Escape. Offscreen mouse tests alone passed before the
  repair and did not expose the device mismatch.
- Selection observers, tap reservation and dragging now accept Mouse and TouchPad
  devices. Per-block link handlers exclude both during selection, preserving
  delayed-link suppression without duplicate immediate activation. TouchScreen
  and Stylus links retain their existing handling.
- Added full-playground regression checks with explicit Mouse and TouchPad/Finger
  mouse events. They cover mouse-acquired focus (without forcing preview focus),
  collapse, forward/reverse cross-block dragging, Shift-click, double/triple-click,
  whole-word/paragraph dragging, Ctrl+C signal and matching clipboard text,
  Ctrl+A, Escape, host-focus preservation/button copying, editor-owned shortcuts,
  and shortcuts after returning to the preview by mouse.
- Added full-playground TouchPad/Finger checks for delayed single-link activation,
  repeated-click/drag suppression, code-action exclusion and copying, returning
  preview focus, host edge scrolling and Escape cancellation while held.
- Focused host checks passed: full-playground repair cases and CTest `view`,
  `selection`, `selection-fractional` (3/3). The mouse and TouchPad/Finger shortcut
  cases also passed in a native Wayland full-playground window.
- The user physically checked the opened repaired Wayland viewer and confirmed
  Ctrl+C, Ctrl+A, click-drag, double-click, triple-click and double-click with Shift.
  The user pasted the Ctrl+A/C output, confirming clean projected document text
  without Markdown formatting delimiters. Additional physical edge-scroll,
  exclusion and focus-transfer checks were provided as a manual checklist; those
  behaviors have automated full-playground coverage. Desktop pointer automation
  was stopped because approval prompts interfered with cursor placement.
- `python3 scripts/ci-toolchain/qt_versions.py --sync-host qmake6`: configured
  development Qt remains 6.12.0; no change required.
- Final repair matrices passed (exit 0):

  | Qt | Evidence directory | Shared CTest | Static CTest | Other checks |
  | --- | --- | --- | --- | --- |
  | 6.8.0 | `build-ci-selection-repair-minimum-final` | 17/17 | 17/17 | QML lint, packaging/consumers, benchmark smoke passed |
  | 6.12.0 | `build-ci-selection-repair-development` | 17/17 | 17/17 | QML lint, packaging/consumers, benchmark smoke passed |

  The initial minimum runner attempt in `build-ci-selection-repair-minimum`
  failed because the sandbox denied Docker socket access. Its evidence was
  preserved; the successful run used approved Docker access and a fresh directory.
- Strict release documentation validation passed. Final-head hosted check status
  is maintained on PR #25, without a follow-up evidence-only commit.
- Merge, tagging and publication remain outside this repair.

### Blank preview-space follow-up

The user completed the remaining physical checks and reported that all passed
except ordinary-click collapse below the rendered document. Focus transfer,
editor-owned shortcuts, whole-unit dragging, edge scrolling/Escape, link
suppression and code-action copying were confirmed. Clicking within the document
cleared selection, but blank viewport space below it did not.

- Added a full-playground regression that selects all, clicks near the bottom of
  a viewport taller than its document, and checks collapse at the document end.
  Both Mouse and TouchPad/Finger rows failed before the follow-up fix.
- The playground now sets preview height to the greater of viewport and document
  content height. This includes blank space in the existing selection hit area
  while retaining document content height for scrolling; library APIs are unchanged.
- Both regression rows now pass. Focused full-playground checks and host CTest
  `view`, `selection`, `selection-fractional` passed (3/3); host QML lint passed.
  The repaired demo was rebuilt and reopened for the remaining physical recheck.
- Both complete follow-up matrices passed (exit 0): Qt 6.8.0 in
  `build-ci-selection-blank-minimum` and Qt 6.12.0 in
  `build-ci-selection-blank-development`. Each passed shared/static CTest (17/17
  per variant), QML lint, packaging/installed consumers and benchmark smoke.
  Strict release documentation validation and whitespace checks passed again.
  Final-head hosted outcomes remain on PR #25.
