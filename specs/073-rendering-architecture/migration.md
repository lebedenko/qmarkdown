# 2.x architecture decision record

Status: **evaluation incomplete; no backend selected; no production rewrite approved**.

## Established direction

Keep the private Markdown parser, semantic document model and host-controlled
styling boundaries. Separate a source-mapped layout snapshot from explicit glyph,
background, decoration, selection, object and clipping elements. Cache immutable
layout/resource generations; translation should not recreate glyph resources.
Permit bounded raster elements where quality requires them, with observable
invalidation and memory cost. Do not adopt document HTML, QTextDocument or WebEngine.

## Candidate decisions

| Candidate | Evidence | Decision |
| --- | --- | --- |
| Qt distance field with current padded runs | Public text node cannot ingest the adjusted run positions | Direct adapter rejected; alternative layout composition remains unproven. |
| Qt curves with current padded runs | Same public input boundary | Same adapter blocker; no quality/performance rejection inferred. |
| Native Skia | No native Skia SDK located in inspected system/project paths | Unimplemented and unqualified, not rejected. |
| Retained paragraph pixels | Historical blur/sampling complaints and performance rejection | Not an accepted 2.x replacement. |

## Dependency/resource accounting

The standalone Qt probe adds no shipping dependencies and links only existing Qt
Quick components. Qt owns glyph caches and render-thread node resources; the probe
retains one layout and one text node. Diagnostic raster captures are separate and
are excluded from the motion notification sample. Actual binary sizes and hashes
are retained with verification. No GPU-memory or CPU-work measurement is claimed.

Skia would add a native graphics library, build recipe, license/provenance and
GL/Vulkan interop/lifetime handling. Independent shaping additionally needs a
font manager and shaping/unicode stack; using Qt's existing shaped runs instead
must explicitly preserve source and fallback-font identities. No package-size or
memory estimate is presented as measured. Software and other platforms need later
qualification even after Linux native success.

## Decisions still required before a decision-complete migration specification

1. Prove a public Qt layout composition for padding/objects without reshaping
   errors, or implement a native Skia positioned-run renderer and its Qt bridge.
2. Reproduce each reported stroke/clip/seam defect in minimal and realistic
   documents. Keep actual-source interactive controls and browser references.
3. Pass both native APIs at all four DPRs, including motion, resize, scale
   transitions, complex shaping, tables, selection, hit testing and objects.
4. Collect actual presentation feedback, CPU/GPU work and bounded memory costs;
   require p95 within one refresh and max within two, separate from capture work.
5. Only then define API breaks, compatibility adapters, rollout order and removal
   criteria in an owner-reviewed production requirements/design/task plan.

This document is a blocker record and migration outline, **not** the requested
final decision-complete migration specification. The unchecked tasks remain work.

## Demonstrator continuation

The continuation adds an isolated prepared-geometry Qt fallback and an independent
SDL2/SkParagraph control. Neither changes the production renderer. Public
QSGTextNode remains the native path for representable paragraphs; actual padded
code and objects use explicitly labelled raster fallback at the physical phase.
Hybrid output must not be cited as native padded-text support.

Skia's inserted reservations preserve an explicit mapping to UTF-16 source, but
reservation wrapping/bidi/selection semantics are still a qualification gate.
The pinned upstream Vulkan window helper is an experiment dependency, not a
proposed production integration layer. Its build and adaptation costs must be
included in a later backend decision.

Remaining decisions include full table composition, cross-block selection,
compositor DPR coverage for SDL, owner temporal visual acceptance,
presentation timing, CPU/GPU profiling and dependency/resource budgets. No backend
selection or decision-complete production migration specification follows from
capture replay or successful compilation.
