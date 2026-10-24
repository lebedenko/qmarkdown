# Shaping and decorated-content gate: actual verification

Owner approval: implementation of the supplied plan on 2026-10-24, recorded in
requirements, design and tasks before implementation. This revision is part of
Feature 065. PR [#51](https://github.com/lebedenko/qmarkdown/pull/51) merged at
2026-10-24 12:05:38 UTC; PR [#52](https://github.com/lebedenko/qmarkdown/pull/52)
merged at 17:43:09 UTC. Their historical delivery instructions and evidence remain
unchanged. Base: `96c530660aab25654cd611c70af68affdb58761d` (current main).

## Planned gates

Fresh exact Qt 6.8.0 and 6.12.0 builds and existing whole-run software controls
precede the decorated corpus CPU matrix. Both CPU targets must pass before static
transport on software and native OpenGL/Vulkan on both targets. All static targets
must pass before every-fixture motion/clip traces, followed by three window
recreations retaining CPU images. Finish each started matrix despite mismatches.
A required failure stops progression. Missing required environments are incomplete.

The fixed 45-fixture corpus has 720 phase cases per DPR/order, at DPR 1, 1.25,
1.5 and 2, with independent X/Y phases 0, .25, .5 and .75. CPU comparisons require
exact premultiplied equality between shipping paint, independently painted
full-canvas components, integer-translated tiles and full-component crops.

## Actual results

Both fresh exact-SDK builds succeeded. The existing historical, basic and
extended whole-run controls completed in forward/reverse processes on both SDKs;
all accepting whole-run cases passed. Historical single-glyph diagnostics retain
their earlier expected failures.

The final decorated CPU stage completed all **11,520 cases** (5,760 per SDK).
Both SDKs produced identical aggregate results, with exact forward/reverse case
metadata and metric equality:

| DPR | Cases per SDK, both orders | Failing cases per SDK | Shipping/tile changed pixels per SDK | Maximum channel delta |
| --- | ---: | ---: | ---: | ---: |
| 1 | 1,440 | 0 | 0 | 0 |
| 1.25 | 1,440 | 170 | 18,768 | 255 |
| 1.5 | 1,440 | 330 | 23,590 | 255 |
| 2 | 1,440 | 0 | 0 | 0 |

Shipping paint versus independent full-canvas components and component crops has
**zero changed pixels in every final case**. Translated tiles fail 500 cases per
SDK, 1,000 total. Tile/crop differences equal shipping/tile differences. The
failure is not clipped support: cropped ink and border ink are zero throughout.
All 45 fixture coverage records, explicit font checks, glyph/source metadata,
component counts/order and three-path emission counts pass. The independent
collection audit confirms exact forward/reverse equality.

**The gate fails at CPU preparation.** Static software/native OpenGL/Vulkan
transport, fractional motion/clip traces and window recreation were not started.
The corresponding harness paths are implemented but unqualified. No production
design review, renderer integration, perceived-quality or performance claim is
permitted by these results. Thresholds remain exact and unchanged.

## Preserved development attempts

Attempt 1 explicitly rejected an inherited Mono Oblique font on Qt 6.12.0 as
unexpected fallback, leaving its matrix incomplete. Qt 6.8.0 completed its matrix
and retained fractional-DPR failures. The fixture was corrected to explicitly
select the supplied Mono Regular style and weight. Attempt 2 on Qt 6.12.0
completed with no coverage failures but found full-component mismatches at DPR
1.5 when paragraph origins were folded into rectangle/line coordinates. The
final harness instead retains original geometry and the shipping painter
transform. The earlier metrics, captures and source snapshots remain under
`attempt-1` and `attempt-2`; they do not qualify the final implementation. These
were fixture/coordinate corrections within the approved scope, with no threshold
relaxation or shipping change. Every launched DPR/order process was completed.

## Evidence and reproduction

[Aggregate metrics](evidence/decorated/summary.json),
[independent audit](evidence/decorated/audit.json),
[source/font/executable identities](evidence/decorated/identities.json),
[host runtime identities](evidence/decorated/host-runtime-identity.json),
[minimum runtime identities](evidence/decorated/minimum-runtime-identity.json),
[initial commands](evidence/decorated/commands.json),
[final commands](evidence/decorated/final-commands.json), and
[SHA-256 manifest](evidence/decorated/sha256.json) are durable repository evidence.
Raw JSON Lines retain every case/component/glyph, source index, font-file ordinal,
colour, origin, recorded line, background rectangle/radius and physical bounds.
Representative forward-order captures are retained for every fixture/DPR,
including the first failure; reverse controls retain complete metrics without
duplicate PNGs. The raw transparent PNGs are premultiplied comparisons, not a
visual-quality assessment. The ligatures phase (.75, 0) DPR-1.25 difference was
visually inspected and shows glyph-edge differences inside the supported bounds.

Use the [standalone instructions](retained-prototype/README.md) and the retained
launchers for exact commands. Source staging used:

```sh
python3 specs/065-padded-code-rendering/retained-prototype/stage.py build-ci/065-decorated/stage
cmake -S specs/065-padded-code-rendering/retained-prototype -B build-ci/065-decorated/host -G Ninja -DQMARKDOWN_STAGE="$PWD/build-ci/065-decorated/stage" -DCMAKE_BUILD_TYPE=Release
cmake --build build-ci/065-decorated/host --target decorated-content-gate whole-run-gate retained-glyph-gate retained-glyph-diagnostic -j2
```

Qt 6.12.0 used the host SDK and GCC 16.2.1. Qt 6.8.0 used the retained exact-SDK
Docker image and GCC 13.3.0, with a fresh build directory. Only the exported source
tree received `prepared-runs.patch`. The working shipping sources, public API,
dependencies and version remain unchanged. Final harness hashes identify the
uncommitted standalone inputs used against the recorded main source base; the
continuation PR commit identifies the complete delivered payload.

## Repository verification and delivery

`task check` passed documentation links, changed evidence/configuration/Python
syntax and all 87 Qt-independent Python tests. `git diff --check` passed. The
evidence SHA-256 manifest and unchanged verified harness identities were checked.
Legacy prerequisite stdout retained under a misleading JSON Lines extension was
renamed to `.txt.gz` without changing its bytes, so syntax validation remains
strict. Hosted final-head outcomes are recorded with the continuation PR. Hosted Light success cannot qualify this experiment.
The PR remains unmerged with auto-merge disabled; no evidence-only follow-up
commit is created.
