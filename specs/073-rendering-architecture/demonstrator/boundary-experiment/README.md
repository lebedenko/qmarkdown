# Explicit-boundary SkParagraph feasibility experiment

**Automated feasibility gates passed for the tested isolated slice.** Owner
approval of B1–B8, design and tasks was received on 2026-11-05. This continuation
preserves the diagnostic patch in commit `d3b1f05` and all earlier failures.
It does not select a production backend, qualify performance or change Qt controls.
Owner visual acceptance was recorded on 2026-11-06 for inspected cases only;
see the [measurement continuation](../performance-experiment/README.md).

Each code-span edge bounds upstream shaping context, including adjacent spans
with identical styling. The adjacent lam–alef case produces separate glyphs 1141
and 1117; the whole-span case retains ligature 3254. Paragraph-wide bidi, upstream
script/font fallback and Unicode break flags remain in use. Padding does not
insert text or placeholders. Real inline objects retain complete source intervals.

The patch bypasses paragraph shaping-cache reads/writes while metadata is present.
Metadata changes discard derived Unicode/bidi state and shaping; width-only changes
reuse shaped runs. A finite candidate scan evaluates visual-fragment costs and
bounds the existing wrapper by source endpoint. It retains upstream stretches,
trimming, metrics and emergency handling; original grapheme boundaries constrain
emergency endpoints. Each fragment pays both padding edges. Formatting and fitting
share the visual-run/identity calculation. Painting, range rectangles, hit testing
and objects traverse those adjusted positions. Integral baseline/height reservation
prevents negative rounding deltas. The containment tolerance remains 0.05 pixels.

Trailing ghost whitespace keeps upstream selectable, unpainted behavior. There is
no painted fragment for an entirely trimmed whitespace range. Near-edge snapping
is retained and diagnosed separately from interior caret checks. Interior probes
use `min(0.1, cellWidth / 4)` logical pixels, outside upstream's 0.01-pixel snap
zone. Zero-width newline boxes have no independently hittable interior; source
mapping still retains the newline. Explicit fallback fonts are unchanged; existing
missing glyphs are not claimed to be corrected by this layout experiment.

## Reproduce

Use the original pinned checkout and prerequisites from the
[demonstrator](../README.md). The control must be clean at
`00227028b0c3cb3052432a456af2027a49e598b2`. Stage and output paths must be new.
No download, shipping dependency change or overwrite is performed.

```sh
python3 specs/073-rendering-architecture/demonstrator/boundary-experiment/build.py /tmp/qmarkdown-073-skia /tmp/073-boundary-reproduction-skia build-ci/073-boundary-reproduction
python3 specs/073-rendering-architecture/demonstrator/boundary-experiment/verify.py build-ci/073-boundary-reproduction build-ci/073-boundary-reproduction-geometry
python3 specs/073-rendering-architecture/demonstrator/boundary-experiment/audit.py build-ci/073-boundary-reproduction-geometry build-ci/073-boundary-reproduction-geometry/extent-audit.json
python3 specs/073-rendering-architecture/demonstrator/boundary-experiment/native_verify.py build-ci/073-boundary-reproduction build-ci/073-boundary-reproduction-geometry build-ci/073-boundary-reproduction-native
```

The build preserves internal relative symlinks, rejects external/absolute symlinks,
checks the staged tree before patching, checks the patch hash/application and
rejects changed files outside the five-file pin. Builds use two workers. Build
records include dependencies, tools/configuration, source hashes, library/binary
hashes and unchanged control identities. CPU and native adapters share `layout.h`.
The CPU executable retains the historical name `fragment-preflight`; its row
status describes the boundary check, while `verify.py` evaluates layout gates.

The geometry runner checks 751 existing fixture widths / 775 blocks, 14 boundary
regressions, nine invalid-range/input cases and separate control/zero-padding
baselines. It refines 41 observed transition brackets to 1/64 logical pixel and
checks 147 block probes below/at/above them. The extent audit additionally checks
14,272 code cells and 2,901 visual fragments. Counts describe the retained run.
The original upstream width-rounding behavior remains enabled.

The native runner requires passing, unchanged CPU inputs and runs serially with
20/10/10-second startup/operation/shutdown deadlines. Fifteen fixtures on two APIs,
three normal recreations and one motion-replay process each produced 120 passes.
The historical runner called the drawable/window ratio of 1 “DPR 1”; no other
ratios were observed or emulated by zoom. The subsequent
[scale diagnosis](../../evidence/performance-experiment/README.md) establishes that
this SDL coordinate convention coexists with a 1.25 desktop and buffer/surface
mapping. Historical raw records retain their original labels.

## Interactive review

The actual patched native viewer is built as `patched/skia-compare`:

```sh
python3 specs/073-rendering-architecture/demonstrator/boundary-experiment/launch.py build-ci/073-boundary-preserved --backend opengl --fixture adjacent
python3 specs/073-rendering-architecture/demonstrator/boundary-experiment/launch.py build-ci/073-boundary-preserved --backend vulkan --fixture objects
python3 specs/073-rendering-architecture/demonstrator/boundary-experiment/launch.py build-ci/073-boundary-preserved --manifest specs/073-rendering-architecture/demonstrator/boundary-experiment/minimal.json --fixture adjacent-lam-alef
```

Space/S/F control movement; R replays, 0 resets, +/- zoom, drag selects and Ctrl+C
copies. See the original viewer controls in [demonstrator instructions](../README.md).
Matched OpenGL/Vulkan stationary captures and all selected/resize/replay frames
are retained in the [evidence index](../../evidence/boundary-experiment/README.md).
Automated replay and these capture-integrity inspections are not owner acceptance.

## Maintenance and limits

The patch adds 198 lines and removes 12 across five private files:
`ParagraphImpl.h/.cpp`, `OneLineShaper.cpp`, `TextWrapper.cpp`, and `TextLine.cpp`.
It changes private object layout and relies on private bidi ordering, clusters,
line metrics, wrapper stretches and visual-run traversal. The adapter must use
matching library ABI defines. Each Skia pin change requires rebase/review and
rerunning boundary, cache, equivalence, geometry, selection and native gates.

Candidate scans can be quadratic; costs and performance are unqualified. Cache
optimization, production interfaces/dependencies, Qt integration, general tables,
cross-paragraph selection and backend selection remain outside scope. The Qt 6.8
Vulkan shutdown failure remains independent. Merge/publication are not authorized.
