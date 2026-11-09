# Smoothing-off retained motion qualification

> Storage cleanup: generated artifacts linked below were removed from Git history.
> See the [retention policy](../../../docs/artifact-storage.md); historical results remain, raw-data revalidation requires regenerated inputs.

Diagnostic revision approved by the owner's supplied implementation plan on
2026-11-04. No shipping source, public API, dependency or package changed. Earlier
stationary diagnostics and captures remain intact. This record distinguishes
actual results from the approved checks in feature 065.

## Source and instrumentation

Both isolated snapshots export commit
`8665b5df53c6da339610e8a84a9e27aebff2058b`. Identical `BlurProbe` friendship exposes
existing raster counters and complete private cursor/link/code rectangles only.
The candidate changes `setSmooth(retained)` to `setSmooth(false)` in the staged
renderer; atom and unsupported-DPR paths already use false and retain placement.

Renderer SHA-256:

- Baseline: `1dae1a962daec40cc4ee9b0e55c0a2563deeb3085e90275a71354f78ff77b451`
- Candidate: `768c73ce9acf793fbe8a7f6ef082c4fe58302b5e633226b44db15491b1a5b369`

Every run records its executable hash, actual Qt, platform, graphics API, DPR and
source manifest. Glyph font files are resolved through Fontconfig, SHA-256 hashed
and checked against actual glyph-font `head` tables. Native device/backend details
remain in `stderr.log` alongside each trace. No fallback backend is accepted.

## Harness corrections during qualification

Exploratory runs are excluded from final evidence:

- Appending content beyond a clip need not change visible pixels. Content mutation
  now changes visible text. The clip is removed only after movement, before
  mutation checks, so off-clip focus rectangles can be visibly tested.
- A manually constructed ARGB control differed from native painted textures in
  a few translucent colored glyph pixels. The independent image now uses native
  RGBA or software ARGB and uploads directly to its own texture node. The focused
  native candidate mixed-direction stationary check then passed exact equality.
- Frame-count-based reversal could occur after settling when capture was slow.
  Final traces reverse after 120 ms, assert forward motion is still active, retain
  both velocity signs and continue until settling. Capture timing is not display
  frame rate.

Final evidence is stored separately under `build-ci/065-blur-motion/final/`.
Earlier `/tmp/blur-*` and `build-ci/065-blur-motion/native-6.12.0/` exploratory
artifacts are not qualification results. They have not been deleted.

## Final matrix and outcome

Final matrix execution is complete and qualification is rejected. No production correction is recommended.
Native perceptual acceptance has not been supplied by the owner.

The candidate's Vulkan stationary comparison failed on **both exact Qt targets**
at fixture `mixed-decorated`, phase `0-2` (X 0, Y .5 physical pixels from origin).
The required equality gate remains failed. At the failing frame, raster paints
and layout rebuilds were zero, and bounds, allocation and interaction checks had
passed. Qt 6.8.0's baseline Vulkan motion comparison also failed equality, so this
evidence does not isolate a production regression caused by disabling smoothing.
A focused repeat with the existing painted-image control type also failed the
Qt 6.12.0 candidate at the same phase. No tolerance was substituted for equality.

The left half is the candidate paragraph; the right half is the independently
retained nearest image. Captures are unmodified application framebuffer grabs:
Qt 6.12.0 failure (removed artifact: `evidence/motion/qt-6.12.0-vulkan-nearest-failure.png`) and
Qt 6.8.0 failure (removed artifact: `evidence/motion/qt-6.8.0-vulkan-nearest-failure.png`), with
[6.12.0 metadata](evidence/motion/qt-6.12.0-vulkan-nearest-failure.json) and
[6.8.0 metadata](evidence/motion/qt-6.8.0-vulkan-nearest-failure.json).
The 6.12.0 image differs in 87 pixels between halves, maximum channel difference
247, within physical bounds (144,126)–(390,134) of the left half. The cause of this
sampling mismatch is unresolved; owner acceptance cannot replace this failed gate.


## Confirmed software performance rejection

The completed Qt 6.12.0 software DPR 1.25 five-pair Release run failed the approved
10% frame-cost limit in **all five pairs**, for both sparse and dense workloads.
Median paired candidate/baseline ratios:

| Workload | Preparation | Frame median | Frame p95 |
| --- | ---: | ---: | ---: |
| Sparse | 0.900 | 1.819 | 2.005 |
| Dense | 1.040 | 1.806 | 1.993 |

Raster paints remained zero and retained image allocation stayed equal. These are
capture-trace costs, including event processing and framebuffer capture, without
PNG encoding or image-control comparison. They are not display frame rates. The
workload preserves the existing 12 sparse/dense paragraphs, widths, ranges,
spacing and 120 movement offsets; this harness prepares them in an already
exposed Flickable window. Preparation covers text creation through first capture,
not application/window startup. Image allocation bytes are derived from retained
surface dimensions and DPR, not total GPU memory.

This repeatable regression independently rejects smoothing off under the supplied
acceptance criteria. No production correction is recommended, irrespective of a
later favorable motion-quality review.

## Completed matrix — 2026-11-04

All requested environments were available and attempted: exact Qt 6.8.0 and
6.12.0, offscreen software DPR 1/1.25/1.5/2, native Wayland OpenGL and Vulkan
at observed DPR 1.25. Native GPU: NVIDIA GeForce GTX 1660 SUPER.

| Environment | Qt 6.8.0 baseline / candidate | Qt 6.12.0 baseline / candidate |
| --- | --- | --- |
| Software DPR 1 | pass / pass | pass / pass |
| Software DPR 1.25 | control equality failed / pass | control equality failed / pass |
| Software DPR 1.5 | control equality failed / pass | control equality failed / pass |
| Software DPR 2 | pass / pass | pass / pass |
| Wayland opengl DPR 1.25 | pass / pass | pass / pass |
| Wayland vulkan DPR 1.25 | control equality failed / control equality failed | pass / control equality failed |

“Pass” covers all 12 fixtures, Cartesian phases/return, real drag/flick reversal
and settling with pixel alignment off/on, and subsequent renderer mutation checks.
Every recorded warmed movement frame and every workload run retained zero raster
paints/layout rebuilds, stable surface identity/bounds and interaction rectangles.
Complete baseline/candidate geometry comparisons passed on OpenGL and software
DPR 1/2. The fractional-software and Vulkan control guards interrupted some runs;
fixtures/phases/mutations after those guards remain unverified. No paired fixture
that was actually recorded showed a geometry or font-file mismatch.

The unchanged software baseline also failed its **linear image proxy** equality
at DPR 1.25/1.5. This is a control-parity gap, not evidence of a newly introduced
baseline regression. See the representative capture (removed artifact: `evidence/motion/qt-6.12.0-software-1.25-baseline-control-failure.png`)
and [metadata](evidence/motion/qt-6.12.0-software-1.25-baseline-control-failure.json).

Five alternating Release pairs ran per Qt target on software DPR 1.25 and native
OpenGL DPR 1.25. Workload timings for Vulkan and other DPRs were not measured.
All pairs had zero warmed paints, 12 surfaces, equal retained image bytes and
matching font files. Preparation/frame medians below are medians across five runs;
B/C means baseline/candidate. All values are **capture-trace milliseconds**, not FPS.

| Target / backend | Workload | Preparation B/C | Frame median B/C | Frame p95 B/C | Image bytes B=C |
| --- | --- | ---: | ---: | ---: | ---: |
| 6.12.0 / native | Sparse | 44.761 / 43.214 | 2.031 / 2.037 | 2.450 / 2.391 | 1,303,200 |
| 6.12.0 / native | Dense | 6.065 / 6.150 | 2.034 / 2.036 | 2.457 / 2.434 | 1,303,200 |
| 6.8.0 / native | Sparse | 38.450 / 38.429 | 2.008 / 1.996 | 2.523 / 2.435 | 1,303,200 |
| 6.8.0 / native | Dense | 6.142 / 6.219 | 1.992 / 1.993 | 2.354 / 2.418 | 1,303,200 |
| 6.12.0 / software | Sparse | 6.257 / 8.034 | 0.265 / 0.480 | 0.424 / 0.850 | 1,303,200 |
| 6.12.0 / software | Dense | 2.653 / 2.750 | 0.270 / 0.489 | 0.431 / 0.864 | 1,303,200 |
| 6.8.0 / software | Sparse | 9.700 / 19.382 | 0.246 / 0.451 | 0.298 / 0.505 | 1,303,200 |
| 6.8.0 / software | Dense | 2.490 / 2.638 | 0.345 / 0.451 | 0.379 / 0.745 | 1,303,200 |

Both software targets failed the repeatable 10% frame-cost gate. Native OpenGL
pairs passed it. See [machine-readable results](evidence/motion/summary.json) for
all paired ratios, binary/source identities, font paths/hashes and raw trace hashes.
Raw records, every motion-frame PNG, positions/timestamps/velocity and backend logs
remain under `build-ci/065-blur-motion/final/`. Interrupted attempts are preserved.

## Native review and delivery

Native OpenGL source/image-control equality passed on both SDKs. The switchable
viewer uses one actual source variant and the opposite sampling image proxy in
the same viewport, preserving content position. It repeats the code/combined and
retained-scroll fixtures; click the header to switch, drag slowly, reverse and
release to inspect settling. Labels remain outside the document.

Baseline resting capture (removed artifact: `evidence/motion/qt-6.12.0-opengl-baseline-halfphase.png`) and
candidate resting capture (removed artifact: `evidence/motion/qt-6.12.0-opengl-candidate-halfphase.png`)
show the same combined fixture/half-pixel Y phase. These are framebuffer captures;
they do not establish perceptual acceptance or post-compositor display quality.

Owner review remains pending. Automated rejection stands independently of that
review. No production correction, merge or publication is recommended or performed.
Any later production correction requires an approved retained-rendering specification
revision and resolution of the failed gates.

Standalone Release builds passed for both variants on exact Qt 6.8.0 and 6.12.0.
Repository lightweight validation and whitespace results are recorded below.
Shipping files and original stationary captures were preserved; no shipping runtime
suite or package checks were repeated for this standalone diagnostic change.


`python3 scripts/check.py` passed, including **87/87 Qt-independent tests**,
configuration/evidence/Python syntax and documentation links. `git diff --check`
passed. Snapshot comparison found only the expected renderer/manifest differences;
both renderer files matched their recorded SHA-256 values.

The native review window was opened on exact Qt 6.12.0, Wayland OpenGL (API 3),
observed DPR 1.25. Review remains pending; the window is diagnostic only and its
visual outcome cannot override the automated rejection. Reopen it with:

```sh
QT_QPA_PLATFORM=wayland QSG_RHI_BACKEND=opengl QT_SCALE_FACTOR=1 /tmp/blur-motion-candidate/blur-motion view /tmp/blur-native-review
```
