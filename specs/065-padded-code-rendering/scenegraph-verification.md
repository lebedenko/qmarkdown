# Retained run-texture scene-graph gate — actual verification

## Scope and source

The owner explicitly approved this bounded revision by requesting implementation
of the supplied plan on 2026-10-23. See S1–S9 in [requirements](requirements.md),
[design](design.md), and [tasks](tasks.md). Verification continued on 2026-10-24. The interrupted host session cleared
`/tmp`; fresh final builds and raw output are retained under the ignored durable
`build-ci/065-scenegraph` directory before evidence collection.
The continuation starts at PR #51's verified commit
`18f1259e27e9407d642b309b79ce7a795c6fc0fc`; PR #51 remains unchanged.

The shipping source tree is exported from that commit; only the existing
prepared-run snapshot patch is applied to the exported tree. The new executable
is compiled by the repository-owned standalone project, outside shipping CMake.
No shipping renderer, public API, dependency, version or installed package changes.

The GUI prepares actual complete runs and immutable raster images, preserving
font, glyphs, positions, origins, colour and order. The render interface contains
only images, physical bounds, DPR and generation/run identities. Each public
image node owns its texture, created on the render thread. Textures have linear
filtering/clamp wrapping, no mipmaps/atlas, and DPR-1 image metadata. Decorations
are disabled consistently. Reference paths are crop tiles and separate full-canvas
textures per run; neither is a proposed paragraph cache.

## Environments and procedure

Fresh Release/Ninja configurations use exact Qt 6.12.0 on Arch Linux and exact
Qt 6.8.0 from the cached CI image, with two build workers. The minimum image's
Wayland plugin could not initialize. Its cached exact SDK was copied to a
disposable `/tmp` location; native runs use that SDK and the same built binary
against the host's working Wayland/NVIDIA driver stack. Nothing was installed or
replaced. Public Vulkan headers already present on the host are mounted read-only
for the standalone target's device-identity query, without changing library inputs.

Both SDKs use the identical explicitly loaded DejaVu Sans Oblique/Mono files and
verify prepared head/name/cmap/glyf tables. Fresh prior software preparation gates
pass: 2 historical + 256 basic + 960 extended cases per SDK/order, 4,872 total.
Historical environment-resolved controls remain separate and retain their failures.

Every transport process asserts the actual SDK, scene-graph API and effective
window DPR. Native queries identify the NVIDIA GeForce GTX 1660 SUPER using public
OpenGL renderer/vendor/version and Vulkan physical-device properties; software
GPU fallback cannot qualify. Natural display DPR is calibrated rather than assumed.
Native GUI targets run serially. Fixed-state capture disables vsync throttling and
sets the QPA update idle interval to zero; there are no animations or timing claims.

The viewport is fixed at 600×384 logical pixels (rounded outward to a common
grid for effective DPR and Qt platform scale), large enough for every fixture.
Changing text layout width does not resize the native swapchain.
Both logical-to-platform and logical-to-physical extents are integral, and every
capture must match the physical window size before pixels are compared. This is
an unscaled transport test. Arbitrary viewport sizes, display migration and other
transforms are not qualified. Public `grabWindow()` synchronizes and renders each capture; the harness verifies
a newer synchronization and exact dimensions. It does not wait for a compositor
presentation callback. Invalid extents are recorded, not compared or cropped.

Application counters describe preparation and image node/texture creation and
ownership. Fresh QPainter diagnostic painting is excluded from candidate
preparation counters. These counters do not measure internal GPU uploads or memory.

## Harness development observations (not qualifying evidence)

Earlier attempts are diagnostic and do not qualify any final stage. Their
temporary raw files were lost when the interrupted machine restarted; the
following observations are session notes, not retained reproducible matrices.
Historical committed PR #49/#51 evidence remains intact.

- The first canvas used arbitrary rounded logical window heights. At DPR 1.25,
  physical viewport rounding introduced rescaling; software full-width row
  differences and GPU one-channel differences were observed.
- Concurrent native windows produced incomplete frame waits. Later runs serialize
  GUI windows and wait for rendering after a new synchronization, not an already
  in-flight frame. Callback state outlives render-thread shutdown.
- Effective-DPR alignment alone was insufficient on native Wayland/OpenGL:
  a requested 750×205 capture stably returned 750×206. The corrected viewport
  aligns to both effective DPR and the Qt platform-scale grid; the capture-size
  assertion remains strict.
- A minimum-Qt Vulkan calibration crashed during the extra pre-destruction hide
  step. The retained GDB backtrace places the fault in the NVIDIA driver during
  Qt window visibility handling. Removing redundant hide/release calls and using
  standard QQuickWindow-owned teardown makes the focused calibration exit cleanly.

A matching-size OpenGL DPR-1.25 probe passed exact transport parity before the
restart. Later incomplete attempts motivated a fixed native viewport and direct
synchronized grabs rather than compositor-frame waits. No tolerance was raised;
only the fresh, retained final matrix below can qualify the experiment.

## Final stage results

All six targets complete every required stage and both fresh control orders:

| Stage | Cases per SDK/backend | All six targets | Required differences | Result |
| --- | ---: | ---: | ---: | --- |
| Static transport | 4,872 | 29,232 | 0 | Pass |
| Scroll/clip, including warm-up captures | 7,488 | 44,928 | 0 | Pass |
| Three-window ownership smoke | 96 | 576 | 0 | Pass |

All **74,736 scene-graph cases** pass exact candidate/crop/full-reference equality.
All prepared run/glyph occurrence checks and ordered image-node counts pass.
Cropped ink, tile-border ink and clip leaks are zero. All hot scroll captures
preserve application counters and node/texture identities after warm-up; no new
preparation/raster/image-node/application-texture creation is observed. The
independent audit verifies 4,608 exact returns to origin, 576 matching recreation
checks and 432 explicitly balanced fixture/window teardowns. Final summaries also
report zero live owned roots, image nodes and textures.

Fresh-text diagnostics are intentionally different: static native captures have
at most one channel value of difference from QPainter; fractional scrolling has
maximum channel differences of 192 on software and 156 on OpenGL/Vulkan, on both
SDKs. These are not failures of the selected transport gate, but they prevent any
claim of exact fresh-text parity or perceived-quality acceptance.

[Aggregate metrics](evidence/scenegraph/summary.json),
[independent audit](evidence/scenegraph/audit.json),
[identities](evidence/scenegraph/identities.json) and
[stage progression](evidence/scenegraph/progression.json) accompany complete
compressed JSONL and stderr for every final process. Exact commands and selected
environments are in the per-target summaries and [launch record](evidence/scenegraph/launch.json).
Representative reference/candidate/difference PNGs cover both backgrounds at
DPR 1.25 and 2, candidate-first, for every SDK/backend/stage; all other captures
remain in the durable ignored workspace. Both comparison orders retain complete
numeric records. The complete fresh preparation output is retained separately.

The exact detector checks known positive and negative premultiplied-pixel samples
at process startup (two altered pixels, maximum channel delta 40). Font table,
GUI/render-thread, atlas, phase, viewport and observed-transform assertions remain
enabled in Release builds. No threshold was relaxed to obtain these results.

`python3 scripts/check.py` and `git diff --check` pass. Qt synchronization with
`python3 scripts/ci-toolchain/qt_versions.py --sync-host /usr/bin/qmake6` reports
6.12.0 without changing the committed targets. A native Vulkan DPR-1.25 colour
boundary reference/candidate pair was inspected; their decoded pixels are equal.
These checks and the retained native readbacks do not measure presented-frame
timing, perceived shimmer or production performance.

## Delivery and limits

Reproduction commands and stage guards are in the [standalone README](retained-prototype/README.md).
Source/font/toolchain/library identities, commands, compressed raw metrics/logs and
representative captures accompany the final results. A missing or failing required
target prevents progression; scroll and recreation results cannot be inferred from
static success. Hosted Light is separate from the exact-SDK experiment.

One new stacked PR contains specifications, implementation and actual evidence.
Final-head hosted outcomes belong in its description/checks, avoiding evidence-only
follow-up commits. Leave it unmerged with auto-merge disabled; leave PR #51 unchanged.
Passing any bounded gate permits design review only. Production invalidation,
backgrounds/decorations, broader shaping, interactions, resource budgets, perceived
shimmer and performance remain unqualified.
