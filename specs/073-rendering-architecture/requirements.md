# 073 — QMarkdown 2.x rendering architecture

## Approval and scope

The owner's 2026-11-05 instruction to implement the supplied architecture plan
approves this evaluation, its design and mapped tasks. It does not approve a
production rewrite, API change, release or backend selection without evidence.
Feature 065's existing experiments and working-tree changes remain historical.

- R1: Audit layout, shaping, padding, transforms, allocation, sampling and display.
  Isolate defects before attributing them; retain the overlapping historical scene.
- R2: Independently evaluate public Qt distance-field, Qt curve and native Skia
  controls with matching fonts/content/scales. Browser output is a visual reference.
- R3: Prove padded code, mixed formatting and inline-object placement without
  selection-foreground emission, private Qt APIs, QTextDocument or WebEngine.
- R4: Keep glyphs, backgrounds, decorations, selection and clips explicit; cache
  preparation/resources, allowing bounded raster work with measured costs.
- R5: Deadline initialization/rendering stages. Qualify Linux Wayland OpenGL and
  Vulkan at observed DPR 1, 1.25, 1.5, 2; resting, slow motion, flick, reversal,
  settling, resize and scale changes. Software/other platforms are deferred.
- R6: Cover shaping, fallback, overhang, rounded code, rules, tables and objects.
  Inspect actual-source images/motion; exact replay is a separate correctness gate.
- R7: Separate capture from pacing; measure CPU/GPU/preparation/memory and hardware
  refresh. Retain the existing p95 <= one refresh / maximum <= two refresh budget.
- R8: Select only after both native APIs pass quality and performance. Otherwise
  record blockers and unresolved decisions without authorizing a rewrite.

See [design](design.md), [tasks](tasks.md), [verification](verification.md).

## Approved demonstrator continuation — 2026-11-05

The owner explicitly requested implementation of the supplied “Build the native
rendering comparison demonstrator” plan, then asked to retry. That instruction
approves this bounded continuation, including requirements, design and tasks.

- D1: Shared source-indexed fixtures, explicit fonts, padded/adjacent code, mixed
  formatting, wrap, bidi, ligatures, marks, fallback, overhang, objects and document.
- D2: Independent Qt distance/curve controls; cached public nodes where supported,
  labelled physical-phase raster fallback using isolated prepared geometry elsewhere.
- D3: Standalone SDL2 Wayland Skia Ganesh GL/Vulkan with independently shaped
  SkParagraph; pin official source/dependencies outside shipping dependencies.
- D4: Browser visual references and source-preserving selection; unsupported
  reservation semantics must retain a reproducer and explicit status.
- D5: Vulkan reduction cases, observed exposure/frame readiness, robust capture
  errors, monotonic records, pre/post hashes and external 20/10/10-second deadlines.
- D6: Interactive movement, replay, resize, scale, clipping and recreation controls;
  actual DPR matrix on both exact Qt SDKs and both native APIs. Owner visual
  acceptance remains separate from automated checks.
- D7: Retain captures and support matrix, preparation/resource/dependency costs.
  Performance qualification, backend selection and production migration follow later.

Shipping APIs, dependencies and behavior remain unchanged. No merge or publication
is authorized by this milestone.
