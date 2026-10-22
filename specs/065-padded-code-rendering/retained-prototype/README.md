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
