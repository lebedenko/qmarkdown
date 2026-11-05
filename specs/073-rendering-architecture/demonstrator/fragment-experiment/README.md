# Isolated fragment-edge feasibility experiment

**Blocked at shaped-cluster preflight.** This is a checked upstream diagnostic
patch and an independent CPU adapter, not an implemented fragment-padding engine.
Approval for F1–F7 was received on 2026-11-05. The approved design explicitly stops
when a span edge inside a shaped cluster cannot be accommodated without changing
shaping. No shipping source or original Qt/Skia control was changed.

The minimal input is Arabic lam–alef (`لا`) in the existing DejaVu Sans Mono font,
22 logical pixels, width 190, with adjacent UTF-16 code spans `[0,1)` and `[1,2)`.
Both are valid grapheme intervals. Upstream produces one glyph (3254), one cluster
covering UTF-8 `[0,4)`, and width 13.2451171875. Each span needs its own six-pixel
horizontal padding edge at byte 2. Translating an intact glyph cannot insert that
interior gap. Shaping the characters separately produces glyphs 1141 and 1117;
that changes shaping and is shown only as a diagnostic counterfactual.

The private upstream patch accepts identity-bearing source-span metadata and
reports edges inside shaped clusters. It neither mutates glyphs nor claims that
an empty issue list means padding support. The adapter validates UTF-16 intervals
using ICU grapheme boundaries and maps them to UTF-8 once, preserving complete
source ranges for real objects. It inserts no padding text or placeholders.
Disabled mode records shaped glyphs/fonts/positions, line ranges/metrics, source
maps, range rectangles, caret hit results and object bounds for exact comparison.

## Reproduction

Prerequisite: the original pinned source and built libraries produced by
[the existing build instructions](../README.md), plus the same installed fonts,
CMake, Ninja, C++ compiler, nlohmann JSON, SDL2, ICU, HarfBuzz, FreeType, fontconfig,
libpng and zlib development packages. No network download is performed here.
Use fresh stage/output paths; the script refuses to overwrite previous artifacts.

```sh
python3 specs/073-rendering-architecture/demonstrator/fragment-experiment/build.py /tmp/qmarkdown-073-skia /tmp/073-fragment-reproduction-skia build-ci/073-fragment-reproduction
python3 specs/073-rendering-architecture/demonstrator/fragment-experiment/verify.py build-ci/073-fragment-reproduction build-ci/073-fragment-reproduction-evidence
```

The build copies files (no hardlinks), checks the original revision and clean
tracked tree, checks the pinned patch hash, runs `git apply --check`, records
patched source hashes, regenerates GN and builds with two workers. Original
libraries and build configuration are hashed before/after. Each adapter build
uses the exact library ABI defines. Build records include dependencies, tools,
configuration, system versions and output identities. The verification also
checks that the existing original viewer and selection executables are unchanged.
The original control binaries must remain at `build-ci/073-skia-demo/`.

Direct minimal case (expected exit **1**, explicit capability blocker):

```sh
build-ci/073-fragment-reproduction/patched/fragment-preflight specs/073-rendering-architecture/demonstrator/fragment-experiment/minimal.json --enabled
```

`--disabled` exits 0 for valid input; malformed intervals exit 2. The verification
script exits 0 only when the expected blocker, disabled equivalence and rejection
regressions are reproduced. Its summary still says `outcome: blocked`.
The 751-case disabled scan is not a padded geometry pass. Fractional wrap probes,
finite padded candidate fit, adjusted authoritative positions, native GPU gates
and a patched native viewer were not reached after the preflight blocker.

## Object diagnosis and review

`historical-objects` compiles the unchanged historical adapter into a CPU-only
diagnostic. Its `objects@679` trace shows raw line height 36.1884765625 rounded to
36, `InternalLineMetrics::delta()` −0.1884765625, baseline 28 (already integral),
and object path top −0.19. This is line-height rounding, not fractional baseline
rounding. At DPR 1 the original non-antialiased round rectangle paints rows 0–35,
but logical bounds still fail the unchanged 0.05 tolerance. Removing the padding
placeholders in disabled mode avoids this particular extra descent and overhang;
it does not prove an authoritative padded vertical-reservation fix.

Launch the offline image review with magnification and counterfactual toggles:

```sh
python3 specs/073-rendering-architecture/demonstrator/fragment-experiment/review.py --open
```

The images are unchanged unpadded shaping versus separately shaped characters,
not before/after images of a successful fix. The original interactive controls
remain available through [the demonstrator launcher](../README.md).
No native GL/Vulkan process was run for this experiment, because geometry did not
pass. Observed Skia DPR coverage remains the historical DPR 1; fractional DPRs
remain unavailable. Qt 6.8 Vulkan shutdown and owner visual acceptance stay open.

## Required next decision and maintenance cost

The approved constraints cannot all hold for arbitrary grapheme-aligned adjacent
spans crossing one glyph. A future design must explicitly choose whether span
edges are shaping boundaries (permitting changed ligatures/Arabic joining),
restrict supported edges to existing shaped-cluster boundaries, or introduce
sub-glyph clipping/placement with new geometry and visual semantics. Upstream
already proportionally clips multi-grapheme ligatures for ranges; independently
moving those clipped pieces would split the glyph and requires an explicit change
to the approved intact-cluster contract. None of these choices is adopted here.
This finding does not prove that all fragment-aware SkParagraph extensions are
impossible, and does not require replacing the entire shaping engine.

The diagnostic patch adds 40 lines in two private upstream files. It has no ABI
stability guarantee; a pin update requires rechecking cluster/source semantics,
reapplying/reviewing the patch and rerunning the equivalence/rejection probes.
This is a lower bound on maintenance, not a cost estimate for the unimplemented
line breaker/formatter. The copied source/build stage uses additional disk space;
there is no shipping dependency or performance qualification.

See [actual verification](../../verification.md) and
[retained evidence](../../evidence/fragment-experiment/summary.json).
