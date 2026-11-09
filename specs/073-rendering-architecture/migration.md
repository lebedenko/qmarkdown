# 2.x architecture decision record

## Current decision — evidence through ba843e0

Reviewed 2026-11-09. **Evaluation incomplete; no backend selected; no production
rewrite approved.** Native Skia is implemented and feasible for a tested isolated
slice; it is not production-qualified. Continue using the existing renderer while
these gates remain open. This is a disposition of existing evidence, not a claim
that the existing renderer meets the 2.x quality/performance requirements.

At the historical review cutoff, the owner approved the documentation review and
next-milestone proposal only. The later scale-study approval below supersedes that
execution restriction within its bounded scope.
No measurement, analysis regeneration, renderer implementation, backend selection,
integration, commit or publication is authorized. The historical sections below
retain their original scope and approvals; this section supersedes their current
status claims. Support requirements, host-controlled styling, architectural
boundaries and quality thresholds are unchanged.

### Requirements and evidence gates

“Passed within stated coverage” closes only the named subgate. “Failed” preserves
an observed failure; “unavailable” means required evidence could not be obtained;
“untested” means the required qualification has not been performed. Multiple rows
per requirement deliberately prevent a partial pass from becoming an overall pass.
All evidence predates or is included in `ba843e0`.

| Requirement / gate | Status | Evidence and tested configuration | Remaining limitation |
| --- | --- | --- | --- |
| R1 — coordinate/layout audit and defect isolation | Passed within stated coverage | [Audit](audit.md), [phase diagnosis](evidence/phase-experiment/README.md) and [corrected scale facts](evidence/performance-experiment/scale.json): retained Qt controls and pinned Skia on the shared Wayland workstation; 24 zero/padded diagnostic cases. | Counts are not CPU-time shares; compositor sampling filter and exact final coverage remain unknown. No universal defect attribution. |
| R2 — public Qt distance-field/curve direct padded-run adapter | Failed | [Initial verification](verification.md#implemented-and-measured) and [correctness support](evidence/correctness/support.json): Qt 6.8.0/6.12.0 public QSGTextNode accepts layouts, not arbitrary adjusted glyph runs. | Rejects this adapter, not either Qt backend. Alternative native composition is untested; padded/object demonstrator output uses labelled raster fallback. |
| R2 — comparable independent native controls | Untested | [Scale explanation](evidence/performance-experiment/README.md#corrected-scale-explanation): SDL2-compat 2.32.74-1, Ganesh GL/Vulkan, desktop 1.25, drawable/window ratio 1. Qt controls cover observed window DPR 1/1.25/1.5/2. | Equal physical text sizing across Qt/Skia is not established; browser references are visual references, not qualification controls. |
| R3 — padding, shaping, source mapping, selection and objects | Passed within stated coverage | [Boundary support](evidence/boundary-experiment/support.json), [geometry summary](evidence/boundary-experiment/geometry-summary.json): pinned Skia `00227028b0c3cb3052432a456af2027a49e598b2`, five-file explicit shaping-boundary patch; 751 fixtures/775 blocks, 14 regressions, 41 transition brackets, 14,272 vertical cells and 2,901 fragments; containment tolerance 0.05. | Code edges deliberately delimit shaping under B1–B8. Explicit fallback only, unresolved glyphs and near-edge snapping remain documented; not general document acceptance. Qt staged selection correction does not create native padded-run support. |
| R3 — old whole-paragraph reservation adapter | Failed | [Correctness support](evidence/correctness/support.json): wrapping loses fragment padding and fractional object bounds fail. [Fragment support](evidence/fragment-experiment/support.json) records the intact cross-span glyph constraint. | Later approved shaping boundaries supersede that constraint for the new patch only; neither failure rejects all SkParagraph mechanisms. |
| R4 — explicit geometry/resource reuse | Passed within stated coverage | [Performance report](evidence/performance-experiment/README.md#native-rendering-presentation-and-memory), [native summary](evidence/performance-experiment/native-summary.json): completed native stationary/motion/selection scenarios have zero measured layout calls; resize lays out. [Recovery counters](evidence/performance-experiment/recovery/diagnostic-summary.json) verify completed shaping-reuse cases. | Resource reuse in an isolated renderer does not establish bounded total production memory, invalidation correctness for every resource or Qt integration. |
| R4 — scanner retention | Failed | [Continuation summary](evidence/candidate-scan-continuation/summary.json) and [diagnosis](evidence/candidate-scan-diagnosis/report.md): 176 cells, 20 pairs each, eight overhead gates fail despite all six primary gates passing. [Latest decision](evidence/candidate-scan-prepared-isolation/decision.json): retention false. | Failure to establish the budget is not proof of a nonzero slowdown. Later calibration and two diagnostic targets do not repair the historical qualification. |
| R5 — native API/lifecycle coverage | Passed within stated coverage | [Boundary native summary](evidence/boundary-experiment/native-summary.json): 120 GL/Vulkan processes at observed SDL drawable/window ratio 1. [Correctness verification](verification.md#final-native-matrix-and-checks): Qt 6.12.0 both APIs and Qt 6.8.0 OpenGL. | Isolated SDL success does not qualify a Qt/Skia bridge or erase Qt lifecycle failures. |
| R5 — minimum-Qt Vulkan lifecycle | Failed | [Correctness support](evidence/correctness/support.json), [lifecycle reduction](verification.md#vulkan-lifecycle-reduction): Qt 6.8.0 empty windows fail default/basic/live-release variants; three recreations each. Qt 6.12.0 passes those reductions. | Ownership/cause unresolved; no approved minimum-version relaxation or corrected integration path. |
| R5 — required comparable scale matrix and transitions | Unavailable | [Scale facts](evidence/performance-experiment/scale.json) and [boundary support](evidence/boundary-experiment/support.json): native ratio 1 only; historical 1.25/1.5/2 ratio cases unavailable. | Desktop scale is 1.25, not 1. No full equal-size Qt/Skia matrix at required scales or qualified cross-output scale transitions. |
| R6 — full-document composition | Untested | [Selection/composition limits](verification.md#selection-and-composition-limits), [correctness support](evidence/correctness/support.json): document fixture uses separated table rows; selection is within blocks. | General tables and cross-block drag selection remain unsupported. Paragraph, overhang, object and replay passes do not close this gate. |
| R6 — owner visual acceptance | Passed within stated coverage | [Performance review scope](evidence/performance-experiment/README.md): owner's visual review accepted only for inspected cases. | Blanket temporal/quality acceptance across fixtures, APIs, outputs and scales is untested. Earlier `ownerVisual: pending` describes the earlier boundary stage, not a contradiction or blanket later approval. |
| R7 — actual presentation pacing | Failed | [Native scenarios](evidence/performance-experiment/native-scenarios.md), [post-reboot retry](evidence/performance-experiment/recovery/native-retry-summary.json): refresh 16.6675 ms; retry p95 16.799 ms, maximum 16.824 ms, target false. | Strict p95 <= one refresh / maximum <= two remains unchanged. Four original missing-feedback cases remain gaps; GL interval-1 and Vulkan mailbox are not equal scheduling. |
| R7 — CPU/GPU and memory qualification | Unavailable | [CPU summary](evidence/performance-experiment/cpu-summary.json) and [performance report](evidence/performance-experiment/README.md): CPU/layout/paint, GL Ganesh GPU elapsed, process RSS and GPU-cache bytes measured on shared workstation. | Vulkan GPU elapsed unavailable; RSS is whole-process, GPU cache is not total GPU memory. Partial/time-limited workloads and unavailable historical environment facts preclude complete qualification; no new memory threshold is inferred. |
| R8 — dependencies, maintenance, integration and selection | Untested | [Boundary maintenance](demonstrator/boundary-experiment/README.md), [recovery provenance](evidence/performance-experiment/README.md#recovery-completion--2026-11-06): pinned private SkParagraph patch, Skia/font/Unicode/shaping dependencies and experimental SDL/upstream window helpers. | Pin updates need patch/rebase/ABI and full regression review. Shipping build/license/package/resource budgets and Qt bridge ownership/lifecycle are not qualified. R3/R5/R6/R7 gaps forbid selection or migration. |

### Candidate disposition and reconciled findings

| Option | Current disposition | Decision cost / unmet prerequisite |
| --- | --- | --- |
| Public Qt distance-field or curve composition | Keep as unproven alternatives; direct adjusted-run adapter rejected. | Must demonstrate native padding/objects and source-preserving composition through public APIs. Hybrid raster captures cannot supply that proof. Existing Qt cache/lifecycle ownership reduces new dependency work but does not resolve the minimum-Qt Vulkan failure. |
| Patched native Skia | Feasible isolated paragraph mechanism; not selected. | Requires comparable scales, complete composition, lifecycle/integration, broader visual acceptance, pacing and resource qualification; carries private upstream patch and dependency maintenance. |
| Continue existing renderer | Current operational disposition while evaluation remains incomplete. | Preserves shipping behavior and host styling without a speculative migration. Historical blur/sampling/performance concerns remain; continued use is not acceptance as the 2.x replacement. |

The 2026-11-06 scale correction supersedes the interpretation of earlier “DPR 1”
labels, not their raw observations. At desktop 1.25 the 800×600 buffer/window maps
through a 640×480 viewport: 1.25 pixels per surface unit, one document unit per
buffer pixel at zoom 1. A 22-unit font is 22 buffer pixels, not the 27.5 pixels of
22 surface units. No explicit buffer-scale request was observed; compositor
sampling is unknown. Comparisons among those native variants remain useful;
Qt/Skia size equivalence and the four-scale gate remain open.

[Path isolation](evidence/candidate-scan-prepared-isolation/report.md) completed
2,001 processes and resolved all six contemporaneous controls. Dense metadata's
I−C interval is [-63191.75, -24420.0] ns/paragraph; prepared prose's is
[-700.3125, 921.578125]. Both residual envelopes fit, but only dense meets the
strict improvement criterion. This is **mixed diagnostic support**, not scanner
retention, equivalence, absence of a prose effect or stack-only causal attribution.
No further scanner study follows merely from the favorable dense result.

The old whole-paragraph padding and object failures belong to the reservation
adapter; B1–B8 explicitly revised cross-span shaping and passed the new mechanism.
Conversely, native feasibility cannot repair minimum-Qt Vulkan shutdown, missing
presentation feedback, full-document gaps or failed pacing. The post-reboot retry
is a separate cohort, not a replacement for original failures. Historical source
loss listed in the [availability record](evidence/performance-experiment/recovery/timed-source-availability.json)
remains a provenance limitation. Frozen sources, including the archived
`tool/test_isolation.py`, take precedence for identity-sensitive review; the
[cleanup record](evidence/candidate-scan-prepared-isolation/source-cleanup.json)
does not authorize replay with the whitespace-cleaned working copy.

### Coordinated scale study — approved 2026-11-09

The owner approved implementation and execution of the [bounded four-scale
study](scale-study-protocol.md), replacing the earlier design-only proposal.
The [corrected study](evidence/scale-study-v2/README.md) completed all four fresh
sets: 544 passes, 120 Qt 6.8 Vulkan shutdown failures and 16 dependent recreations
skipped. Qt 6.8 OpenGL, all Qt 6.12 configurations and both Skia APIs pass their
fresh correctness checks. Equal-size comparability fails above 100%: Qt nominal
font height follows desktop scale (27.5/33/44 pixels), while SDL/Skia stays at 22.

An approved atomic-handoff repair and ten-window replacement completed the live
sequence: seventy semantic/coordinate/containment checks and thirty repeated-state
comparisons pass, with eight clean shutdowns and two Qt 6.8 Vulkan shutdown failures.
The two earlier harness failures remain diagnostic evidence. Total actual launches
are 685 within the 691 ceiling; original scale 125% is restored and verified.

Retained SDL live mappings differ from fresh mappings at lower scales; Qt retains
its initial buffer resolution. Consequently R2/R5 comparability remains failed,
while the named source/coordinate/repeatability subgates pass within coverage.
Attribution to compositor/toolkit/adapter behavior needs bounded diagnosis, and the
minimum-Qt Vulkan lifecycle failure remains unresolved. No backend is selected or
production qualification established. The [consolidated study result](scale-study-results.md)
defines these limits and the next prerequisites. Historical R1–R8 dispositions
above remain the cutoff record.

## Historical decision record — superseded status summary

The following opening assessment predates native implementation. Its statement
that Skia was unimplemented is historical only; use the current decision above.
Subsequent dated sections preserve the evolution of constraints and approvals.


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
