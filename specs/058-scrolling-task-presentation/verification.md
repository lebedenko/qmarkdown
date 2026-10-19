# Actual verification

Verified locally on 2026-10-19 with Qt 6.12.0, GCC 16.2.1 and offscreen Qt Quick.
Package 1.13.0 / QML 1.13 remain unchanged. `sync-host /usr/bin/qmake6` confirmed
configured development Qt 6.12.0 without changing the target file.

## Provider behavior

- Code-block regressions cover angle wheels and phased pixel input (including a
  moving ScrollBegin), both edges, edge-reaching events consumed completely,
  reversal, short blocks, horizontal input, Shift-wheel, and an observing parent
  WheelHandler configured to permit propagation.
- Replacement-marker regressions cover tight/loose, checked/unchecked, empty,
  wrapped, mixed/nested/ordered tasks; 45×60 and live 73×85 artwork; eight-pixel
  text gaps, two-pixel indicator clearance, font/width/option changes, clipping
  and at least one content pixel. Selection endpoints/plain-text payload, Find
  navigation rectangles, link hit testing and read-only accessibility pass.
- Both marker policies pass light/dark image comparisons against Controls artwork
  in Fusion, Basic and Material at scales 1, 1.25 and 2. Fallback and host artwork
  are included. Fusion replacement capture was also visually inspected.
- Shared and static installed task consumers pass against isolated `/tmp` prefixes.
  A public-API-only wheel harness linked to the isolated shared installation passes
  all seven QtTest outcomes. Markdown Viewer was configured and built under `/tmp`
  against that installation; its offscreen `--smoke-test` exits 0. Sibling sources,
  dependency pins and system packages were not modified.

## Implementation findings

A literal `WheelHandler.blocking: true` rejects QML events at edges but leaves the
underlying event point accepted on host Qt. The private native SourceWheelArea
uses public Qt event delivery and sets event and point acceptance from actual
movement. It initializes enclosing native gesture state with a zero-movement begin
when consuming a moving ScrollBegin. Native outer drag thresholds apply after
handoff; consumed source deltas do not preload them. The math handoff regression
now supplies a third unconsumed pixel frame for that threshold. Baseline CodeSurface
was temporarily rebuilt to isolate this difference, then the candidate was restored.

Static verification also exposed duplicate backend definitions when Qt scanned
adjacent QML fixtures into the standalone syntax unit target. Import scanning is
now disabled only for that target, which already compiles its backend directly.

## Limits and host integration

The sibling viewer's existing parent `WheelHandler` leaves `blocking` true. A
provider-independent fixture reproduces suppression of native angle-wheel scrolling
in a plain Flickable with that observer on Qt 6.12.0. Setting `event.accepted = false`
alone does not undo point acceptance. The corrected observer fixture and guidance
use `blocking: false`; the unchanged sibling viewer still needs that host correction.
This is distinct from exclusive provider scrolling and prevents claiming full native
acceptance of the unchanged viewer.

Physical mouse-wheel/touchpad acceptance in a native harness and Markdown Viewer
was unavailable: this session exposes no physical input or native-app control.
Offscreen synthetic events and viewer startup are not native hardware acceptance.

Both configured packaging runners were attempted via `python3 scripts/run-ci.py
--qt-role minimum` / `development`. Both stopped before tests at image preparation:
Docker buildx is missing and the legacy builder rejects `--progress` (exit 125).
No exact Qt 6.8.0 local packaging result is claimed. Hosted required-check results
are recorded with delivery below.

## Final checks and delivery

- `cmake --build build-shared --parallel 4` and the equivalent static build: pass.
- `ctest --test-dir build-shared --parallel 4 --output-on-failure`: **32/32 pass**
  (27.90 seconds); static: **32/32 pass** (26.60 seconds).
- `all_qmllint` in both builds: pass. The existing import-only example emits an
  informational unused-import message; no new QML warnings remain.
- `python3 scripts/check-docs.py` and `git diff --check`: pass.
- Final isolated shared/static installed task consumers: exit 0. Installed wheel
  harness: 7/7 outcomes pass. Isolated viewer offscreen startup: exit 0.

Local logs are under `/tmp/qmarkdown-058-candidate-*`. Earlier focused/baseline
investigation logs use `/tmp/qmarkdown-058-*`; they are not release assets.

The feature branch `fix/nested-scrolling-task-presentation` starts at unchanged
`0b22f44`. Push and PR creation are authorized; merge and release are excluded.
Hosted `packaging (minimum)` and `packaging (development)` status must be read from
the final PR head; local Docker preparation failures are not successful gates.
