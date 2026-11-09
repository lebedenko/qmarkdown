# Four-scale correctness study — results

**The bounded study is complete. Equal-size comparison is blocked by the SDL/Skia
viewer's coordinate mapping, and Qt 6.8 Vulkan has a reproducible shutdown failure.**
No backend is selected or production-qualified. The existing shipping renderer is
unchanged. DP-1's original 125% configuration was restored and verified.

The study's purpose is to make the architecture comparison trustworthy: establish
correct behavior and comparable physical dimensions before judging quality or
performance. It supplies evidence for that decision, not a production migration.

## What ran

Exact Qt 6.8.0/6.12.0 × distance-field/curve × OpenGL/Vulkan, plus pinned explicit
shaping-boundary Skia × OpenGL/Vulkan; fifteen fixtures at actual desktop scales
100%, 125%, 150%, 200%. Document zoom was 1 for scale comparisons, with separate
zoom/reset operations. Existing fonts, padding, shaping and tolerances were retained.
Qt controls retain their documented raster fallbacks; their passes do not establish
pure native padded-run composition or general document correctness.

| Coverage | Passed | Failed | Skipped | Graphical launches |
| --- | ---: | ---: | ---: | ---: |
| Four fresh-window sets | 544 | 120 | 16 | 664 |
| Completed replacement live pass | 8 | 2 | 0 | 10 |
| Logical 690-cell schedule | 552 | 122 | 16 | 674 |

All 122 failures above are Qt 6.8 Vulkan exits with signal 11 after the shutdown
record. Each fresh configuration ran all fifteen independent fixtures; its failed
initial document made the two dependent recreations ineligible. Eight other
configurations pass every fresh scale set. The live pass completes all seven
checkpoints in all ten windows before two Vulkan shutdown failures.

Two preserved harness failures add eleven historical attempts: the original one
with incomplete xdg-output name handling, and ten interrupted live windows after
a confirmation-file publication race. Their causes, captures and terminal journals
remain intact. Explicitly approved replacements yielded **685 actual launches**,
below the 691 ceiling. There are no missing logical schedule cells; skipped
recreations are not represented as passes. Superseded interrupted checkpoints remain
untested in their own historical run.

## Configuration and physical dimensions

DP-1 and every requested desktop setting were read back; window/output associations,
DPR or SDL drawable ratios, window/buffer/viewport extents and source/binary/font
identities are retained. No Qt scale override or font-size normalization was used.

For the nominal 22-unit font, the recorded vertical mappings imply:

| Desktop scale | Fresh Qt | Fresh SDL/Skia | Live Qt, started at 200% | Live SDL/Skia, started at 200% |
| --- | ---: | ---: | ---: | ---: |
| 100% | 22 px | 22 px | 22 px | 11 px |
| 125% | 27.5 px | 22 px | 27.5 px | 13.75 px |
| 150% | 33 px | 22 px | 33 px | 16.5 px |
| 200% | 44 px | 22 px | 44 px | 22 px |

These are nominal font sizes transformed through recorded surface/window mappings,
not measured glyph ink heights. At 150%, SDL surface-width rounding introduces a
0.5-physical-pixel horizontal difference; the vertical dimensions are exact and
rounding cannot explain the size ratios. Geometry tolerances were not enlarged.

Fresh Qt uses an 800 × 600 surface and scales its buffer with DPR. Fresh SDL/Skia
keeps an 800 × 600 buffer while its surface shrinks as desktop scale rises. Live
windows retained the initial fractional preference (240/120): Qt kept DPR 2 and
an 800 × 600 surface; SDL kept a 400 × 300 surface. Thus ten collected lower-scale
SDL live checkpoints have different physical sizing from fresh windows at the
same settings. Equal-size quality/performance comparisons are not valid yet.

The complete Wayland traces show no later fractional-preference event in these
windows, despite output updates. This records behavior of the tested compositor,
toolkit and adapter combination; it does not establish which layer should change.
No compositor sampling-kernel or final screen-image quality claim is made.

## Live transitions and correctness

The ten replacement windows stayed alive through 200 → 150 → 125 → 100 → 125 →
150 → 200%. All **70 source, inverse-coordinate and object-containment checkpoints
pass**, with stable window IDs. All **30 repeated-scale comparisons** match exactly
for captures, geometry, mappings and selections. Eight windows then shut down
normally; Qt 6.8 Vulkan distance/curve exited with signal 11.

These passes establish the named semantic and repeatability checks. They do not
close the separate physical-size/lifecycle mapping gap. Checks exercise the viewer
mapping paths and synthetic coordinates; they are not blanket end-user interaction
acceptance. Capture/contact-sheet inspection by the agent is not owner visual
acceptance. No performance qualification follows from this multi-window pass.

## Architecture consequence

Continue using the shipping renderer while evaluation remains open. Before a fair
Qt/Skia quality or performance comparison, establish one document-to-surface-to-buffer
coordinate contract and diagnose fresh versus live scale handling. Fixing this must
preserve source/input mappings; multiplying font sizes to make screenshots match
would conceal the underlying ownership problem. Separately isolate and resolve the
minimum-Qt Vulkan shutdown failure without dropping Qt 6.8 support.

The study does not authorize those renderer/integration changes, a backend choice,
or publication. Native composition, broader document coverage, owner visual review,
performance and integration/resource qualification remain separate prerequisites.

## Evidence and verification

- [Consolidated 690-cell accounting](evidence/scale-study-live-recovery/consolidated.json).
- [All four fresh sets and their archives](evidence/scale-study-v2/README.md).
- [Completed live pass, archive and restoration](evidence/scale-study-live-recovery/README.md).
- [Seventy live checkpoints and fresh/live mapping comparisons](evidence/scale-study-live-recovery/live-checkpoints.json).
- [Original failed attempt](evidence/scale-study/README.md) and
  [interrupted live pass](evidence/scale-study-v2/terminal/summary.json).
- [Delivery validation](evidence/scale-study-live-recovery/delivery-validation.json)
  verifies frozen identities, 70 capture hashes, completed-journal recovery,
  duplicate-launch rejection under a fatal sentinel, restoration and launch bounds.

All historical evidence through ba843e0 remains unchanged. Sources, binaries,
libraries, fonts, commands, raw attempts and archive hashes/read-back are retained
in the linked evidence chain. The atomic publisher passes eleven focused tests;
the preceding validator correction passed eighteen offline tests. The original
observer checks passed 45 fixture/build cases. Final Light/links/syntax/whitespace
results are recorded in [verification](verification.md). No shipping changes,
commits, merges or publication were performed.
