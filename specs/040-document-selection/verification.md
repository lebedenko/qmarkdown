# Verification — document-wide selection and copy

## Planned checks

- V1: Exact projection text/ranges for nested lists/quotes, ordered/bullet/task
  markers, tables/empty cells, image alt text, code/HTML whitespace, thematic
  boundaries, empty content and mixed GFM syntax; resource split offsets.
- V2: Public defaults, normalized endpoints, exact Copy signal payload/count,
  disabled/empty methods, Select All, Escape, standard shortcuts, focus transfer
  and playground clipboard feedback.
- V3: Forward/reverse cross-block drags, Shift-click anchor extension, ordinary
  link click versus drag/Shift suppression, gap and cell hits, atomic images,
  action-control exclusion, wheel scrolling and host touch scrolling.
- V4: Unicode emoji/combining graphemes, bidi, wrapped/aligned table cells and
  padded inline code; narrow widths and fractional scaling; highlight color
  preset/override/reset, foreground preservation and unchanged layout.
- V5: Replacement/toggle invalidation, masked effective-option changes, image
  completion, resource-policy changes, resizing/fonts/styles, destruction and
  recreation of delegates; independent state in multiple views.
- V6: Focused tests first, then relevant shared/static suites, strict 652-example
  CommonMark checks, QML lint, installed consumers, compatibility and packaging
  on configured minimum/development Qt. Synchronize host Qt per workflow before
  PR verification; require both packaging jobs and resolved review threads before
  squash merge. Inspect versions/API guidance together. No tag/publication.

## Actual results

Implementation was explicitly approved on 2026-10-14. The library, playground,
1.7 versions, installed consumers and documentation are implemented.

- Host Qt synchronization: `python3 scripts/ci-toolchain/qt_versions.py --sync-host qmake6`
  reported 6.12.0; committed configuration already matched, so no target file changed.
- Host focused selection and fractional-scale selection checks pass, including
  projection, normalized offsets, exact Copy signals, graphemes/bidi/alignment,
  padded-code edges, one-pixel-width glyphs, empty lines, enabled/disabled sizing,
  images/policies, table-cell hits, focus/shortcuts, links and host action/scroll routing.
- Host view/styling/fractional regression checks pass. QML lint passes with the
  existing import-only example's unused-import informational message.
- Whitespace and current-version documentation/link checks pass.
- Early matrix attempts exposed font-dependent fixture sizing, action-exclusion
  reset and Qt 6.8 Flickable mouse-grab routing. Focused rechecks pass after the
  corrections. The Qt 6.8 installed consumer also required the existing explicit
  `style: MarkdownStyle { ... }` declaration pattern for revisioned properties.
- Complete local minimum Qt 6.8.0 and development Qt 6.12.0 matrices pass:
  `build-ci-040-minimum-complete` and `build-ci-040-development-complete`.
  Each runs shared/static 16-test suites, strict 652-example CommonMark checks,
  QML lint, relocated linked/plugin-only/static consumers, version compatibility,
  private symbol checks and advisory benchmark smoke. See
  [compact actual results](verification-summary.json).
- The final pending-link-tap cancellation and empty image-selection highlight
  fixes were added after those complete runs. Focused selection and fractional
  checks pass for these final lifecycle changes on
  host Qt 6.12.0 and SDK Qt 6.8.0. Hosted packaging checks will verify the final
  committed head before squash delivery; the prior local snapshots are not claimed
  as final-head evidence.

## Protected delivery

The implementation is prepared on `feat/document-selection-1.7`, based on merged
main. One focused PR will retain all source, tests, documentation and actual local
results. Both required hosted packaging jobs and resolved review threads gate the
authorized squash merge. Hosted execution logs remain in Actions; no evidence-only
PR, tag or publication is authorized or performed.
