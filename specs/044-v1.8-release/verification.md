# Verification — 1.8 desktop reading improvements

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
