# 062 — Verification

## Planned checks

- Syntax/resolution: forward and repeated references; normalized labels; undefined,
  empty, escaped and malformed markers; first-definition wins; visible duplicates;
  unused definitions; containers and four-space continuation; paragraph interruption;
  code/HTML/formula protection; excluded nested/link/image references.
- Rendering/projection: superscript geometry and atomic marker selection; exact
  `[n]` references and `n. ` endnote bodies; ordinary rich inline/block bodies;
  enabled tables, math and highlighting; image/resource/custom-renderer policies;
  no backlink text in copy/Find; asynchronous offset stability.
- Navigation: heading stability, generated fragment collisions, repeated occurrence
  targets/backlinks, note-body outline, host navigation/activation signals, Tab and
  Backtab, Enter, accessible names and geometry, focus-chain exit.
- Delivery: disabled pinned CommonMark baseline, older imports, independent switches,
  installed shared/static consumers, math/highlighting ON/OFF, affected suites,
  QML lint/docs and exact Qt 6.8.0/6.12.0 packaging gates. Record native-input
  acceptance independently from offscreen automation.

## Actual results

At draft time only specifications existed. The user approved requirements, design
and T1–T7 on 2026-10-19, conditional on Feature 061's CI gates. Both exact Qt gates
passed on PR #41 at b00384c before footnote implementation began. No merge or
publication of that dependency was performed.

Implementation and focused verification on 2026-10-20:

- Parser cases pass for forward/repeated references, normalization, duplicate/
  unused/undefined behavior, code/link/image/math protection, rich note bodies and
  container continuations. A regression closes notes when enclosing quote/list
  prefixes disappear, preventing lazy continuation outside the note container.
- Native footnote tests pass: exact numbered copy/Find, reference atom geometry,
  collision-free generated fragments, note-body outline, keyboard and accessible
  links, per-occurrence backlinks, allowed/denied note images and custom fenced
  renderer precedence. Offscreen capture `/tmp/062-footnotes.png` was visually
  inspected. This does not establish native-input acceptance.
- Initial math OFF checks (math/import/footnotes: 3/3) and highlighting ON checks
  (math/footnotes/syntax/syntax-view: 4/4) pass. They are refreshed after the final
  enclosing-container continuation correction.
- A working-tree 1.16.0 source archive includes both private syntax scanner headers.
  The explicit allowlist, docs and whitespace checks pass. Host Qt sync remains
  6.12.0; private symbol validation reports 146 prefixed definitions.

Final local verification:

- Final shared suite: 33/33 pass (`/tmp/062-verified-shared-tests.log`).
- Final static suite: 33/33 pass (`/tmp/062-verified-static-tests.log`).
- Parser: 400 checks pass, including the final 1000-byte label boundary correction.
- Pinned CommonMark and footnote-focused checks pass after that correction.
- The full packaging driver finishes with `PASS: shared and static relocated
  package consumers`, including the standalone library-only build. Final relocated
  footnote consumers were refreshed and pass for shared module linking ON/OFF and
  static ON (`/tmp/062-final-consumers.log`).
- QML lint, ordinary docs, whitespace, scanner archive inclusion and 146-symbol
  isolation pass. Host Qt remains 6.12.0.

Final refreshed backend checks pass: math OFF math/import/footnotes 3/3;
highlighting ON math/footnotes/syntax/syntax-view 4/4. Required hosted results are
recorded in the implementation PR so final-head evidence does not invalidate its tree. The unchanged local Docker/buildx preparation barrier from Feature
061 prevents local exact-container gates; hosted Qt 6.8.0/6.12.0 gates remain
required. Native desktop input acceptance is unavailable in this session; automated
offscreen tests are recorded separately. Merge, tagging and publication are excluded.
