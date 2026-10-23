# 065 — Padded inline-code rendering

## Approval

On 2026-10-22 the owner explicitly requested implementation of the supplied
Iteration 065 plan. That request approves its requirements, design and mapped
tasks, including the prototype gate, one non-draft PR, and stopping for a revised
design if the gate fails. Merge and publication are excluded.

## Requirements

- R1: After preparation, padded inline-code paragraphs, including rounded
  backgrounds, scroll by transforming retained content. No paragraph glyph raster
  paints, layout rebuilds or glyph-node rebuilds during settled scrolling.
- R2: Preserve authoritative layout geometry, wrapping, shaping, padding,
  alignment, hit testing, mappings, decorations and interaction behavior.
- R3: Prototype public QSGTextNode selection-foreground interval emission with
  transparent surrounding text and one shared draw-only layout. Preserve glyph
  identities and positions, overhang and complete ink; emit every visible glyph
  once without hidden paragraph content.
- R4: Gate on exact Qt 6.8.0 and 6.12.0, including software rendering. A failure
  stops integration and retains diagnostic evidence for a revised design.
- R5: If the gate passes, retain small rounded-background image textures until
  geometry/style/DPR changes; retain native eligibility and atom raster fallback.
- R6: Preserve public QML/CMake APIs, versions and dependencies. No private Qt
  APIs, geometry changes or substitute bitmap paragraph caching.
- R7: Qualify runtime-confirmed OpenGL/Vulkan DPR 1/1.25/1.5/2, lifecycle,
  interactions and resource retention. Compare fresh 1.17.1 and candidate fixed
  traces separately from instrumentation; report preparation/resources/memory and
  timing without assuming speedup. Replay p95 <= one refresh, max <= two.

See [design](design.md), [tasks](tasks.md) and [actual verification](verification.md).

## Retained glyph-texture revision — approved 2026-10-22

The owner's explicit request to implement the supplied retained glyph-texture
plan renews approval for the following replacement experiment. The preceding
requirements and failed selection probe remain historical, not implementation
direction. Production integration requires a later approved plan.

- T1: Extract actual prepared `m_paintedRuns` in a staged diagnostic patch; keep
  fonts, indexes, positions, padding translations, colors and decorations.
- T2: Rasterize individual glyphs using public QPainter APIs on the preparation
  thread, including ink, transparent margins and local subpixel phase. Share
  immutable images per surface by font identity/index/color/DPR/phase.
- T3: Transfer only images and geometry to render-thread-owned image nodes and
  textures. Preserve backgrounds, Find, selection, glyphs, focus order; keep
  decorations separate and rounded backgrounds in small retained images.
- T4: Invalidate affected resources on content/style/DPR changes. Settled
  scrolling transforms retained content only; overlays retain glyph resources.
- T5: Correctness failure rejects the experiment and stops expansion. Otherwise
  qualify exact Qt 6.8.0/6.12.0 software, then confirmed OpenGL/Vulkan at DPR
  1/1.25/1.5/2, shaping, clipping, fractional translation and lifecycle.
- T6: Require zero replay layout/raster/upload/node rebuilds; balanced teardown.
  Compare fresh 1.17.1 fixed traces with clean timing separate from counters:
  p95 <= measured refresh interval, maximum <= two. Report preparation, textures,
  nodes, image bytes and bounded memory. A lifecycle or timing failure rejects.
- T7: One open non-draft PR with specs/prototype/evidence and final-head hosted
  checks. No merge, auto-merge, release, package replacement, public API, version,
  dependency or shipping-renderer changes. Automated success permits design
  review only; visual acceptance and production readiness are separate.

## Diagnostic revision — approved 2026-10-23

The owner's explicit request to implement the supplied diagnostic plan approves
these requirements, design and tasks before implementation. Both failed probes
and their evidence remain historical. No shipping or scene-graph work is authorized.

- D1: Compare whole prepared runs, reconstructed and metadata-preserving individual
  glyphs, local tiles, original-coordinate translated tiles and full-canvas crops.
- D2: Preserve order, fonts, indexes, positions, colors and painter settings; disable
  decorations consistently. Log metadata, transforms, phases, bounds and image DPR.
- D3: Independently select comparisons; never stop remaining controls on mismatch.
- D4: Reproduce historical fX at (24.25,24.25), DPR 1/1.25; test f/X/fX/fXfX
  at DPR 1/1.25/1.5/2 with independent physical X/Y phases 0/.25/.5/.75.
- D5: Use fresh exact Qt 6.8.0/6.12.0 offscreen software builds; retain exact pixel,
  support, channel, border and emission metrics, identities and representative images.
- D6: Report evidence-supported diagnosis and recommendation in the experiment continuation PR, open and
  unmerged, auto-merge disabled. Exact parity qualifies this matrix only.
