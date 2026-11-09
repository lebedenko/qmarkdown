# Scale-contract correction and verification

**Fresh-window physical sizing is corrected. Live raster-density acceptance remains
blocked by a reproduced missing fractional-scale notification outside the
Qt/Skia demonstrators.** No backend is selected or production-qualified.
DP-1's complete original 125% configuration was restored and verified; every
window is closed. No commit, push or publication was performed.

## What changed

The isolated [SDL3 adapter and coordinate contract](demonstrator/scale-contract/README.md)
use SDL3 3.4.18 high-density windows and distinct window size, buffer size,
pixel-density and display-scale queries. Drawing, input inversion, layout width
and selection checks share the mapping. Mapping generations bind captures to
current geometry, including scroll and zoom. GPU surfaces follow buffer changes;
layout follows logical viewport/zoom changes. Qt controls have instrumentation
only. Fonts, fixtures, padding, shaping, source semantics and geometry tolerances
are unchanged; shipping code and public APIs are untouched.

The SDL2-compatible probe at 125% requested a 640×480 viewport for an 800×600
buffer. Direct SDL3 instead retains an 800×600 surface and 1000×750 buffer. No
requested desktop scale is fed into rendering, no viewport is manually owned,
and no font multiplier or replacement live window was used.

## Separate acceptance outcomes

| Gate | Observed outcome |
| --- | --- |
| Physical comparability | All 6,629 obtained capture mappings pass nominal physical-size/logical-viewport assertions. Qt and Skia retain 800×600 baseline surfaces and identical nominal font mapping. This is mapping evidence, not screenshot-derived ink measurement or owner visual acceptance. |
| Raster density | All 6,559 fresh capture mappings pass. In the live pass, 20 checkpoints at 200% pass and 50 lower-scale checkpoints fail: every window retains density 2 and a 1600×1200 buffer. Acceptance is blocked. |
| Correctness | All 70 live source/inverse/containment checkpoints pass. Fresh cells 014, 562 and 563 have incomplete or failed scripted correctness coverage; they remain failures. Existing assertions establish only their documented coverage. |
| Transitions | All ten window identities remain stable; all thirty repeated-scale comparisons match exactly. Only 20 of 70 live captures exactly match fresh windows at the same scale; fifty fail because buffers remain at the initial density. No contexts/windows were replaced. |
| Lifecycle | 551 final processes exit 0, 121 exit with signal 11, and two are killed after deadlines. Qt 6.8 Vulkan failures remain independently classified and unrepaired. |

Nominal font mapping in fresh windows is 22, 27.5, 33 and 44 physical pixels at
100%, 125%, 150% and 200%, respectively. Baseline buffers are 800×600, 1000×750,
1200×900 and 1600×1200. In lower-scale live windows, correct surface dimensions
must not conceal the stale buffer density and resulting compositor resampling.
No sampling-kernel or final screen-image quality claim is made.

## Complete cell accounting

| Cohort | Passed runner assertions/exit | Failed | Dependent skips | Launches |
| --- | ---: | ---: | ---: | ---: |
| Fresh 100% | 135 | 31 | 4 | 166 |
| Fresh 125% | 136 | 30 | 4 | 166 |
| Fresh 150% | 136 | 30 | 4 | 166 |
| Fresh 200% | 136 | 30 | 4 | 166 |
| Persistent windows | 8 | 2 | 0 | 10 |
| Final schedule | 551 | 123 | 16 | 674 |

These runner statuses do **not** override the separate failed density/transition
gates. All 690 scheduled cells are accounted for; sixteen recreations were skipped
because their prerequisite document failed. There were no final retries or
replacement windows. Eleven diagnostic launches are retained separately, for
**685 total launches out of the 750 ceiling** (11/60 diagnostic allowance).
Recorded automated time, excluding builds and human waits, is 1,278.612 seconds,
within four hours. Final live active time is 45.490 seconds.

Additional correctness failures are retained precisely:

- Cell 014: Qt 6.8 distance/OpenGL, whole-lam-alef at 100%; motion stopped at frame
  28 and the operation deadline expired. No cause is established from that alone.
- Cell 562: Qt 6.8 curve/Vulkan, padded at 200%; timeout during the selected capture.
- Cell 563: Qt 6.8 curve/Vulkan, adjacent at 200%; motion replay failed. Its trace
  contains a Space key event and additional interactive scroll increments. This
  attempt is contaminated by input and does not establish a renderer replay defect.

The other 118 fresh failures are Qt 6.8 Vulkan shutdown-only failures. Two more
Vulkan shutdown failures occur after all live checkpoints. Initial adapter diagnostic
d005 caught a stale-scroll mapping and failed replay; d006 verified its correction.
The final candidate was frozen afterward and never changed during the matrix.

## Minimal external reproduction and attribution

Runtime: labwc 0.20.2-1, wlroots0.20 0.20.2-1, NVIDIA GeForce GTX 1660 SUPER with
615.78.08, SDL3 3.4.18-1, sdl2-compat 2.32.74-1, exact Qt 6.8.0 and 6.12.0.
Package records, compositor/library hashes, executable/font identities, toolkit
records and raw timestamped Wayland traces are retained in the evidence archives.

The minimal `sdl3-probe` uses SDL alone: an 800×600 high-density window and the SDL
window-surface API. It contains no Qt, Skia, document model or custom viewport code.
Build target `sdl3-probe` from the isolated directory's CMake project, then:

1. Set DP-1 to 200% and verify `wlr-randr --json`.
2. Run `SDL_VIDEO_DRIVER=wayland WAYLAND_DEBUG=1 sdl3-probe --live`, retaining
   stdout and stderr separately. It reports density/displayScale 2 and 1600×1200.
3. Change DP-1 to 150% without closing the probe and verify output read-back.
4. Expected: `preferred_scale(180)` and a 1200×900 buffer. Observed in d011:
   `zxdg_output_v1#32.logical_size(2560, 1440)` arrives, but fractional object #38
   retains only `preferred_scale(240)`; repeated SDL queries remain at density 2.
5. Restore the original output configuration and send `quit` on stdin.

The same probe started at 125% in d007 received preferences 150 → 180 → 150 during
125 → 150 → 125%. Starting at 200% is therefore a material reproduction condition.
d011 also retained density 2 after restoration to 125% and exited cleanly.

The missing preference is observable in the incoming protocol stream across both
Qt versions and SDL3, and reproduces without the demonstrators. This supports an
external compositor/protocol-path attribution for this runtime; it does not identify
a particular labwc/wlroots source line or justify patching either in this scope.

## Remaining prerequisite and evidence

Correct compositor-side fractional-scale notifications for already mapped windows
created at 200% are required before live raster-density/fresh equivalence can pass.
After that external correction, a separately authorized verification must establish
correct live buffers without window recreation. Qt 6.8 Vulkan lifecycle repair
remains outside scope, and the three incomplete/contaminated fresh cells need
explicitly authorized new coverage before claiming an entirely clean correctness run.
Owner visual acceptance, production qualification and performance remain separate.

[Evidence index](evidence/scale-contract/README.md) includes full accounting,
per-checkpoint audits, restoration, contact sheets and archive SHA-256/read-back.
The [approved protocol](scale-contract-protocol.md) and all historical approvals,
commit 8a392ae, sources, runners and evidence remain preserved.
