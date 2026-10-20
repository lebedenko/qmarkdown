# 061 — Verification

## Planned checks

- Parser: both delimiters; odd/even escapes; dollar runs, whitespace, currency/digits,
  empty/unmatched/multiline cases; literal emphasis/entities/backslashes in bodies;
  code/HTML/destinations/titles/image protection; references, containers and table pipes.
- Model: exact normalized body/source spans, UTF-16 offsets after supplementary
  characters, heading/outline text, text/image slicing and stable copy/search strings.
- Layout: surrounding font sizes, scale/color, text/display metrics, line heights,
  narrow fallback, complete-atom wrapping, RTL/mixed bidi, code padding and links.
- Lifecycle: pending/failure/OFF/saturation fallback, replacement/destruction,
  independent switches, stale results, cache mode isolation and asynchronous relayout.
- Interaction: atomic hit/drag/repeated-click selection, partial-source retained
  selection, exact clipboard request text, active Find and closing-Find offsets,
  link traversal/accessibility, images/tables, resources and custom renderer regression.
- Delivery: older imports, default-off pinned CommonMark corpus, math ON/OFF,
  installed shared/static consumers, affected suites, QML lint, docs and exact Qt
  6.8.0/6.12.0 packaging gates. Native pointer/keyboard acceptance is recorded
  independently of automated/offscreen checks; unavailable checks get exact reasons.

## Actual results

2026-10-19: inspected parser, owned inline/block model, FormattedText glyph layout,
math worker, existing math guide and current delivery workflow. Prepared draft
requirements/design/tasks. No runtime changes or runtime verification performed.
`python3 scripts/check-docs.py` passed (current 1.14.0 documentation and local
links); `git diff --check` passed. Implementation,
At draft time, implementation and runtime/packaging checks were pending approval;
the explicit user approval below supersedes that status.

## Approved implementation — 2026-10-19

The user explicitly approved requirements, design and T1–T7. Parser/model, shared
math requests, measured text atoms, projection mappings, QML 1.15 opt-in, playground
and development documentation are implemented. Published 1.14 summaries remain.

Actual checks on host Qt 6.12.0:

- Parser: 397 checks pass, including both delimiters, literal bodies, escaped/run/
  currency/Unicode-whitespace cases, protected contexts, table pipes, container
  projections, supplementary-character offsets and table source coordinates.
- Focused math tests cover LTR/RTL/mixed atoms, full-atom cursor/selection geometry,
  partial-source selection retention, wrapping/narrow fallback, adjacent zero-advance
  atom line breaks, replacement, QML table/heading/link projection and active Find.
  Existing block math regressions remain in the same suite.
- Math OFF: math/import checks pass (2/2). Highlighting ON with math ON: syntax,
  syntax-view and math checks pass (3/3).
- `all_qmllint`, ordinary documentation validation, `git diff --check` and bundled
  symbol isolation pass. Development host synchronization reports 6.12.0 unchanged.
- Offscreen capture at `/tmp/061-inline.png` was visually inspected: heading,
  linked formula and table-cell formulas render with native surrounding text.
  This is automated/offscreen evidence, not native-input acceptance.
- A generated 1.15.0 source archive includes the new private scanner from the
  explicit distribution allowlist. No source archive is published.

Intermediate failures were corrected before the final checks: RTL atom edge hits,
adjacent zero-advance atom wrapping, the playground sample count, development
changelog ordering required by the docs gate, and an installed-consumer finder
that initially matched a containing TextBlock rather than its painted text item.
An early broad run overlapped my library relink and is invalid evidence (loader
reported “file too short”); subsequent suites run after their builds finish.

Local exact Qt 6.8.0 and 6.12.0 container attempts both stop before compilation:
the installed Docker uses the legacy builder, lacks buildx and rejects
`docker build --progress` (exit 125). Logs and preparation artifacts are under
`/tmp/qmarkdown-061-minimum` and `/tmp/qmarkdown-061-development`. Hosted final-head
`packaging (minimum)` and `packaging (development)` checks remain required.
Native desktop pointer/keyboard acceptance was not run: this session provides
o native computer-control surface; offscreen Qt tests do not establish it.

Final host shared suite: 32/32 pass (`/tmp/061-verified-shared-tests.log`).
Final host static suite: 32/32 pass (packaging resume log). Relocated consumers pass
for shared module linking ON/OFF and static module linking ON, including rendered
inline/block math, exact body copy and 1.15 imports. The packaging driver resumed
from unchanged verified shared build/test/install stages after fixing the consumer
finder; it then performed static build/test/install and consumer checks. After the
final source-coordinate correction, the shared library was rebuilt, all 32 tests
passed again, and the relocated shared installation/math consumers were refreshed.

Logs are `/tmp/061-packaging-final.log`, `/tmp/061-packaging-resume.log` and
`/tmp/061-installed-shared-refresh.log`. The remaining standalone library-only build
and final-head hosted gate results are recorded in the implementation PR, alongside
PR/check links, so evidence updates do not invalidate the verified tree.
Merge, tagging and publication are excluded.
