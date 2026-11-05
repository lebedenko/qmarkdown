# Inline-code paragraph blur investigation

The sections through Delivery verification record the original stationary study.
The retained motion qualification revision follows that historical evidence.

## Approved diagnostic scope

The owner's supplied implementation plan authorizes this investigation on
2026-11-04: compare plain, bold, italic, link, inline-code and combined paragraphs
at 125%; separate smoothing from device alignment; preserve logical geometry;
measure regular-text edges on software and native Wayland. No production change
or public API change is authorized. This continues feature 065.

Design: a standalone executable links the unchanged library. Each fixture uses
the same visible text from the playground's mixed paragraph (image fallback is
represented by its text). A fixed DejaVu Sans 18 px font, 740 px layout width and
fractional x=24.5 origin make the device phase explicit. This isolates a mechanism;
it does not reproduce unknown owner window geometry/font settings. Only formats
vary across fixtures. Within each fixture, diagnostic surface changes leave
text, wrapping, padding and interaction cells unchanged, checked at every capture.
The aligned control changes the private raster origin and repaints with the
existing painter's inverse translation. It leaves the surface size unchanged.

Tasks completed: standalone harness; six fixtures; current/nearest/aligned
controls for code-bearing fixtures; five stationary ancestor scroll offsets
0/.25/.5/.75/1; capture and metadata retention; regular-prefix edge measurement;
native/software comparison; performance-boundary assessment. No continuous
scrolling or production correction is implemented.

## Reproduction

From the repository root:

```sh
cmake -S specs/065-padded-code-rendering/blur-probe -B /tmp/qmarkdown-blur -G Ninja
cmake --build /tmp/qmarkdown-blur -j2
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software QT_SCALE_FACTOR=1.25 /tmp/qmarkdown-blur/blur-probe /tmp/blur-software > /tmp/blur-software.jsonl
QT_QPA_PLATFORM=wayland QSG_RHI_BACKEND=opengl QT_SCALE_FACTOR=1 /tmp/qmarkdown-blur/blur-probe /tmp/blur-wayland-125 > /tmp/blur-wayland-125.jsonl
```

The native desktop was already at 125%. Setting QT_SCALE_FACTOR=1.25 there
multiplies the scale to 1.5625; that exploratory run is excluded. Check recorded
effective DPR rather than assuming the environment variable is the final DPR.
Native runs require access to the compositor socket and graphics device.

## Actual results — 2026-11-04

Host Qt 6.12.0, native Wayland OpenGL (graphicsApi 3) and offscreen software
(graphicsApi 1), effective DPR **1.25** in both final runs. Both processes returned
0 with all **50 captures** each. Resolved glyph-run families: DejaVu Sans for
body text; DejaVu Sans Mono additionally for inline code. No font fallback family
was observed. Bold, italic and link-only fixtures use a scene-graph surface;
inline code and combined fixtures use one raster surface for the whole paragraph.
Full per-capture physical surface positions, smoothing state, fonts, height and
metrics are in [software records](evidence/software/results.jsonl) and
[Wayland records](evidence/wayland/results.jsonl). PNGs with matching capture names
are stored beside each record file. These are application framebuffer grabs,
not photographs or post-compositor display captures.

The metric is mean absolute horizontal/vertical adjacent-pixel luminance
contrast in the common `Try` prefix, using a 27 × 24 logical-pixel region.
It excludes every formatted word, code background and link decoration. Larger
values mean steeper edges in this particular matched region; this is not a
universal perceptual sharpness score. The region follows the paragraph, with
rounded physical bounds. Font phase and backend differences prevent treating
cross-backend absolute values as an exact parity test.

| Fixture/control | Software contrast range | Native Wayland contrast range |
| --- | ---: | ---: |
| Plain, bold, italic, link (each) | 23.088–23.351 | 23.326–23.571 |
| Inline code / combined, current | 23.156 | 20.292–21.765 |
| Inline code / combined, smoothing off | 23.156 | 23.184 |
| Inline code / combined, aligned and smoothing off | 23.064 | 23.112 |

Native current raster contrast is 6.1–12.5% below its smoothing-off control,
with identical layout and raster origin. Code-only and combined fixtures give
identical prefix measurements: bold/italic/link composition is not necessary
to trigger this result. Software smoothing on/off produces identical metric
values in this region, so software alone would miss the native softness.

At rest the native retained surface starts at physical (27.625,48). At scroll .5
it is (27.625,47.375). With smoothing disabled these same positions remain;
with aligned placement they become (29,48). Native text and aligned raster have
similar edge contrast, but do not claim exact pixel equality. The unchanged
retained raster has integer local coordinates on a common DPR grid; ancestor
placement reintroduces fractional physical texture translation.

[Enlarged native comparison](evidence/comparison.png) shows the plain, retained,
smoothing-off and aligned regular-text regions at scroll .5. Visual inspection
confirms broader grey edges in the retained control. Original full-resolution
captures remain authoritative; the comparison uses nearest-neighbour enlargement.

## Diagnosis and performance boundary

The responsible trigger in this controlled fixture is **padded inline code**:
it routes the complete paragraph through the retained QQuickPaintedItem path.
That path retains its local raster phase and enables linear texture filtering.
Fractional physical placement resamples every glyph in that paragraph, including
ordinary text outside the code. The smoothing-only native control isolates that
filtering contribution. This explains a mechanism consistent with the reported
paragraph softness; the owner's exact paragraph/window has not been captured.

Disabling smoothing keeps the retained placement and is structurally compatible
with zero raster paints during warmed movement, but introduces discrete nearest
sampling and possible motion shimmer. This harness deliberately forces repaints
to compare stationary images: it supplies **no performance qualification** or
continuous-motion quality evidence for that candidate.

Device-aligned placement changes the raster origin during movement, requiring
repainting to preserve glyph placement. It conflicts with approved P2's zero
warmed ancestor-translation raster paints. Do not recommend integrating it under
the existing retained-rendering approval. A correction needs updated requirements,
design/tasks and approval, including motion quality and P5's paired Release
preparation/frame limits (reject repeatable regressions above 10%), surface and
allocation accounting, both exact Qt targets and native fractional rendering.

No production fix is recommended from this stationary study alone. Minimum Qt
6.8.0, Vulkan, other fonts/scales, continuous scroll cost, selection gestures and
physical display/compositor sharpness remain untested. Logical layout pointer,
height and complete text-range interaction rectangles remained unchanged within
each fixture across the diagnostic variants.

## Delivery verification

Standalone configure/build passed on exact host Qt 6.12.0 (the existing QTP0004
warning was nonfatal). The final software and native runs exited 0, each with
50 records and 50 nonempty PNGs; geometry assertions passed. Both final logs
record effective DPR 1.25 and the expected renderer API. `python3 scripts/check.py`
passed, including all 87 Qt-independent tests; `git diff --check` passed.
Shipping files were unchanged, so the production runtime suite was not repeated.

## Retained motion qualification revision — 2026-11-04

The supplied owner-approved plan adds `motion.cpp`, `stage_motion.py` and
`run_motion.py`. The original executable and all earlier captures above remain
historical stationary evidence. Production source is never edited.

```sh
python3 specs/065-padded-code-rendering/blur-probe/stage_motion.py /tmp/blur-source
cmake -S specs/065-padded-code-rendering/blur-probe -B /tmp/blur-candidate -G Ninja -DCMAKE_BUILD_TYPE=Release -DQMARKDOWN_STAGE=/tmp/blur-source/candidate
cmake --build /tmp/blur-candidate --target blur-motion -j2
# Repeat with baseline; repeat both builds with the exact Qt 6.8.0 CMAKE_PREFIX_PATH.
python3 specs/065-padded-code-rendering/blur-probe/run_motion.py /tmp/blur-baseline/blur-motion /tmp/blur-candidate/blur-motion /tmp/blur-results --qt 6.12.0
# Native desktop at 125%, without multiplying its scale:
python3 specs/065-padded-code-rendering/blur-probe/run_motion.py /tmp/blur-baseline/blur-motion /tmp/blur-candidate/blur-motion /tmp/blur-native --qt 6.12.0 --native
QT_QPA_PLATFORM=wayland QSG_RHI_BACKEND=opengl QT_SCALE_FACTOR=1 /tmp/blur-candidate/blur-motion view /tmp/blur-view
```

The runner requires the existing `fc-match` utility and DejaVu fonts. It refuses
an existing output directory and stops on failed gates.
`--continue-matrix` retains failures while exercising independent configurations;
it still returns failure for an incomplete or failing matrix. `--resume` reuses
recorded runs only when executable hashes match and renames interrupted attempt
directories before retrying; it preserves failed gates. It
checks actual Qt/backend/DPR, retains every frame's capture, position, velocity,
moving state and elapsed capture-trace timestamp, and alternates five Release
pairs. Pass `--library-path` for SDK libraries when needed. Performance-only runs
use `--performance-only`; their sparse/dense workload matches the existing
`scrollingWorkload` test's 12 paragraphs, ranges and 120 capture frames. Timing
includes framebuffer capture and is **not display frame rate**. Preparation
includes first capture. Report both raw pair ratios and median ratios; three of
five ratios above 1.1 constitutes a repeatable regression.

Stationary mode uses the original six fixtures and the six existing retained
scroll cases (mixed direction, wrapped adjacent code, tiny right alignment,
each with/without clipping). All sixteen Cartesian phases are physical pixels,
plus origin and return. Native and software control comparison happens in the
same frame: the right viewport starts 800 logical pixels later, giving exactly
the same physical sampling phase for each qualified DPR. Candidate control uses
nearest sampling; baseline control uses linear sampling. The independent image
is constructed once from the painter before warmed counters begin, then uploaded
directly through a texture node. Its image format matches Qt: ARGB for software,
RGBA for native, preserving premultiplied colored-glyph rounding. Subsequent
motion does not call back into text/layout or force paragraph paints. Both
halves must match exactly during drag acceleration, flick deceleration, reversal
and settling. Full character rectangles, surface allocation and layout signals
are checked; mutation gates follow movement. The clip is removed for mutation visibility
checks so that an off-clip focused link cannot produce a false failure. Fontconfig-resolved font files are
hashed and their raw `head` tables checked against actual glyph fonts.

The native preview repeats code/combined/retained fixtures down one scrollable
viewport. Click the label to switch sampling while preserving Flickable state.
One side is the actual linked source variant, the other an independently retained
image with opposite sampling. The automated simultaneous equality gates establish
that this image proxy matches each source's rendering. The label explicitly says
"sampling"; it does not load both shared libraries into one process. Owner review
of resting sharpness, slow shimmer, stepping, reversal and settling remains
required. See [actual qualification results](motion-verification.md).

Summarize retained results without treating gaps as passes:

```sh
python3 specs/065-padded-code-rendering/blur-probe/summarize_motion.py build-ci/065-blur-motion/final /tmp/blur-summary.json
```

`BLUR_FIXTURE=<fixture-id>` narrows direct executable runs for diagnosis. The
qualification runner clears this variable and verifies expected fixture/capture
counts, so a focused run cannot qualify the full matrix.

## Native-only calibration revision — 2026-11-04

The supplied implementation plan approves the appended N1–N5 scope in feature
065. `stage_motion.py` now pins `43d6bba` and exports policy `native-only-v2`.
The earlier unconditional candidate and its results do not qualify this revision.
Production source and all historical evidence remain unchanged.

Build isolated baseline/candidate snapshots as above, using a fresh staging
folder. Run focused calibration before any further qualification:

```sh
python3 specs/065-padded-code-rendering/blur-probe/run_calibration.py /tmp/blur-native-only-baseline/blur-motion /tmp/blur-native-only-candidate/blur-motion /tmp/blur-native-only-calibration --qt 6.12.0 --backend software --scale 1.25
```

Use software scales 1.25/1.5 and native Vulkan `--scale 1` on the desktop at
observed DPR 1.25; repeat with exact Qt 6.8.0. The runner requires a new output
directory and stops at the first unresolved stage. It rejects old source/policy
identities. First real paint instrumentation runs only in `calibrate` mode.
The independent raw raster uses the real paint device settings; the authoritative
retained painted-image control uses those independent pixels. Opacity switching
compares complete framebuffer images at identical coordinates. No tolerance,
alignment adjustment or source update is used. Raw pixels and failures are saved.

The historical preview does not qualify N5. The revised runner below replaces
translated equality with complete same-coordinate replay for N2/N3. A revised
labelled viewer remains downstream of successful automated qualification. Actual
revision results are in
[native-only verification](native-only-verification.md).

The revised `motion.cpp` and `run_motion.py` now include complete static replay
and separate same-coordinate pose-control records. Use the ordered entry point
for this revision rather than invoking the historical matrix commands above:

```sh
python3 specs/065-padded-code-rendering/blur-probe/qualify_native.py /tmp/blur-native-only-min-baseline/blur-motion /tmp/blur-native-only-min-candidate/blur-motion /tmp/blur-native-only-baseline/blur-motion /tmp/blur-native-only-candidate/blur-motion build-ci/065-native-only/final --minimum-library-path /path/to/qt-6.8.0/lib
```

The entry point stops after any failed calibration, replay, renderer or performance
stage. Its fresh evidence directories preserve actual results and all cases not
reached after a guard. It does not launch or qualify the historical review viewer.

`blur-review` is a separate viewer target, so preparing review does not change
measurement executables. It requires all four native correctness stages to pass, uses the baseline
source, and verifies each proxy's raw pixels against both frozen source variants
as an initial raw-pixel check. This does not validate the scrolling composition
(see the owner report below). The toolbar reports a performance rejection
separately; visual review cannot override that gate. Both toolbar controls sit outside the
document: click the baseline/candidate label to switch without resetting scroll;
click `pixelAligned` to change alignment. Run it separately on both native APIs:

```sh
QT_QPA_PLATFORM=wayland QSG_RHI_BACKEND=opengl QT_SCALE_FACTOR=1 /tmp/blur-native-only-review/blur-review build-ci/065-native-only/final /tmp/blur-qualified-review-opengl opengl
QT_QPA_PLATFORM=wayland QSG_RHI_BACKEND=vulkan QT_SCALE_FACTOR=1 /tmp/blur-native-only-review/blur-review build-ci/065-native-only/final /tmp/blur-qualified-review-vulkan vulkan
```

These commands remain guarded until both native comparisons pass on both SDKs.
The viewer selects its evidence by actual Qt version; use the minimum-SDK viewer
and its SDK library path to review Qt 6.8.0. Explicit owner review
must cover resting sharpness, slow-scroll shimmer, stepping, reversal and settling
in both alignment modes on both backends. Silence does not establish acceptance.


Owner feedback on 2026-11-05 invalidates the review viewer's former “validated
image proxy” claim: artifacts depend on scroll position across both SDKs/native
backends/alignment modes, while baseline source looks correct. Initial raw-pixel
checks did not qualify the complete scrolling composition. The current label is
**UNQUALIFIED image proxy**. Existing open windows retain their original binary's
label. See the [owner report](native-only-verification.md#owner-artifact-report--2026-11-05).
Qualification remains stopped; actual candidate-source behavior at those poses
has not been isolated.

## Actual-source artifact diagnosis — 2026-11-05

`blur-review` now links **either** pinned baseline or candidate, selected by
`QMARKDOWN_STAGE` at build time. Its default view is labelled **BASELINE — actual
source** or **CANDIDATE — actual source**. Clicking the left toolbar switches only
to that executable's matching, explicitly unqualified proxy. Native baseline
uses linear sampling; native candidate uses nearest. `scene.h` shares text,
fonts, ranges and paragraph construction with `motion.cpp`. The original
24-paragraph initialization, clipping and 120-pixel spacing are preserved.

Build both targets using fresh stage/build directories as described above. Run:

```sh
python3 specs/065-padded-code-rendering/blur-probe/run_artifact.py /tmp/065-artifact-candidate/blur-review /tmp/065-owner-scan --evidence build-ci/065-native-only/final
python3 specs/065-padded-code-rendering/blur-probe/run_artifact.py /tmp/065-artifact-baseline/blur-review /tmp/065-owner-pose --evidence build-ci/065-native-only/final --pose 320
```

The default scan starts at 240–360 in 0.2-logical-pixel steps (quarter physical
pixel at required observed DPR 1.25), deduplicates actual content positions,
records drag/deceleration/reversal/settling, replays those live frames, then
scans the full remaining range in 0.8-logical-pixel steps. `--pose` instead
replays one diagnostic pose, Cartesian quarter-physical phases, origin and
bottom viewport. `--unaligned` disables pixel alignment; `--backend vulkan`
selects Vulkan. A nonzero exit preserves the failed stage; do not attribute a
proxy mismatch until frozen replay succeeds. Source/proxy captures exclude the
toolbar and cover the full 1000 × 800 physical-pixel document viewport.

For interactive owner inspection, run each **separate executable** directly;
the second argument must name a fresh output directory:

```sh
QT_QPA_PLATFORM=wayland QSG_RHI_BACKEND=opengl QT_SCALE_FACTOR=1 /tmp/065-artifact-baseline/blur-review build-ci/065-native-only/final /tmp/065-owner-baseline opengl
QT_QPA_PLATFORM=wayland QSG_RHI_BACKEND=opengl QT_SCALE_FACTOR=1 /tmp/065-artifact-candidate/blur-review build-ci/065-native-only/final /tmp/065-owner-candidate opengl
```

Use `/tmp/065-artifact-min-baseline/blur-review` and
`/tmp/065-artifact-min-candidate/blur-review` for exact Qt 6.8.0; set its SDK `lib`
directory in `LD_LIBRARY_PATH` if necessary. Repeat with Vulkan and both toolbar
alignment states. These viewers supply actual source for inspection; they do
not revoke the performance rejection or establish owner motion acceptance.
See [diagnosis and actual verification](artifact-verification.md).
