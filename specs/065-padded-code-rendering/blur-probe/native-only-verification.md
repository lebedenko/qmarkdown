# Native-only smoothing diagnostic qualification

The owner approved the supplied plan on 2026-11-04 (N1–N5 in feature 065).
**Qualification is rejected at the Qt 6.8.0 software performance gate.** No
production correction, merge or publication is recommended or performed.
Owner reported candidate-proxy artifacts on 2026-11-05; motion quality is not
accepted. This is independent of the failed performance gate.

## Source and frozen measurement identities

Both isolated snapshots export `43d6bba842bb5b7ab23889e49d5f138c6f22a7fb`.
The candidate disables retained smoothing only on observed OpenGL/Vulkan;
software, unknown APIs, missing windows, atom and unsupported-DPR fallbacks keep
baseline behavior. No shipping file, public API, dependency or package changed.
The previous unconditional candidate was rejected as input by the policy/commit
identity guards. Both snapshots received identical diagnostic instrumentation.

The source-tree comparison found only the expected renderer and manifest
variants. Renderer hashes match their manifests; private headers are identical.
Renderer SHA-256:

- Baseline: `b52bcd44bbb6c9e41fdbf5eff26854b0cf6dcdfc39fb314361428160f56a7c98`
- Candidate: `708c64c09a68439415f0172979b738c6c53c86bda95fcb72e3e822d1c67d32ca`

[Machine-readable evidence](evidence/native-only/summary.json) retains actual Qt,
backend/DPR, executable/library/renderer/harness hashes, font files and hashes,
GPU/driver logs, paired measurements and trace hashes. Native GPU was NVIDIA
GeForce GTX 1660 SUPER; kernel driver was **615.78.08**. Raw Vulkan driver fields
are also retained as Qt reported them. Font files were checked against the actual
glyph-font `head` tables. No renderer fallback qualified.

The ordered run used unchanged binaries throughout, under
`build-ci/065-native-only/final/`. Its four executables, matching libraries and
harness sources were preserved in that directory's `frozen-binaries/` before
post-gate fixes. The exact [measured C++ harness](evidence/native-only/measured-motion.cpp)
is preserved separately from the final development harness. Original source
snapshots remain under `/tmp/blur-native-only-final-source/`.

## Calibration and correctness actually completed

All six focused configurations passed for both variants: exact Qt 6.8.0/6.12.0,
software DPR 1.25/1.5 and native Wayland Vulkan at observed DPR 1.25. Code,
combined-format and mixed-direction fixtures passed raw-raster equality and all
16 same-coordinate phases, with zero additional source paints/layout rebuilds.
There were 36 focused first-paint records and 576 focused framebuffer comparisons.

The first real paint records format, physical dimensions, image DPR, transform,
clipping state and render hints. Software uses a QPixmap paint device; native
uses QImage. The independent image copies those settings. The retained painted
control maps its physical pixels to the source's logical bounds and matches
geometry, antialiasing and sampling. Opacity switching compares the same viewport
coordinates. No tolerance, alignment shift or warmed source repaint was used.
Translated/direct texture-node differences are diagnostic observations only.

| Environment | Qt 6.8.0 baseline/candidate | Qt 6.12.0 baseline/candidate |
| --- | --- | --- |
| Software DPR 1 | pass/pass | pass/pass |
| Software DPR 1.25 | pass/pass | pass/pass |
| Software DPR 1.5 | pass/pass | pass/pass |
| Software DPR 2 | pass/pass | pass/pass |
| Wayland OpenGL DPR 1.25 | pass/pass | pass/pass |
| Wayland Vulkan DPR 1.25 | pass/pass | pass/pass |

“Pass” here means the **frozen harness's recorded correctness checks**, not full
qualification of the candidate or of subsequent harness revisions. Every
configuration covered 12 fixtures, the 16 Cartesian quarter-physical-pixel phases
and exact origin return. Live Flickable drag, deceleration, in-flight reversal
and settling ran with pixelAligned off/on. All 31,680 captured source poses
replayed exactly in a static container at the captured actual content-item
position. All 21,120 applicable retained-control comparisons matched those
replayed source images exactly. Elapsed capture timestamps, velocities, actual
positions, source transforms and clip geometry remain in the traces.

Warmed source raster paints/layout rebuilds stayed zero; surface identities,
bounds and cursor/link/code/range rectangles stayed unchanged. All eight raster
fixtures passed applicable post-movement selection, Find, focused-link, content
and style invalidation (1,872 mutation records). Focused-link checks are skipped
where no link exists. The four scene-graph-only fixtures have movement/replay and
geometry coverage; the mutation loop is specific to retained raster fixtures.
Baseline/candidate geometry/font/allocation comparisons passed. Software
stationary PNGs were exactly equal between variants at every required DPR on
both SDKs, confirming output preservation.

## Performance rejection and cases not reached

Five fresh alternating Release pairs ran on **Qt 6.8.0 software DPR 1.25**.
The committed sparse/dense workload and preparation/frame timing boundaries were
preserved. Capture instrumentation, PNG writing and control comparison were off.
Every workload had zero warmed paints/layout rebuilds, 12 stable surfaces,
1,303,200 retained image bytes, and matching font files in both variants. This
accounts for retained image storage, not total GPU memory.

The sparse p95 candidate/baseline ratios were:
`1.0129, 1.6625, 1.5422, 1.8757, 1.0244`.
Three of five exceed **1.10**, triggering the approved rejection rule.

| Workload | Median paired preparation ratio | Median paired frame-median ratio | Median paired p95 ratio |
| --- | ---: | ---: | ---: |
| Sparse | 1.0862 | 1.0354 | **1.5422** |
| Dense | 1.0259 | 1.0203 | 0.8287 |

These are medians of **paired ratios**, not ratios of medians. All timings are
capture-trace costs, including event processing/framebuffer readback, **not
measured display frame rate**. The failure's cause is not isolated by this run;
the acceptance gate nevertheless failed. No tolerance or substitute threshold
was applied, and no repeat-until-pass pairs were run.

The ordered run stopped normally at that first failing performance stage.
Qt 6.8.0 OpenGL/Vulkan and Qt 6.12.0 software/OpenGL/Vulkan performance pairs
were **not reached**. Historical unconditional-candidate pairs were never reused.
No incomplete comparison was reported as a geometry mismatch.

## Evidence gap and post-gate diagnostic fixes

An auxiliary raw-image filename used the surface address. One address was reused
in Qt 6.8.0 software DPR 2 candidate stationary mode: `tiny-right`'s raw PNG pair
was overwritten by `wrapped-adjacent-linked-clipped`. All framebuffer captures
and both recorded raw-equality results remain; the earlier raw PNG pair is
**missing evidence**, not a renderer or replay failure. There are 420 raw-paint
records and 419 unique saved raw pairs. Focused calibration and native raw pairs
were unaffected. This gap is retained in the summary rather than reconstructed.

Audit also found that the measured performance loop lacked the plan's explicit
per-frame smoothing assertion. Final `motion.cpp` now asserts the policy before
warming and after every timed frame, outside the existing timing boundaries.
Raw output is now isolated by fixture, preventing the observed filename reuse.
The proposed idle-wait optimization was **not implemented**; replay timing stays
unchanged. Neither fix changes the candidate's smoothing policy or shipping code.

Final Release builds passed for both variants on both exact SDKs. Twelve final
single-run policy smokes (two variants × three backends × two SDKs) passed the
new assertion before warming and across all 120 movement frames, with zero
warmed paints/layout rebuilds. Focused Qt 6.8.0 software DPR 2 calibration passed
for both variants using the isolated capture directories. See
[assertion smokes](evidence/native-only/assertion-smoke.json) and
[artifact calibration](evidence/native-only/artifact-smoke.json).

These are **post-gate diagnostic smokes**, not new paired qualification evidence.
The final development harness has not received another full matrix or paired
performance qualification. The frozen attempt's rejection stands; later fixes
do not turn it into a pass. Earlier exploratory runs under
`build-ci/065-native-only/` remain separate: the initial unsupported-QPixmap
probe, incorrect image-to-logical-size control, sandbox-denied Wayland attempt,
and intermediate smokes do not qualify this revision.

## Native review and delivery checks

The separate `blur-review` executable built on both exact SDKs. It requires all
four native correctness stages to pass and checks every displayed proxy's raw
pixels against both frozen source variants. Four windows were opened and emitted
`reviewReady`: Qt 6.8.0/6.12.0 × native OpenGL/Vulkan, each observed DPR 1.25.
[Review identities](evidence/native-only/review.json) retain their actual backend,
Qt, DPR, source and executable identity. Each window labels **performance
rejected**, baseline source/candidate validated image proxy, and pixelAligned.
Both controls sit outside the document; switching preserves the scroll position.

The owner was asked to assess resting sharpness, slow-scroll shimmer, stepping,
reversal and settling in both alignment modes across the four windows. Review
remains pending; silence is not acceptance. Even favorable motion review cannot
replace the failed performance gate. No later production revision is recommended.

Repository lightweight validation (including 87 Qt-independent tests) and final
whitespace/document checks are recorded after the final diagnostic fixes below.

Changed files are feature 065's requirements/design/tasks; blur-probe README and
CMake configuration; staging, calibration, ordered qualification, matrix and
summary runners; motion/review C++; and the linked native-only evidence/record.
Shipping runtime tests and package checks were not repeated for this standalone
experiment.

Final `python3 scripts/check.py` passed after the diagnostic fixes, including
**87/87 Qt-independent tests**, evidence/configuration/Python syntax and document
links. `git diff --check` passed. The final source changes stay entirely under
feature 065. Review is still pending; no approval is inferred from the windows
being opened.


## Owner artifact report — 2026-11-05

The owner reports scroll-position-dependent artifacts in **CANDIDATE image proxy**
on both Qt 6.8.0/6.12.0, both OpenGL/Vulkan, and pixelAligned off/on. The
**BASELINE source** looks correct in every reported combination. The supplied
[annotated screenshot](evidence/native-only/owner-proxy-artifacts.png) shows Qt
6.12.0 OpenGL DPR 1.25 with pixelAligned true. The other combinations are owner
observations; separate captures were not supplied.

This supersedes the pending/no-feedback status above: candidate-proxy visual
review has reported artifacts and is **not accepted**. The five requested motion
quality criteria have not received a complete explicit assessment.

The earlier “validated image proxy” label overstated validation. The viewer
compared initial raw image pixels with both variants; it did not compare its
complete 24-paragraph scrolling composition against the actual candidate at each
observed pose. Frozen single-fixture replay results do not establish that this
viewer composition is faithful. The label now says **UNQUALIFIED image proxy**.
Historical review identities/logs retain the label emitted at their original run.

Classify this as a **review-proxy/control-parity failure**, with the underlying
cause unresolved. The report does not isolate whether the actual candidate source
renderer has the same artifact. Do not attribute it to production rendering or
claim it is merely a proxy implementation bug without direct candidate-source
comparison. No tolerance, shift, repaint, or production correction was introduced.
The existing performance rejection stands independently. Qualification remains
stopped; no production revision is recommended.

Feedback-record verification: the relabelled viewer built on exact Qt 6.8.0 and
6.12.0. Document links, summary JSON syntax and `git diff --check` passed. Existing
review windows were not replaced; they retain the original executable/label.
No new rendering qualification or performance pairs were run.


## Actual-source follow-up — 2026-11-05

Separate baseline/candidate actual-source viewers now exist. Completed targeted
poses show exact source/proxy viewport fidelity; an intermediate full scan/live
trace also matched. The last Qt 6.8 Vulkan candidate unaligned replay stalled
before ready, so attribution stops at that stage. The earlier report is preserved;
no proxy-only diagnosis or motion acceptance is established. See the
[actual-source diagnostic record](artifact-verification.md) for hashes, captures,
completed checks and unreached cases. The performance rejection remains binding.
