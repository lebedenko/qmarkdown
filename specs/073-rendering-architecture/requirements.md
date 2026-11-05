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

## Bounded correctness continuation — approved 2026-11-05

The owner explicitly approved these updated requirements, design and mapped
tasks on 2026-11-05 ("approved"). Approval covers C1–C6 only; prior demonstrator
approvals and failures remain unchanged.

- C1: Reproduce `bidi-objects` code-edge selection on exact Qt 6.8.0 and 6.12.0.
  Record prepared cursor cells, line membership, visual direction and hit tests.
  Correct only isolated staged preparation, with one mapping shared by interactive
  selection and geometry tests. Exact copied source must remain `code`.
- C2: Test padding on every visual Skia code fragment, vertical reservation,
  object placement and exact source copying. Trial per-fragment reservations in
  the existing whole-paragraph SkParagraph layout, with at most eight layout
  passes and explicit cycle/non-convergence rejection. Preserve UTF-16 mappings.
- C3: Isolate minimum-Qt Vulkan shutdown using the existing empty-window case.
  Compare default/basic render loops and scene-graph release while the event loop
  remains active, retaining teardown order and bounded backtraces on both SDKs.
  Adopt a harness correction only with preserved backend and successful normal
  shutdown and recreation evidence.
- C4: Begin with CPU/offscreen geometry on both exact SDKs and pinned Skia.
  Cover original failures, widths immediately around wrapping transitions,
  adjacent code, mixed RTL/LTR, combining marks, surrogate pairs and objects,
  with both selection directions and exact copied source.
- C5: After geometry passes, run affected fixtures serially on OpenGL/Vulkan,
  both Qt renderers and observed Qt DPR 1, 1.25, 1.5 and 2. Skia uses only its
  observed DPR; unavailable scales remain explicit. Check stationary capture,
  selected text, resize/reset, motion/replay and three process recreations.
  Shared layout or lifecycle changes require the complete demonstrator matrix.
- C6: Preserve historical failures and input/binary/font identities; append
  before/after results, update support and review/migration records, and run
  supervisor tests, repository lightweight checks and whitespace validation.
  Close gaps only with passing behavior; otherwise name the failing case,
  attempted correction and capability required next.

No shipping API, dependency, renderer, version or release changes are proposed.
Broader layout replacement, production fixes, cross-paragraph selection, general
tables, performance qualification and backend selection remain outside scope.
Owner visual acceptance, merge and publication are separate authorization gates.
