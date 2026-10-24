# Production retained paragraph verification

Approved scope: P1–P6 in [requirements](requirements.md). Historical per-run
experiments and failures remain unchanged, including the preserved PR #54 branch.

## Regression and implementation

The new warmed-scroll regression was first run against current main
`96c530660aab25654cd611c70af68affdb58761d`, with only private paint-count
instrumentation and the test added. Qt 6.12.0/software/DPR 1 failed on the first
fractional translation: paint count **2**, expected **1**. Surface local position
also changed from (-1,-1) to (-1.25,-1).

The production change retains the existing Qt-owned QQuickPaintedItem in local
coordinates for padded, atom-free text on supported DPR grids and enables linear
filtering. Ancestor callbacks leave placement and pixels unchanged. DPR lifecycle
notifications explicitly repaint even when logical bounds remain equal. There
is no new image cache, resource owner, public API, dependency or version change.

## Focused results

Passed on exact Qt **6.8.0 and 6.12.0**:

- Software/offscreen: DPR **1, 1.25, 1.5, 2**, all eight configurations passed
  (15 test cases/configuration, zero failures/skips).
- Native XCB: **OpenGL and Vulkan**, DPR **1.25 and 1.5**, all eight
  configurations passed. Runtime QSG logs confirm the requested APIs on the
  NVIDIA GeForce GTX 1660 SUPER, NVIDIA driver **615.78.08** (Vulkan identifies
  driver 615.312.512, API 1.4.351); no software GPU fallback.
- Unsupported DPR **1.25081** fallback check passed on host Qt 6.12.0.

The committed tests cover
mixed-script/decorated rounded code, wrapped adjacent linked strikeout code and
tiny-width right-aligned code, each clipped and unclipped. They compare motion
with an independently retained image and exact origin restoration; require zero
raster paints/layout notifications and stable surface identity/bounds/cursor
geometry; and check content, colour, padding, corner radius, width, selection,
Find and focus invalidation. Existing link/selection/copy/decorations/contrast
regressions remain in the full runtime suite.

The actual Flickable trace includes fractional contentY, velocity/deceleration
and reversal, comparing both images in the same captured frame. Its 1024-logical-
pixel window fits the native desktop at fractional DPR. An initial 1920-wide
control was constrained to 1705 at DPR 1.5, invalidating the midpoint comparison;
the smaller reproducer resolved that test failure without relaxing equality.
The independent image painter explicitly matches production antialiasing.

The DPR-transition regression uses software paint-device render targets for
DPR 1 → 1.25 → 1.5 → 2 → 1 and injects the native DPR item notification. This
checks the production handler deterministically on one screen; it is not a
physical multi-monitor migration test. Reparenting and three window recreation
cycles preserve logical geometry and one surface. Reference atoms remain
unsmoothed and repaint on movement; unsupported DPR 1.25081 preserves fallback.

## Five paired Release runs

Host Qt **6.12.0**, software/offscreen, DPR **1.25**, GCC 16.2.1; Release builds
of main and candidate, identical test workload and private counters. Five fresh
process pairs alternate main/candidate order. Each workload warms 12 padded
paragraphs, then replays 120 fractional vertical translations. Sparse formats
6 characters per paragraph; dense formats 96. Preparation includes constructing
the paragraphs and the first presented capture. Trace frame duration includes
processEvents and grabWindow; these are capture-trace timings, not display FPS.

Medians of the five paired measurements:

| Workload | Metric | Main | Candidate |
| --- | --- | ---: | ---: |
| Sparse | Warm raster paints | 1,428 | 0 |
| Dense | Warm raster paints | 1,428 | 0 |
| Sparse | Preparation | 13.741 ms | 12.794 ms |
| Dense | Preparation | 4.908 ms | 4.774 ms |
| Sparse | Frame median / p95 | 1.346 / 2.145 ms | 0.462 / 0.962 ms |
| Dense | Frame median / p95 | 1.468 / 2.498 ms | 0.393 / 0.820 ms |
| Both | Existing surfaces | 12 | 12 |
| Both | Image allocation bytes | 1,303,200 | 1,303,200 |

No repeatable preparation or trace-frame regression above 10% was observed.
The measured benefit is eliminated warmed rasterization with unchanged surface
count/allocation and lower capture-trace time. No interactive frame-rate gain is
claimed. An earlier five-pair run before the DPR-handler fix independently found
zero candidate paints and the same allocation; the table uses the final fix.

Reproduce via `qmarkdown-styling-test scrollingWorkload` in a Release build with
`QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software QT_SCALE_FACTOR=1.25`.
The main comparison exports the named main commit and overlays only the test,
private counter/friend and counter increment. No placement/filtering changes are
applied to that baseline. Raw paired JSON, commands, binaries/builds, diagnostic
captures and complete local logs remain ignored under
`build-ci/065-production/`; no captures are committed.

## Repository and delivery checks

`task check` and `git diff --check` passed. Final-diff Core passed on both exact SDKs: 29/29 shared runtime tests and
30/30 installed consumers plus 30/30 relocated consumers per target, QML lint
and symbol privacy. Local artifacts:
`build-ci/6.8.0/20261024T185545Z-9yk7g6uo` and
`build-ci/6.12.0/20261024T185543Z-deocyamy`.
Hosted final-head outcomes and PR delivery status are recorded in the production
PR body/checks; they are not inferred from local success. Merge,
auto-merge and release are excluded from this approval.
