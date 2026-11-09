# Correct scale mapping — approved 2026-11-09

The owner supplied the implementation plan “Feature 073: correct scale mapping
and verify comparability” and explicitly requested implementation in this session.
This approves the requirements, design and tasks below. Preserve commit 8a392ae,
all historical sources, runners and evidence. No commit, push or publication.

## Requirements

SC1: At zoom 1, one document unit is one logical surface unit. Retain nominal
fonts (including 22 units), fixture sizes, 24-unit padding and 800 × 600 baseline.
Desktop scale s makes nominal content s times as large in physical pixels.
Keep physical comparability and raster density as separate acceptance gates.

SC2: Separate document, toolkit-window, Wayland-surface and buffer coordinates;
record forward/inverse transforms, zoom, scroll, extents, content scale, density
and mapping generations. Use one mapping for drawing, clipping, layout and input.
Only integer-extent-derived rounding is allowed; existing tolerances stay fixed.

SC3: Preserve explicit shaping boundaries, preparation, source selection and
object containment. Use pinned SDL3 3.4.18 with high pixel density, window/pixel
size, pixel-density and display-scale queries. Query after events and checkpoints,
including unchanged-buffer transitions. Toolkit owns the Wayland viewport.
No requested-scale rendering override, font multiplier or replacement live window.
Qt controls may change only for instrumentation or a demonstrated mapping defect.

SC4: Record exact Qt 6.8.0/6.12.0, sdl2-compat 2.32.74, SDL3, compositor,
drivers, runtime libraries, fonts and executable identities. Minimal Qt, SDL2
and SDL3 probes correlate timestamped toolkit events and Wayland object IDs.
An external failure requires a reproducible trace and leaves the affected gate
blocked; no toolkit/compositor patch or system package replacement.

## Design

Use sibling demonstrator/scale-contract and evidence/scale-contract directories.
For Wayland, surface coordinates are toolkit window coordinates; buffer ratios
are derived independently on each axis from integer extents. Content units map
through the reported window display scale divided by pixel density, then through
the measured window-to-buffer ratios. Record the resulting physical extent and
compare against compositor scale separately. Do not infer density from screenshots.
Generation changes on any mapping input, including zoom and scroll. Layout
invalidates on logical viewport/zoom changes; GPU surfaces on buffer changes.
Independent expected-coordinate tests must supplement round-trip checks.

## Tasks and checks

- SC-T1: Record approval and contract; inventory runtime and baseline output.
- SC-T2: Implement and test shared mapping, isolated SDL3 adapter and four probes.
- SC-T3: Test fractional rounding, resize/reset, unchanged-buffer scale changes,
  reordered/duplicate/missing events, stale generations, wrong output, malformed
  records, deadlines/resources, atomic handoff, interruption/resume and duplicates.
- SC-T4: Diagnose with at most 60 launches, each retaining hypothesis, revision and
  expected observation. Freeze candidate before final verification; reserve 690
  final launches separately. Build with two workers; execute graphically serially.
- SC-T5: Complete original ten-configuration matrix: fifteen fixtures plus two
  dependent document recreations at four scales (at most 680 fresh processes).
  Retain motion/replay, clipping, settling, lifecycle, bidirectional selection and
  exact copied-source assertions. Skip only dependent recreations after failure.
- SC-T6: Keep ten windows/contexts alive through 200 → 150 → 125 → 100 → 125 →
  150 → 200%; pause other scripted activity while inspecting each. Compare fresh
  and repeated-scale visits. No silent retries or replacement final windows.
- SC-T7: Archive traces, captures/contact sheets, identities and full accounting;
  verify hashes/read-back and original output restoration. Update architecture
  and verification; run final Light, links and tracked/new-file whitespace checks.

Use tested atomic confirmation publication, append-only journals, immutable
captures and resumable untouched-cell scheduling. Retain 20-second startup,
10-second operation/shutdown, 120-second fresh process, 4 GiB per process,
8 GiB combined live and four-hour automated limits (builds/human waits excluded).
Total ceiling 750 launches. Request actual DP-1 changes from the owner, verify
identity/read-back before execution, restore recorded original configuration on
completion or termination. Qt 6.8 Vulkan exit failures remain failures independent
of rendering assertions. Report physical comparability, raster density,
correctness, transitions and lifecycle separately. Owner visual acceptance,
production/performance qualification and backend selection remain separate.
