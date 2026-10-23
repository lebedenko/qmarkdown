# Retained glyph preparation gate

This standalone diagnostic is excluded from shipping CMake. It deliberately stops
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
