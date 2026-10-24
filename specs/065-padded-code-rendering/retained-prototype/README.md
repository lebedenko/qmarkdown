# Feature 065 standalone software gates

All executables here are excluded from shipping CMake. See the whole-run
continuation below for its passing software gate; the scene-graph continuation
at the end is the current experiment. The historical standalone diagnostic deliberately stops
before scene-graph texture creation: the preparation image gate fails. It is not
a retained scrolling renderer or a performance benchmark.

From the repository root (choose new, nonexistent staging destinations):

```sh
python3 specs/065-padded-code-rendering/retained-prototype/stage.py /tmp/qmarkdown-065-retained-stage
cmake -S specs/065-padded-code-rendering/retained-prototype \
  -B /tmp/qmarkdown-065-retained-build \
  -DQMARKDOWN_STAGE=/tmp/qmarkdown-065-retained-stage
cmake --build /tmp/qmarkdown-065-retained-build --target retained-glyph-gate --parallel 4
cd /tmp/qmarkdown-065-retained-build
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software GATE_DPR=1 ./retained-glyph-gate
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software GATE_DPR=1.25 ./retained-glyph-gate
```

The latter command returns 1 for the retained reproducer. Images are written in
the current directory. `GATE_DPR` controls QImage DPR, not a window/screen's DPR.
The tool uses QPainter's software raster engine; it does not create a Quick
window or claim a runtime-confirmed scene-graph backend. Select an exact Qt SDK
with `CMAKE_PREFIX_PATH`; use a separate build directory for each SDK.

The patch exposes prepared runs by value and invokes existing preparation only
in the staged source. `stage.py` prints the exported HEAD identity. The harness
uses the actual font/index/position/color, never substring shaping. Each tile
contains outward-rounded raw-font ink bounds plus two physical transparent
pixels. Its drawing origin retains the local fractional physical raster phase.
QRawFont equality, index, color, DPR and phase identify shared per-surface images.
The original glyph order is retained. Decorations are disabled in both glyph
paths; the fixture has none. No backgrounds, overlays or atoms are exercised.

The direct control draws every prepared glyph separately at its original global
position without a tile. The reference draws whole prepared runs. The candidate
composites tiles at integer physical image origins with smooth image filtering
disabled (QPainter default). Border ink checks catch tile-edge ink. Changed pixel
counts are exact premultiplied ARGB comparisons; lost/extra mean alpha support
vanishes/appears, not proof of geometric clipping. This conservative gate accepts
no changed ink at matching phase. Its rejection concerns this implementation;
it does not demonstrate that all per-glyph strategies fail.

Further shaping, padding, scene-graph transfer, filtering, overlays, lifecycle,
retention and cost work is conditional on a passing preparation gate. See the
[actual report](../retained-verification.md) for results and limits.

## Coordinate and metadata diagnostic (2026-10-23)

The separate `retained-glyph-diagnostic` executable leaves the earlier gate intact.
Use the same staging command above with a fresh destination, then configure separate
fresh builds for exact Qt 6.12.0 (host `/usr/bin/qmake6`) and 6.8.0
(`/opt/Qt/6.8.0/gcc_64` in the existing CI container). Recorded source export:
`4baf0fb934ced7b732267e1ed83bbab529cf0ef9`.

```sh
python3 scripts/ci-toolchain/qt_versions.py --sync-host /usr/bin/qmake6
python3 specs/065-padded-code-rendering/retained-prototype/stage.py /tmp/qmarkdown-065-diagnostic-stage
cmake -S specs/065-padded-code-rendering/retained-prototype \
  -B /tmp/qmarkdown-065-diagnostic-612 \
  -DQMARKDOWN_STAGE=/tmp/qmarkdown-065-diagnostic-stage
cmake --build /tmp/qmarkdown-065-diagnostic-612 \
  --target retained-glyph-diagnostic retained-glyph-gate --parallel 4
mkdir -p /tmp/qmarkdown-065-diag-evidence/qt-6.12.0
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
  DIAG_CAPTURES=/tmp/qmarkdown-065-diag-evidence/qt-6.12.0/final-captures \
  /tmp/qmarkdown-065-diagnostic-612/retained-glyph-diagnostic \
  > /tmp/qmarkdown-065-diag-evidence/qt-6.12.0/results.jsonl
```

For minimum Qt, mount the repository read-only at `/source` and `/tmp` at `/work`,
UID/GID 1000, `LANG=C.UTF-8`, `QT_QPA_PLATFORM=offscreen`,
`QT_QUICK_BACKEND=software`, using image
`qmarkdown-ci:6.8.0-amd64-55e391d786afbc0381181ed237876dbc3f4d18be369418fd74fb41aa5c5cd1f2`.
Inside that container:

```sh
cmake -S /source/specs/065-padded-code-rendering/retained-prototype \
  -B /work/qmarkdown-065-diagnostic-680 \
  -DQMARKDOWN_STAGE=/work/qmarkdown-065-diagnostic-stage \
  -DCMAKE_PREFIX_PATH=/opt/Qt/6.8.0/gcc_64
cmake --build /work/qmarkdown-065-diagnostic-680 \
  --target retained-glyph-diagnostic retained-glyph-gate --parallel 4
mkdir -p /work/qmarkdown-065-diag-evidence/qt-6.8.0
DIAG_CAPTURES=/work/qmarkdown-065-diag-evidence/qt-6.8.0/final-captures \
  /work/qmarkdown-065-diagnostic-680/retained-glyph-diagnostic \
  > /work/qmarkdown-065-diag-evidence/qt-6.8.0/results.jsonl
```

Matrix origins compensate the first prepared glyph baseline so its physical phases
are exactly the requested values. Each full run emits 258 cases (two historical origins plus the 256-case matrix),
then returns 1 if any selected comparison differs. It never stops on a mismatch.
`DIAG_COMPARE` selects comma-separated comparison names, e.g.
`reference->preserved,preserved->translated`; default `all` reports eleven pairs.
All paths still execute with selection, preserving control ordering and cache state.
`DIAG_CAPTURES` selects the PNG destination. Historical cases and the first failing
case per DPR/comparison receive reference/candidate/difference PNGs. Difference
channels show maximum premultiplied channel delta amplified 16x.

`preserved` copies the whole QGlyphRun then replaces indexes and positions;
`reconstructed` builds a new run with the same raw font/index/position.
`local` uses the historical zero-position local formula; `translated` retains
positions and applies an integer physical translation to the painter. `crop`
paints one glyph on a full canvas and copies the integer rectangle. These three
compose at integer physical coordinates with image DPR temporarily set to 1;
`legacy` uses local rasterization and the historical logical-coordinate compositor.
All painters use default identical settings (SourceOver, no smooth image transform),
transparent premultiplied ARGB canvases and disabled run decorations. No reordering
or substring shaping occurs. Counts represent occurrences, with no cache deduplication.
Per-glyph logs contain raw font identity, flags, coordinates, phase, bounds, image
DPR and tile transforms. The canvas transform is scale(DPR,DPR), with glyph draw
origin logged separately. Overlap counts locate reference/direct differences within
or outside the support of two or more independently painted glyphs.

Run the unchanged `retained-glyph-gate` in separate processes with `GATE_DPR=1`
and `GATE_DPR=1.25` to reproduce the historical results independently of diagnostic
control ordering. See [actual diagnosis](../diagnostic-verification.md).

## Whole-run continuation (current experiment)

`whole-run-gate` is a separate software-only executable. It copies complete prepared
runs, disables underline/strike/overline consistently, and uses four physical pixels
around outward-rounded ink bounds. It preserves coordinates with a painter
translation; image composition cancels canvas DPR and uses integer physical
positions without image filtering/scaling. The direct, translated tile and
full-canvas run crop paths preserve the same run order. The glyph path is only a
diagnostic. Full-canvas support outside each tile and canvas-edge ink fail the
gate; tile-border ink is reported separately. Backgrounds are not drawn.

Use fresh stage/build destinations with the earlier `stage.py` instructions,
then build `whole-run-gate retained-glyph-gate retained-glyph-diagnostic` with two
workers. Supply the **same files** for both SDKs (not independently resolved
family names); these runs use `/usr/share/fonts/TTF/DejaVuSans-Oblique.ttf` and
`DejaVuSansMono.ttf`, copied into `/tmp/qmarkdown-065-whole-fonts`.

```sh
python3 specs/065-padded-code-rendering/retained-prototype/run_whole.py \
  /tmp/qmarkdown-065-whole-612 /tmp/qmarkdown-065-whole-fonts \
  /tmp/qmarkdown-065-whole-evidence/qt-6.12.0 historical
python3 specs/065-padded-code-rendering/retained-prototype/run_whole.py \
  /tmp/qmarkdown-065-whole-612 /tmp/qmarkdown-065-whole-fonts \
  /tmp/qmarkdown-065-whole-evidence/qt-6.12.0 basic
# Only after both SDKs pass basic:
python3 specs/065-padded-code-rendering/retained-prototype/run_whole.py \
  /tmp/qmarkdown-065-whole-612 /tmp/qmarkdown-065-whole-fonts \
  /tmp/qmarkdown-065-whole-evidence/qt-6.12.0 extended
```

For Qt 6.8.0 use the recorded local CI image in the actual report, mount the
repository read-only at `/source` and `/tmp` at `/work`, and substitute `/work`
for `/tmp`. Configure `CMAKE_PREFIX_PATH=/opt/Qt/6.8.0/gcc_64` and a new build
`/work/qmarkdown-065-whole-680`. Qt 6.12.0 uses the host SDK. Each runner stage
executes forward and reversed controls in separate fresh processes and validates
all case/count/phase records. Basic has 256 cases per order; extended has 960:
single code, adjacent code, colour boundary, wrapping and tiny width, each at
three alignments and all 64 DPR/phase combinations. Historical has two cases
per order plus twice-repeated fresh legacy processes and the recovered 258-case
diagnostic forward/reverse. `DIAG_REVERSE=1` reverses its paint/control and
comparison order; default behavior and historical saved evidence stay intact.

Each stage completes both orders even after mismatches, then exits nonzero for
required whole-run failures. Do not advance to another stage after failure.
Compressed JSONL includes explicit loaded-font SHA-256, runtime/compiler identity,
run/glyph sequences and all metrics. Summary JSON retains commands, environments,
exit statuses and aggregates; stderr and representative PNGs accompany it.

## Retained run-texture scene-graph gate

`scenegraph-run-gate` is a separate executable; the earlier probes are unchanged.
It has three retained paths: translated whole-run tiles, full-canvas run crops
uploaded as tiles, and full-canvas images uploaded separately for each run. The
last path is reference-only and preserves run ordering. Exact capture equality
between these paths determines transport acceptance. Fresh current-phase
QPainter text is diagnostic and cannot qualify fractional text rendering.

The GUI thread prepares fonts, glyphs and immutable images. Only image/physical
geometry/DPR/generation data crosses the synchronization boundary. Each public
image node owns its render-thread-created texture. Filtering is linear, wrapping
is clamp-to-edge; atlas and mipmaps are disabled. Image DPR metadata is one and
node rectangles express physical bounds divided by actual window DPR. A public `grabWindow()` call drives synchronization/rendering; every capture
verifies a newer synchronization instead of depending on a presentation callback.
The viewport is fixed at 600×384 logical pixels, rounded outward to a common
grid for effective DPR and Qt platform scale; exact physical capture extent is asserted before comparison. Invalid capture extents are logged separately; fixtures never resize the window. Application counters describe preparation and image-node/texture creation, not
internal GPU uploads; diagnostic reference painting is excluded from candidate
preparation counters. Resource addresses are scoped to their process/window.

Build fresh standalone configurations and the target `scenegraph-run-gate`
alongside all existing probes, with two workers. Re-run all existing software
preparation stages on both exact SDKs before starting transport qualification.

```sh
python3 specs/065-padded-code-rendering/retained-prototype/stage.py /tmp/scene-stage
cmake -S specs/065-padded-code-rendering/retained-prototype -B /tmp/scene-build \
  -DQMARKDOWN_STAGE=/tmp/scene-stage -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build /tmp/scene-build --target scenegraph-run-gate whole-run-gate \
  retained-glyph-gate retained-glyph-diagnostic --parallel 2
python3 specs/065-padded-code-rendering/retained-prototype/run_scenegraph.py \
  /tmp/scene-build /tmp/qmarkdown-065-whole-fonts \
  /tmp/scene-evidence/qt-6.12.0/software static software --qt 6.12.0
```

Use the same runner for `opengl` and `vulkan`, replacing the API directory and
argument. Run **native GUI matrices serially** so windows do not compete for
Wayland frame callbacks. Native targets use Wayland and a threaded render loop. Fixed-state capture sets
`QSG_NO_VSYNC=1` and `QT_QPA_UPDATE_IDLE_TIME=0`; no animation/timing claim follows
(see [Qt render-loop documentation](https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph.html)).
Software uses the zero QPA idle interval as well;
software uses offscreen Qt Quick software (distinct from the QPainter gate).
Each stage calibrates natural window DPR in a separate process, derives scale
factors, and asserts observed API, exact SDK, hardware device and actual DPR in
every matrix process. Public OpenGL renderer/vendor/version and Vulkan physical
device properties identify hardware. A software GPU or missing identity fails.
The runner completes all eight DPR/order processes despite image mismatches and
validates case uniqueness, prepared phase, occurrences, run order and resource
counts. JSONL, stderr, commands/environments, aggregates and PNGs are retained.

For Qt 6.8.0, build with its exact SDK inside the recorded cached CI image. Its
minimal image lacks a working Wayland plugin dependency environment. The native
attempt therefore copies the **existing exact SDK** to a disposable `/tmp` tree
and runs the same built binary on the host driver/display stack, with:

```sh
LD_LIBRARY_PATH=/tmp/qmarkdown-065-native-qt680/lib:/tmp/qmarkdown-065-scene-680/staged/src/QMarkdown \
  python3 specs/065-padded-code-rendering/retained-prototype/run_scenegraph.py \
  /tmp/qmarkdown-065-scene-680 /tmp/qmarkdown-065-whole-fonts \
  /tmp/scene-evidence/qt-6.8.0/opengl static opengl --qt 6.8.0 \
  --qt-root /tmp/qmarkdown-065-native-qt680
```

`--qt-root` sets the relocated plugin path; retain the SDK/library identities.
No system package is installed or replaced. For Vulkan identity queries the
minimum SDK needs the already-installed public Vulkan headers: mount host
`/usr/include/vulkan` and `/usr/include/vk_video` read-only below a common include
root and pass `-DSCENEGRAPH_VULKAN_INCLUDE_DIR=/native-headers` at configure time.
This affects only the standalone target's includes, not library dependencies.
Keep fresh builds and raw evidence in an ignored durable directory such as
`build-ci/065-scenegraph`; `/tmp` contents can disappear across a restart.

`scroll` and `lifecycle` are conditional stages. Their runner requires passing,
complete summaries for every preceding SDK/backend under the same evidence root;
never run them after a required static failure. Static has 4,872 captures-as-cases
per SDK/backend (both backgrounds, both orders, four DPRs). Scroll has 936 cases
per DPR/order; lifecycle has 12. See the approved specification for the six scroll
fixtures, exact trace and clip rectangle. A passing ownership smoke would still
not qualify broader lifecycle, resource budgets or production rendering.
