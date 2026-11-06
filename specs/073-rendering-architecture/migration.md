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

## Bounded correctness continuation — 2026-11-05

The owner approved C1–C6, design and tasks before implementation. The isolated
Qt preparer retains source commit `43d6bba842bb5b7ab23889e49d5f138c6f22a7fb`.
A checked patch recovers a missing wrapped bidi cursor cell from the same shaped
line's public glyph-run bounds when both cursor affinities collapse. The viewer
normalizes drag endpoints into the logical interval required by the staged
selection painter. Interactive copy and automated checks share these paths.
This is an experiment correction, not a production fix or native padded-text API.

The whole-paragraph SkParagraph trial remains rejected: stable source-fragment
boundaries do not ensure that inserted reservations straddle a visual line break.
The retained wrapping reproducer stabilizes but leaves later fragments without
left padding. A further capability is needed to bind both padding edges to each
visual fragment without changing legal wrapping, shaping or bidi/source mapping.
This step does not authorize another layout engine, atomic code spans or spaces.
Expanded object-bound checks also expose fractional top overhang in some Skia
placeholder reservations; their baseline/vertical-bound semantics need resolution
before composition acceptance. Source-copy success does not close either gap.

The standalone empty-window Vulkan reproducer fails on Qt 6.8.0 in both default
and basic render loops, including explicit release while the event loop runs.
Each variant passes three independent process recreations on Qt 6.12.0. No tested
harness lifecycle variant qualifies as a minimum-Qt correction. Further work
requires a Qt/Wayland/Vulkan teardown diagnosis with the retained bounded
backtraces and resource-callback order; a driver stack frame alone does not
identify ownership of the defect. No SDK, driver or backend change is adopted.

See the appended [verification record](verification.md) and
[current support record](evidence/correctness/support.json). Performance,
production migration, backend choice and owner visual acceptance remain open.

## Fragment-aware upstream experiment — blocked 2026-11-05

F1–F7, design and tasks were explicitly approved before implementation. A checked
40-line diagnostic patch in a separate copy of the pinned Skia build exposes a
boundary constraint before candidate fitting: the existing DejaVu Sans Mono font
shapes Arabic lam–alef into glyph 3254 across two graphemes. Adjacent code spans
`[0,1)` and `[1,2)` retain separate identities, but their shared edge is inside
that glyph. Independent whole-cluster translation cannot provide both padding
edges there. Shaping each character separately yields glyphs 1141 and 1117,
violating the approved unchanged-shaping constraint.

The next architecture decision must choose explicit shaping boundaries at code
span edges (allowing changed ligatures/joining), restrict supported span edges to
shaped-cluster boundaries, or define sub-glyph clipping/placement and its geometry
semantics. Upstream range clipping exists, but moving clipped pieces would split
the intact cluster required by this design. No option is adopted automatically;
this is not evidence that every possible SkParagraph extension is impossible.

Unmodified behavior matches exactly on 751 fixtures / 775 blocks for the
**diagnostic patch only**. Padded candidate scanning, authoritative adjusted
geometry and native qualification stopped before implementation. The prior
placeholder-trial rejection remains valid. Object diagnosis identifies rounding
of a 36.1885 line height to 36 and a resulting −0.1885 vertical delta; the baseline
is already integral. DPR-1 raster coverage does not excuse its logical-bound
failure at the unchanged 0.05 tolerance. A padded reservation fix remains open.

See [experiment instructions and maintenance limits](demonstrator/fragment-experiment/README.md),
[new support record](evidence/fragment-experiment/support.json) and
[actual results](evidence/fragment-experiment/summary.json).
Qt controls and the Qt 6.8 Vulkan shutdown blocker are unchanged. No production
dependency, backend choice, performance qualification, merge or publication follows.


## Explicit shaping-boundary feasibility — 2026-11-05

The owner approved B1–B8 and the revised design/tasks. Code-span edges now delimit
shaping in a new isolated SkParagraph stage. The earlier intact-cross-span-glyph
blocker remains a valid historical result under its old constraints; it does not
block this explicitly revised mechanism.

The [new automated evidence](evidence/boundary-experiment/README.md) passes control
and boundary baselines, padded geometry, transitions, source/caret consistency,
object containment and 120 native OpenGL/Vulkan processes at observed DPR 1.
This establishes feasibility for the tested isolated slice, not a production
backend decision. Owner visual acceptance, unavailable fractional/native DPRs,
performance and production integration remain separate. The five-file private
upstream patch requires rebase/ABI review and full regression checks on pin
updates. The independent Qt 6.8 Vulkan shutdown failure is unchanged.
