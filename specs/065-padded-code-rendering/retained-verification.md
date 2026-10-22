# Retained glyph-texture experiment — actual verification

2026-10-22 decision: **no-go for this candidate; expansion stopped at preparation
image correctness**. Production integration remains excluded. This does not
reject every possible retained-texture design or establish visible degradation.
The [selection-emission failure](verification.md) remains unchanged.

## Identities and commands

Source exported by `stage.py`: `4670000bbbe89835019dd5e0b43abaa649942620`
(the merged PR 47 tree). Only `prepared-runs.patch` modifies its staged renderer
header. Current prototype sources, patch, commands and captures are in this PR;
shipping sources, APIs, versions and dependencies are unchanged.

The exact reproduction commands are in the [prototype README](retained-prototype/README.md).
Host: GCC 16.2.1, Qt 6.12.0, offscreen QPainter software raster engine.
Minimum: local container
`qmarkdown-ci:6.8.0-amd64-55e391d786afbc0381181ed237876dbc3f4d18be369418fd74fb41aa5c5cd1f2`,
SDK `/opt/Qt/6.8.0/gcc_64`. Separate fresh build directories; math disabled in
staged build, default shared linkage. Host configure initially missed the
parent-scope Qt imported target; adding explicit `find_package` corrected this
before the recorded successful builds.

The container mounted the repository read-only at `/source`, `/tmp` at `/work`,
ran as UID/GID 1000, with `QT_QPA_PLATFORM=offscreen`,
`QT_QUICK_BACKEND=software`, `LANG=C.UTF-8`. Configure:

```sh
cmake -S /source/specs/065-padded-code-rendering/retained-prototype \
  -B /work/qmarkdown-065-retained-680 \
  -DQMARKDOWN_STAGE=/work/qmarkdown-065-retained-stage \
  -DCMAKE_PREFIX_PATH=/opt/Qt/6.8.0/gcc_64
cmake --build /work/qmarkdown-065-retained-680 --target retained-glyph-gate --parallel 4
```

Run the same two DPR commands from that build directory. Both SDKs configured
and built successfully. DPR 1 exited 0; DPR 1.25 exited 1 on both. Final controls
were added after the initial failure; these results use the final harness.

## Measurements

Fixture: italic DejaVu Sans request, 40 point, plain `fX`, layout width 400,
original prepared positions plus capture origin `(24.25,24.25)`. Font resolution
and rasterization differ across environments; comparisons are within each SDK,
not between SDK captures. Two occurrences, two emissions and two unique images
in every run. Plain `fX` is the first prerequisite; padding cases were not reached.

| Qt | Image DPR | Changed pixels | Lost / extra support | Max alpha delta /255 | Direct control changes | Tile border ink | Image bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 6.12.0 | 1 | 0 | 0 / 0 | 0 | 0 | 0 | 13360 |
| 6.12.0 | 1.25 | 379 | 1 / 0 | 8 | 0 | 0 | 19740 |
| 6.8.0 | 1 | 0 | 0 / 0 | 0 | 0 | 0 | 15172 |
| 6.8.0 | 1.25 | 306 | 1 / 1 | 5 | 3 | 0 | 22812 |

Raw final stdout is under [evidence](evidence/retained/qt-6.12.0-results.txt)
and [minimum evidence](evidence/retained/qt-6.8.0-results.txt).

At DPR 1.25, direct emission agrees exactly on 6.12.0, but tile rasterization and
composition do not. On 6.8.0, splitting the run also changes three pixels. The
candidate fails the conservative exact same-phase image gate on both SDKs.
No border ink was detected, so these data do **not** demonstrate truncation at
tile edges. Differences are small antialiasing changes, including alpha support
changes; their root cause and perceptual significance are unresolved. No
shimmer resolution or visible quality regression is claimed.

[Qt 6.12 comparison](evidence/retained/qt-6.12.0-comparison.png) and
[Qt 6.8 comparison](evidence/retained/qt-6.8.0-comparison.png) show reference,
candidate on white, then alpha difference amplified 8x on black. The 6.12
comparison was visually inspected; both glyphs are present, with no gross
truncation. Transparent source captures are retained alongside the comparisons.

## Stopped and unavailable work

The approved stop rule prevents expansion after this image gate failure.
Ligatures, combining marks, Arabic/bidi, fallback, padded spans, links,
decorations, wrapping, tiny widths, alignment, fractional scrolling and filtering
captures were not qualified. No textures/image nodes, rounded backgrounds,
overlays or render-thread ownership were implemented. There is no runtime-
confirmed Quick software/OpenGL/Vulkan or screen DPR qualification, no replay
zero-work or resource-balance evidence, and no lifecycle testing.

Fresh 1.17.1/prototype fixed traces, measured refresh intervals, p95/max timing,
preparation timing, node/texture counts and bounded process memory were not run:
the candidate failed before those gates. The image-byte values above measure
CPU glyph image payload only. They are not texture or total-memory measurements.
No speedup or production readiness claim is made.

Documentation/whitespace checks and hosted delivery results are recorded in the
PR. Required hosted packaging checks qualify the unchanged shipping tree; they
do not qualify this standalone experiment. No merge, auto-merge, release or
installed-package replacement is authorized.
