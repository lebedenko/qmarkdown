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
