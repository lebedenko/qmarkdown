# Actual-source artifact diagnosis — 2026-11-05

**Attribution stopped: final native replay stage stalled.** The performance
rejection remains independently binding. Production source is unchanged; no
renderer correction, timing pairs or renewed owner motion acceptance was attempted.

## Implementation and identities

The owner's supplied plan approved the appended requirements/design/tasks before
implementation. `scene.h` now shares fixture text, fonts, formatting and paragraph
construction between `motion.cpp` and `review.cpp`. Review builds directly against
each pinned `43d6bba842bb5b7ab23889e49d5f138c6f22a7fb` native-only-v2 snapshot.
Each viewer defaults to its labelled actual source. Its optional matching proxy
uses the linked source's sampling policy, including nearest for native candidate.
The original 24 paragraphs, 120-pixel spacing, clipping and initialization order
are retained. No spacing, alignment shift, pixel tolerance or forced text update
was used to obtain equality.

Four Release viewer executables were built: baseline/candidate on exact Qt 6.12.0
and 6.8.0. The motion target also built against both variants on both SDKs. CMake's
existing QTP0004 warning was nonfatal. Executables and linked libraries are retained
under `build-ci/065-artifact/binaries/{baseline,candidate,min-baseline,min-candidate}`;
set that directory in `LD_LIBRARY_PATH` when launching a retained bundle (and add
the minimum SDK lib directory for Qt 6.8). Original build paths and commands are
in the [README](README.md#actual-source-artifact-diagnosis--2026-11-05).

[Summary and identities](evidence/artifact-diagnosis/summary.json) retain per-run
executable/library hashes, source manifest including renderer hash, Qt/API/DPR,
harness hashes and full-log hashes. Full logs, captures, interaction rectangles,
transforms and source identities remain in `build-ci/065-artifact/`. Each attempt
uses a distinct output directory. Initial sandbox launch exited on signal 6;
compositor/GPU access required an unrestricted diagnostic run.

## Focused reproduction

Qt 6.12.0 Wayland OpenGL, observed DPR 1.25, pixelAligned true:

- Initial diagnostic binary: 2,560 deduplicated stationary positions, including
  240–360 logical pixels in quarter-physical-pixel requested increments and the
  remaining range in physical-pixel increments. All frozen/source/proxy comparisons
  were exact.
- Next diagnostic binary: 2,966 inspected positions and 286 captured live frames
  covering drag, release/deceleration, reversal and settling. All live/frozen
  replays and matching proxy comparisons were exact. Source paint counts stayed
  at their warmed values, with no layout signals or interaction/bounds changes.
- These scan binaries precede the final addition of targeted pose/phase mode and
  unconditional proxy capture saving. Their hashes are retained separately.
  **They do not qualify complete-trace replay on the final binaries.**

The supplied [owner screenshot](evidence/native-only/owner-proxy-artifacts.png)
was visually inspected against actual-source captures. Scroll 320 reproduces the
visible paragraph arrangement, including narrow wrapped text, strikeouts and the
unclipped tiny paragraph overlapping its successor. The candidate has visibly
harder sampled glyph edges; baseline is softer. This observation does not by
itself identify which stroke distortion the owner considers defective.

Representative authoritative captures at requested scroll 320:

| Linked source | Source | Matching proxy | Exact difference |
| --- | --- | --- | --- |
| Baseline | [source](evidence/artifact-diagnosis/pose-baseline-6.12-opengl-aligned-source.png) | [proxy](evidence/artifact-diagnosis/pose-baseline-6.12-opengl-aligned-proxy.png) | [zero diff](evidence/artifact-diagnosis/pose-baseline-6.12-opengl-aligned-diff.png) |
| Candidate | [source](evidence/artifact-diagnosis/pose-candidate-6.12-opengl-aligned-source.png) | [proxy](evidence/artifact-diagnosis/pose-candidate-6.12-opengl-aligned-proxy.png) | [zero diff](evidence/artifact-diagnosis/pose-candidate-6.12-opengl-aligned-diff.png) |

Metadata records intersecting logical bounds at paragraph indices 4, 7, 12, 15
and 20. Clip state is recorded separately; an intersection with clipping enabled
is not proof of visible overlap. In particular unclipped `tiny-right` extends
146 logical pixels in a scene with 120-pixel spacing. Both actual variants share
this composition. No evidence isolates that overlap as the cause of the reported
candidate-only artifact, so no composition control or repair was introduced.

## Final-binary native pose replay

The final unchanged binaries replayed scroll 320, all sixteen requested Cartesian
quarter-physical phases, origin and bottom viewport. Actual positions are deduplicated:
6 distinct poses with alignment enabled; 18 with alignment disabled.

| Qt/backend | Baseline aligned/unaligned | Candidate aligned/unaligned |
| --- | --- | --- |
| 6.12.0 OpenGL | pass / pass | pass / pass |
| 6.12.0 Vulkan | pass / pass | pass / pass |
| 6.8.0 OpenGL | pass / pass | pass / pass |
| 6.8.0 Vulkan | pass / pass | pass / **stalled before ready** |

All fifteen completed runs reported expected Qt/native API/DPR and exact complete
viewport frozen replay/source-proxy equality, zero warmed raster paints/layout
signals, unchanged source surface identity, bounds and full interaction rectangles.
Labels are excluded from comparison. The sixteenth process produced no ready or
pose record and remained stalled for multiple minutes. It was terminated explicitly;
its empty logs and termination result are retained. The matrix stops here.

## Classification and unreached checks

**Replay/environment failure; visual attribution unresolved.** Completed captures
provide no evidence of a proxy-only mismatch. Actual candidate/source proxy equality
establishes fidelity at those poses, not visual acceptability. Overlap exists in
the original composition but is shared with baseline. The incomplete replay stage
prevents reporting all eight target combinations as passed or assigning the owner's
reported distortion conclusively to candidate rendering or composition.

No diagnostic repair is justified by the isolated evidence. Complete captured-pose
replay on final binaries across all eight combinations, a final-binary full-range
scan, and complete owner motion assessment remain unreached. Do not rerun performance
pairs or integrate a production correction. No exact failing pose was requested:
the stalled stage must be resolved before concluding that reproduction is missing.

Repository lightweight validation passed, including all 87 Qt-independent tests;
Python syntax and whitespace checks passed. Final documentation checks are recorded
with this delivery. Shipping runtime tests were not repeated because all changes
are confined to standalone feature-065 diagnostics and specifications.
