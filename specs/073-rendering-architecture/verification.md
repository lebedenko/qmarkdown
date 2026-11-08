# Actual verification — 2026-11-05

**Partial evaluation; no architecture selected.** This work establishes the
coordinate audit and a narrow public Qt adapter blocker. It does not complete
the owner's requested Skia comparison, authoritative Markdown demonstrator or
migration specification.

## Implemented and measured

- Standalone actual-source distance-field and curve controls build in Release on
  exact Qt 6.8.0 and 6.12.0. No shipping source/API/dependency changes.
- The nonuniform placement probe models code gutters and object advance in copied
  glyph runs. Both raster controls differ; rebuilding the original QTextLayout
  produces exact before/after images. This confirms copied positions do not feed
  back into QSGTextNode. It does not prove every alternative Qt design impossible.
- The independent runner checks observed Wayland/API/DPR/version and imposes a
  20-second process deadline, retaining flushed stage records and failures.
- Actual transform motion runs after captures. Swap notification samples are
  retained but are not presentation, CPU/GPU timing or performance acceptance.

| Exact SDK | OpenGL, both renderers × four DPRs | Vulkan, both renderers × four DPRs |
| --- | --- | --- |
| 6.12.0 | 8/8 placement/replay runs exit successfully | 8/8 exit successfully |
| 6.8.0 | 8/8 exit successfully | 8/8 record placement result then crash after motion sample; **failed** |

Observed DPRs: 1, 1.25, 1.5, 2. Observed screen refresh: 59.997 Hz. See
[development summary](evidence/development/summary.json),
[minimum summary](evidence/minimum/summary.json), and per-case PNG/JSONL siblings.
Binary/source/font hashes are included. Executable sizes at measurement: 73,160
bytes (development), 72,944 bytes (minimum); shared Qt/driver size is excluded.
Only the primary DejaVu Sans font is pinned; fallback font identity is not yet
qualified. QSG logging produced empty stderr in these runs, so no device details
are inferred from it. The separate [hardware record](evidence/hardware.txt) records
the system GPU; it is not proof of the render device chosen by each process.

The [bounded debugger reproduction](evidence/minimum-vulkan-backtrace.log) stops
on SIGSEGV in `QSGRenderThread`, inside NVIDIA's `libnvidia-glcore.so.615.78.08`,
called through the minimum SDK's QtGui/QtQuick libraries. This locates the crash;
it does not establish driver versus Qt versus harness root cause. It occurs
before the process's `exit` stage. No failure is waived or hidden by forcing a
different backend. The earlier feature-065 stall remains a separate unresolved
incident.

The development curve DPR-1.25 actual-source screenshot was visually inspected:
complex-script/fallback text and the deliberately unreserved object position are
visible. That limited inspection is not quality approval. Full visual/motion
review remains pending.

## Failed preliminary attempts

Sandbox native launch failed before application initialization (exit -6); the
native matrix therefore required compositor/GPU access outside the sandbox.
Initial scale requests produced compositor-multiplied DPRs (1.25, 1.5625, 1.875,
2.5) and were rejected. Final runs explicitly account for compositor scale 1.25
and check observed DPR. Preliminary runs under `build-ci/073-development-*` are
not qualification evidence; only the `*-placement` runs are retained above.

## Unreached work and limits

Native Skia is not installed in inspected project/system paths and was not built.
No browser reference, padded full-document adapter, table/selection/object
renderer, flick/resize/scale-transition suite, captured-motion exact replay,
presentation feedback, CPU/GPU profiling or memory bounds have been implemented
in this revision. The interactive mode is a real Qt placement control, not the
requested authoritative full-document demonstrator. The [migration record](migration.md)
explicitly leaves unresolved decisions open. No production rewrite is authorized
by these results. Historical feature-065 files were left unchanged by this work.

Repository lightweight validation passed: `python3 scripts/check.py`, including
87 Qt-independent tests, document links and configuration/evidence syntax.
Python compilation and `git diff --check` passed. Shipping runtime/packaging
suites were not repeated because this change is a standalone experiment.
No commit, PR, merge or publication was performed.

## Demonstrator continuation — actual results

The runnable [demonstrator](demonstrator/README.md) now renders actual padded code,
objects, formatted text and measured document rows. The native SDL2/Skia control
uses independent SkParagraph shaping and both Ganesh APIs. This is usable
comparison tooling with explicit limitations, **not completed visual acceptance or
performance qualification**. The earlier “Unreached work” section describes the
previous placement-only revision and is preserved as historical evidence.

### Native matrix

| Control | OpenGL | Vulkan | Observed window DPR |
| --- | --- | --- | --- |
| Qt 6.12.0, distance + curve, 12 fixtures | 96/96 pass | 96/96 pass | 1, 1.25, 1.5, 2 |
| Qt 6.8.0, distance + curve, 12 fixtures | 96/96 pass | 96/96 crash at shutdown | 1, 1.25, 1.5, 2 |
| SDL2/Skia, 12 fixtures × 3 window cycles | 36/36 pass | 36/36 pass | 1 |

“Pass” means observed identity, captures, exact same-renderer pose replay,
clipping/settling, resize and zoom/reset replay, unchanged hashed inputs and clean
exit. It does not mean visual or composition acceptance. Qt scale multipliers
change the observed window DPR on the existing 1.25 compositor configuration;
they do not demonstrate changing the compositor's own configuration. SDL exposes
DPR 1 here. Its 1.25/1.5/2 cases are **unavailable**, not replaced by document zoom.

Both APIs identify NVIDIA GeForce GTX 1660 SUPER. Qt reports screen refresh
59.997 Hz; SDL reports 60 Hz. These are screen metadata, not presentation timings.
The initial Qt development sweep had one source-change rejection while Skia
implementation was ongoing; the final unchanged-input sweep passes all 192 cases.

Readable summaries:
[Qt development](evidence/continuation/qt-development.json),
[Qt minimum](evidence/continuation/qt-minimum.json),
[Skia](evidence/continuation/skia.json).
[Archive inventory](evidence/continuation/archives.json) contains hashes/sizes for
retained archives with every capture, pre/post identity record and raw log.
Representative actual-source captures:
[Qt distance document](evidence/continuation/qt-distance-document.png),
[Qt curve fractional pose](evidence/continuation/qt-curve-fractional-pose.png),
[Skia GL](evidence/continuation/skia-opengl-document.png),
[Skia Vulkan](evidence/continuation/skia-vulkan-document.png).
These were visually inspected for visible text, padding, objects and block layout.
Owner inspection and temporal quality acceptance remain pending.

### Crash isolation, error paths and lifecycle

All six Qt reductions (empty, rectangle, static text, capture, motion, combined)
were exercised on both exact SDKs with application quit and programmatic window
close. Minimum Qt fails even for an empty window on both paths. Development Qt
passes all reductions on both paths. This rules out text preparation, capture and
motion as necessary preconditions; it does not establish driver fault. A true
owner-operated window-manager close remains part of manual review.

Additional three-cycle document recreations pass on both development APIs and
minimum OpenGL; minimum Vulkan fails on all six renderer/cycle combinations.
Real image-write failures exit 4 both during capture and before the event loop.
See [lifecycle results](evidence/continuation/qt-lifecycle.json) and the retained
[lifecycle command](evidence/continuation/lifecycle-command.py).

The new Qt harness uses observed exposure/rendered frames, scoped window callback
receivers, stopped timers and explicit callback disconnection before destruction.
The original placement-only probe remains historical, including its fixed delays;
it is not the continuation's evidence runner.

Skia integration failures were reproduced and fixed in isolated harness code:
SDL-resolved GL functions replace the incompatible native loader; library ABI
defines are imported into the helper build; explicit fonts use the asset manager;
the Wayland client supplies an initial extent; the present queue is initialized
before failed-swapchain cleanup; and sRGB VkFormat maps to Skia's sRGB color type.
The retained [initialization backtrace](evidence/continuation/skia-initialization-backtrace.log)
and before-fix archives document these changes. A driver frame alone was not used
to assign fault.

### Selection and composition limits

The geometry executables reuse each actual viewer implementation without a GPU
window. Both Qt SDKs pass full-source geometry and object edge/source-copy checks.
One formatted-selection check fails on both: the narrow bidi/object fixture's
reported code edges copy “ode” instead of “code”. That fixture is **unsupported
for Qt code-edge selection**; no shipping fix is implied.
See [development geometry](evidence/continuation/qt-selection-development.jsonl)
and [minimum geometry](evidence/continuation/qt-selection-minimum.jsonl).

Skia passes source-map monotonicity, formatted-range edge checks and object
edge/source-copy checks, including reversed visual edges in bidi. See
[Skia geometry](evidence/continuation/skia-selection.jsonl).
Inserted reservations still do not implement padding on every wrapped code
fragment. The wrapping and narrow bidi/object fixtures are **unsupported for
per-line padding**; retain [this capture](evidence/continuation/skia-wrapping-unsupported.png).
Other reservation cases remain unqualified, rather than silently making code
atomic or substituting Qt layout.

The [per-fixture support matrix](evidence/continuation/support.json) records these
statuses. Cross-paragraph drag selection and general table layout remain
unsupported; the document contains table rows with explicit separators.
Interactive slow movement, flick-like movement, reversal, reset, resize and zoom
are implemented. The separate motion matrix below covers sampled motion; owner
temporal quality review remains pending.

### Costs and verification

Skia pin: 00227028b0c3cb3052432a456af2027a49e598b2. Exact fetched dependency
revisions, system dependency versions, GN identity, build arguments and library
hashes are in [the build record](evidence/continuation/skia-build.json).
The library build took 459.572 seconds with two workers. The external checkout
including outputs occupied approximately 423 MiB, of which outputs were 56 MiB.
System shared libraries/toolchains and font files are additional costs.
Executable sizes: Qt development 127,728 bytes; minimum 123,360 bytes; Skia
11,524,464 bytes. Qt additionally links the isolated prepared library and SDK.

Preparation observations (median / maximum milliseconds across initial and
resize/zoom preparations): Qt development 0.207 / 4.308; minimum 0.199 / 3.391;
Skia 0.091 / 4.298. These are descriptive samples, not performance qualification.
For the document, warmed translated Qt content retains five native text nodes;
three fallback blocks account for six raster paints/uploads across initial and
translated poses. Counters are per current layout generation and reset on
repreparation; shutdown values are not lifetime totals. Skia retains paragraphs
and its GL surface during translation; resize/zoom sequence reports four surface
wraps and nine document paragraph resources. CPU/GPU allocation totals and
presentation pacing are not measured.

The supervisor/fixture suite passes nine tests, including timeout, crash,
malformed/truncated logs, wrong backend/DPR, failure status and source-span
validation. Selection executable status is deliberately failing for Qt's retained
bidi/object limitation; Skia selection checks pass. Adding these separate test
targets left all three viewer binary hashes unchanged. The later opt-in motion
branch was verified separately as described below; pre-motion source snapshots
are retained beside the original matrix archives.

No shipping source/API/dependency changes, merge, publication or backend selection
were performed. Owner visual review and the remaining qualification work are
required before calling the milestone accepted.


### Rendered-frame motion verification

The final motion runner executes 20 slow, 20 flick-like and 20 reversed rendered
steps, captures the sampled pose, and requires exact same-renderer replay.
All 102 cases record 60 steps and successful captured-pose replay:
48 development Qt, 48 minimum Qt, and 6 Skia cases. The three fixtures are padded
code, objects and the document. Qt covers both native APIs, both renderer choices
and DPR 1/1.25/1.5/2; SDL uses its observed DPR 1.

Process results: 78 clean passes and 24 minimum-Qt Vulkan shutdown crashes.
No new backend failure or input-change rejection occurred. Layout preparation
does not run between the first and last motion steps. Native resource retention
and fallback repaint/upload counters remain distinct. Skia's interactive speed is
time-based, so mailbox throughput does not determine movement speed.
These rendered-step samples are not compositor presentation feedback.

See [motion summary](evidence/continuation/motion.json) and
[full motion evidence](evidence/continuation/motion.tar.gz). This opt-in extension
does not retroactively relabel the earlier static/pose matrix as motion evidence.

Final lightweight verification passed: repository documentation/configuration
checks and 87 Qt-independent tests; all nine supervisor/manifest tests; tracked
whitespace validation and explicit whitespace checks on all 83 feature-073 text
files (including untracked additions). The no-index whitespace wrapper initially
misclassified Git's ordinary difference exit code as an error; checking its
diagnostic output correctly reports zero whitespace errors.

[Current binary identities](evidence/continuation/current-binaries.json) record
the motion-capable viewers: development Qt 127,776 bytes, minimum Qt 127,504 bytes,
Skia 11,528,624 bytes. The Qt curve/OpenGL document viewer was opened for owner
review and reached observed readiness on Qt 6.12.0 at DPR 1.25. The generated
self-contained browser reference is available at /tmp/073-browser-reference.html
and can be regenerated with the documented command. Owner acceptance is pending.

## Bounded correctness continuation — 2026-11-05

The owner explicitly approved C1–C6, the updated design and mapped tasks before
implementation. Changes remain under Feature 073; shipping source, APIs,
dependencies, versions, releases, drivers and backend choices are unchanged.
Historical evidence above remains valid for its recorded inputs.

### Qt geometry and selection

Fresh baseline runs reproduce `bidi-objects` copying `ode` on both exact SDKs.
The retained cursor traces identify source line start 22, but no prepared cell
for source 22 (`c`); cells start at 23. Qt's two cursor affinities collapse to the
same x at this wrapped bidi boundary, while the same shaped line's glyph-run
bounds provide the missing nonzero-width cell. This distinguishes missing
geometry from an ambiguous expected string.

The stage-only [patch](demonstrator/prepared-cursor.patch) recovers such a cell
from the public shaped glyph run, preserving visual direction. It does not use
fixture strings, unshaped font advances or a test-only mapping. The original
source commit remains `43d6bba842bb5b7ab23889e49d5f138c6f22a7fb`. Staging checks
the patch before applying it and records its SHA-256 plus output hashes.
Two fresh stages have identical output identities; reapplying the patch fails
as expected. See [stage checks](evidence/correctness/stage-checks.json),
[original stage](evidence/correctness/stage-before.json) and
[patched stage](evidence/correctness/stage-after.json).

The initial native selection capture exposed a separate harness defect:
reversed drag endpoints copied correctly but yielded no painted selection,
because the preparer expects an ordered logical interval. Interactive selection
and automated capture now share endpoint normalization. Both selection directions
require exact copied source and nonempty selection geometry. The native verifier
also rejects selected captures identical to the unselected reset capture.
The interrupted run is explicitly **not accepted**; its
[record](evidence/correctness/interrupted-native.json),
[before image](evidence/correctness/qt-before-reverse-selection.png),
[after image](evidence/correctness/qt-after-reverse-selection.png) and
[source snapshot](evidence/correctness/qt-before-ordered-selection.cpp) remain.

The dense geometry scan covers 751 fixture/width combinations per SDK: integer
widths 60–240 for adjacent spans, wrapping, bidi/objects and the new combined
marks/surrogate-pair/bidi/code/object fixture, plus each other fixture's original
width and ±1. It brackets wrap changes within one logical pixel. Both Qt 6.8.0
and 6.12.0 pass 944 formatted-range checks, 1,888 directional copy checks and
368 object checks. Cursor traces preserve line membership, visual direction,
prepared cells, raw cursor affinities and hit-test results. Actual commands,
binaries, staged libraries, patch, manifests and font identities are in the
[geometry summary](evidence/correctness/geometry-lines.json) and
[full geometry archive](evidence/correctness/geometry-lines.tar.gz).

### Skia reservation trial

The existing pinned SkParagraph layout remains authoritative. The opt-in geometry
trial recomputes source-indexed code-fragment cuts after whole-paragraph layout,
inserts paired padding reservations at new fragment boundaries and repeats until
the cut set stabilizes. Seen-state detection rejects cycles; an eight-pass limit
rejects non-convergence. The scan stabilizes within three passes, but **stability
is not acceptance**. Painting, selection and hit testing retain one final layout;
the rejected trial is not enabled in the native viewer.

The [single-paragraph reproducer](evidence/correctness/skia-minimal.json) shows
why the approach fails: reservations can remain on the preceding line, leaving
later visual code fragments without left padding. At width 190, cuts stabilize
at UTF-16 offsets 12, 25 and 40, yet fragment checks still fail. Tests require
actual padding placeholders, not adjacent ordinary text, on fragment boundaries.
Both original and trial layouts pass exact forward/reverse source-copy checks;
that does not qualify padding, shaping or bidi behavior. No synthetic spaces,
atomic code spans, dependency change or alternate layout engine is adopted.

Expanded object checks additionally observe a fractional top overhang: for
example `objects@679` places the placeholder top at approximately -0.19 with
bottom 35.81 and paragraph height 36. This fails the explicit 0.05 logical-pixel
containment tolerance while preserving its full source label and nominal height.
The next capability is reliable per-fragment horizontal/vertical reservation
and established placeholder baseline/bounds semantics in the authoritative
layout. Full shaping/bidi/wrapping acceptance remains open after these failures.

### Vulkan lifecycle reduction

The existing empty-window case was tested first, followed by a standalone
[empty-window executable](demonstrator/lifecycle.cpp) without text, capture or
motion. Default and basic render loops each exercise ordinary shutdown and
explicit hide/release with nonpersistent scene-graph/graphics resources while
the event loop remains active. Each combination is recreated three times.

All 18 minimum-Qt processes terminate with SIGSEGV; all 18 development-Qt
processes exit cleanly. Ready records verify Vulkan, Wayland and observed DPR 1.
Callbacks record scene-graph initialization, stopping/invalidation, shutdown,
event-loop return and window destruction where execution reaches them. The
minimum threaded live-release case reaches scene-graph invalidation during hide
and crashes before the later release call; the basic variant reaches release.
The faulting thread changes with the render loop. Bounded debugger stacks include
NVIDIA library frames; this does not by itself establish driver fault.
No diagnostic variant qualifies as a harness workaround.

See the [lifecycle summary](evidence/correctness/lifecycle-native.json) and
[full callbacks/backtraces](evidence/correctness/lifecycle-native.tar.gz).
The initial sandbox attempt could not connect to Wayland and is retained
separately as [environment-denied evidence](evidence/correctness/lifecycle-sandbox-denied.tar.gz);
it is not counted as a backend test. Native runs used authorized GPU access.


### Final native matrix and checks

The complete serial matrix ran 1,156 processes: 544 development Qt, 544 minimum
Qt and 68 Skia. All 544 development cases, all 272 minimum OpenGL cases and all
68 Skia cases exit cleanly. All 272 minimum Vulkan cases fail with SIGSEGV during
shutdown; no other process error or input-change rejection occurs. All cases
reach stationary, selected, resize/reset and pose/replay captures, and report
exact selected source. The selected images differ from the unselected images.
These observations do not turn a shutdown crash into a process pass.

Qt covers both renderers, OpenGL/Vulkan and observed DPR 1/1.25/1.5/2. Seven
affected fixtures each receive three ordinary process recreations plus a separate
motion process; all other fixtures run once per configuration. The 238 motion
processes complete 60 rendered slow/flick/reversal steps and exact captured-pose
replay. Skia observes DPR 1 only; 1.25/1.5/2 remain unavailable. Its original
reservation layout runs as a diagnostic control despite failed geometry; the
rejected trial is never enabled in GPU runs. Total supervised process time is
approximately 24.1 minutes, not a performance-qualification measurement.

See [native summary](evidence/correctness/native.json),
[full captures and input identities](evidence/correctness/native.tar.gz), and
[current viewer binary identities](evidence/correctness/current-binaries.json).
The representative [before](evidence/correctness/qt-before-bidi-cursor.png) and
[after](evidence/correctness/qt-after-bidi-cursor.png) stills show the `c` included
in the corrected code background. Automated replay and inspection of these stills
do not establish owner temporal visual acceptance.

A final test-only refinement groups Skia fragments by authoritative line bounds
and visual adjacency, rather than font-dependent ink centers. It preserves
separate bidi fragments and avoids splitting one line solely because of fallback
font metrics. The earlier checker evidence remains in
[its archive](evidence/correctness/geometry-final.tar.gz). The final scan retains
1,785 original-layout padding failures versus 1,414 trial padding failures, plus
60 object-containment failures in each. All exact directional source checks pass;
no cycles/non-convergence are observed. The final geometry executables return 0
for both Qt SDKs and 1 for both Skia modes. Viewer binaries remain byte-identical
after this test-only refinement, so the native matrix remains applicable.

The pinned Skia checkout is clean at
`00227028b0c3cb3052432a456af2027a49e598b2`; current archive/build-record hashes are
retained in [library identities](evidence/correctness/skia-library-identities.json).
The existing [dependency build record](evidence/continuation/skia-build.json)
remains the dependency/provenance reference. No Qt SDK or system driver changed.

Supervisor tests pass (9 tests); repository lightweight checks pass (87 tests,
document links and configuration/evidence syntax); tracked and new-text whitespace
checks pass. These are standalone experiment checks, not shipping/runtime or
performance qualification. See [supervisor log](evidence/correctness/supervisor-tests.log)
and [repository check log](evidence/correctness/repository-check.log).
Owner visual acceptance, merge and publication remain separate gates.

Evidence packaging preserves all 15,267 native files. Identical files are stored
as tar hardlinks; safe extraction was checked against every original file's
SHA-256. See [archive round-trip check](evidence/correctness/native-archive-check.json),
[archive hashes](evidence/correctness/archives.json) and
[evidence index](evidence/correctness/README.md).

## Fragment-aware experiment planning — 2026-11-05

Updated requirements (F1–F7), design and mapped tasks for the isolated upstream
patch experiment. Explicit approval is pending as required by the supplied plan;
no patch, adapter change, staged build or new geometry/native evidence exists yet.
Historical failures and control artifacts are preserved.

Planning validation: `python3 scripts/check.py` passed (documentation links,
configuration/evidence syntax and Qt-independent tooling tests);
`git diff --check` passed. These checks validate the planning change only and do
not satisfy any proposed feasibility gate. The final verification-record append
was additionally checked with `python3 scripts/check-docs.py` and whitespace
validation.

## Fragment-aware upstream experiment — actual result, 2026-11-05

Owner approval of F1–F7, design and tasks was received as "approved" before
implementation. **Outcome: blocked**, using the approved design's explicit stop
condition for a span edge inside an indivisible shaped cluster.

### Isolated build and minimal blocker

Copied pinned revision `00227028b0c3cb3052432a456af2027a49e598b2` into a separate
stage; applied the hash-pinned 40-line patch to two private SkParagraph files.
Built independent patched/unpatched CPU adapters with the original GN/ABI
configuration and two workers. Both builds succeeded. The original libraries,
build configuration and existing Skia viewer/selection executables retained their
identities. No shipping or Qt-control source changed.

The [minimal manifest](demonstrator/fragment-experiment/minimal.json) uses Arabic
lam–alef with two adjacent code spans. UTF-16 offset 1 is a valid ICU grapheme
boundary mapping to UTF-8 byte 2. The pinned shaper produces glyph 3254, cluster
bytes `[0,4)`, advance 13.2451171875. Both identities report the same interior
edge. The patched preflight exits 1 with `blocked-intracluster-edge`. A whole-span
control produces identical glyphs and positions with no interior edge. Separately
shaped characters produce glyphs 1141 and 1117, demonstrating why reshaping would
violate this milestone. No line-fitting or paint-only workaround was introduced.

### Automated evidence and gate limits

The disabled-padding control comparison passes all 751 existing fixture widths,
775 blocks, with exact JSON equality of glyph IDs/fonts/positions, cluster ranges,
line ranges/metrics, object bounds, source maps, range rectangles and sampled
caret hit results. This establishes equivalence of the **preflight patch only**,
not of a fragment-aware formatter. Zero-padding metadata introduces no reported
boundary issues. Seven malformed-range cases reject noninteger, negative,
out-of-range, overlapping, grapheme-splitting and partial-object intervals.

The approved intact-cluster contract fails before padded candidate fit. No padded
751-case geometry pass, fractional transition scan, directional-copy consistency
pass or GL/Vulkan native run is claimed. Native stationary/selected, resize/reset,
motion/replay and recreation checks were gated off. Historical observed Skia DPR
remains 1; 1.25/1.5/2 remain unavailable. Qt 6.8 Vulkan shutdown remains independent.

The unchanged historical adapter reproduces `objects@679`: baseline 28, raw line
height 36.1884765625, rounded height 36, vertical delta −0.1884765625, and object
path bounds top −0.19 / bottom 35.81. `InternalLineMetrics::height`, `delta` and
`runTop` explain the adjustment. An isolated raster draw with the same round-rect
paint covers DPR-1 rows 0–35, but the logical top still violates the existing 0.05
tolerance. Disabled padding has no overhang in this scan; this is not evidence of
a corrected padded vertical reservation.

[Build identities and commands](evidence/fragment-experiment/build.json),
[regression summary/input-output hashes](evidence/fragment-experiment/summary.json),
[historical object trace](evidence/fragment-experiment/historical-objects.jsonl),
and [support status](evidence/fragment-experiment/support.json) retain the results.
The [offline review launcher](demonstrator/fragment-experiment/README.md) displays
CPU images of unchanged joined shaping and separately shaped counterfactuals;
they are not successful before/after padding images. The joined image was visually
inspected for capture integrity; owner visual acceptance remains pending.

The diagnostic patch adds no data members or shipping API, but uses private
SkParagraph internals. Rebase and cluster-semantics checks are required on a new
pin. Full layout-patch maintenance and performance costs remain unmeasured.

Regression checks pass, including byte-identical CPU PNGs for the unpatched and
patched minimal cases. Supervisor tests pass (9 tests); repository lightweight
checks pass (87 tests, document links and evidence/configuration syntax). A final
patch whitespace normalization was checked by rebuilding a fresh stage: patched
source hashes match the preceding build exactly, and the regression suite passes
again. Final documentation/evidence syntax and tracked/new-file whitespace were
checked after evidence collection. Logs are retained alongside the summary.

## Explicit shaping-boundary continuation planning — 2026-11-05

Prepared B1–B8, revised design and mapped tasks for explicit code-span shaping
boundaries. These proposed semantics supersede the cross-span glyph-preservation
constraint for a new isolated stage; historical commit `d3b1f05`, its diagnostic
patch and failure evidence are unchanged. Explicit approval of these revisions
is pending, as required by the supplied continuation plan. No new stage, patch,
layout implementation, geometry result or native run is claimed.

Planning validation: `python3 scripts/check.py` passed documentation links,
configuration/evidence syntax and all 87 Qt-independent tests. `git diff --check`
passed. These checks validate the specification change only; they do not satisfy
the planned boundary, equivalence, geometry or native feasibility gates.

## Explicit shaping-boundary continuation — actual result, 2026-11-05

Owner approval received as "approved" for B1–B8, revised design and mapped tasks
before implementation. **Outcome: feasible for the tested isolated slice.**
This supersedes the earlier cross-span shaping constraint for the new experiment
only; commit `d3b1f05`, its diagnostic patch and all failures remain intact.

A checked five-file patch splits upstream shaping/context at code edges while
retaining paragraph bidi, script/fallback and Unicode breaks. Adjacent lam–alef
uses glyphs 1141/1117 in separate clusters; the whole-span control retains 3254.
The adapter validates UTF-16 grapheme/object intervals and maps them to UTF-8 with
no padding inserts. Metadata-bearing paragraphs bypass cache reads/writes;
metadata replacement/removal invalidates derived state and width-only layouts
reuse shaped runs. Identical round-trip geometry is checked.

A shared visual-fragment calculation drives candidate fitting and formatting.
Finite candidate scans bound the existing wrapper by cluster endpoint and
preserve original grapheme boundaries, upstream word stretches and trimming.
Visual traversal supplies the same translated positions to painting, rectangles,
carets, selection and objects. Baseline/height reservation is integral before
geometry publication; containment tolerance remains 0.05 pixels.

Actual gates passed:

- Exact metadata-free control equivalence: 751 fixture widths / 775 blocks.
- Separate boundary-enabled zero-padding baseline, Latin/Arabic ligatures and
  joining, adjacent identities, bidi, marks, surrogates, fallback, object edges,
  oversized units, zero width, hard breaks and whitespace: 14 added regressions.
- Nine invalid input/range cases rejected, including malformed surrogate input,
  grapheme splits, overlap and partial-object intervals. Unicode break flags
  match the unpatched control; padded shaped runs match the new zero-pad baseline.
- All 751 padded cases pass; 41 observed transition brackets refined to 1/64
  logical pixel and 147 below/at/above block probes pass. Interior hit tests and
  exact source copying in both selection directions pass. Zero-width newline
  boxes have no hittable interior; original source mapping retains them.
- Extent audit: 14,272 code cells and 2,901 visual fragments pass. Near-edge
  snapping remains diagnosed separately (95 probe differences); no containment
  tolerance was changed. Ghost whitespace retains upstream unpainted semantics.
- `objects@679`: baseline 31, line height 42, delta 0, logical object top/bottom
  3/39 and DPR-1 painted vertical bounds 3/39 pass. Raw metrics remain recorded.
- Native: 120 serial processes on OpenGL/Vulkan, including stationary/selected
  captures, resize/reset, 60-frame motion/replay and three normal recreations
  for every fixture/API. Observed DPR 1; 1.25/1.5/2 unavailable. Deadlines and
  input identities retained. Shared CPU/native construction and an interactive
  patched viewer are included. Representative captures were inspected for
  integrity; owner visual acceptance remains pending.
- Supervisor regressions: nine tests pass. Repository lightweight validation:
  documentation/configuration checks and 87 Python tests pass.

The inherited staging routine initially dereferenced a tracked Skia symlink.
Corrected staging preserves relative internal symlinks, rejects external links,
checks a clean copied tree and rejects modifications outside the patch. A fresh
build and CPU rerun passed; all libraries/executables are byte-identical to those
used for the native matrix, so those native checks were not repeated. Exact
reconciliation and original native-input snapshots are retained.

The patch adds 198 lines and removes 12 across private ParagraphImpl,
OneLineShaper, TextWrapper and TextLine internals. Candidate scans may be quadratic;
cache optimization and performance qualification are deferred. Original explicit
font availability/missing glyph limitations remain visible. No shipping dependency,
Qt-control, backend-selection, integration, merge or publication change occurred.
The Qt 6.8 Vulkan shutdown failure remains independent.

See [evidence and hashes](evidence/boundary-experiment/README.md),
[support](evidence/boundary-experiment/support.json), and
[reproduction and viewer](demonstrator/boundary-experiment/README.md).

Final review: normalized whitespace-only context lines in the stored patch, then
applied it to fresh pinned source files and verified byte-identical hashes against
the compiled five-file source set. The tested patch/pin and normalization record
are retained. Final lightweight checks and tracked/new-text-file whitespace checks
passed; this packaging normalization does not change runtime verification inputs.

## Scale/performance continuation — 2026-11-06

The owner explicitly approved implementation of the supplied measurement plan.
Requirements P1–P6, design and mapped tasks were recorded before implementation.
Owner visual review is accepted only for the cases inspected; this supersedes
previous pending-owner-review statements for those cases, without extending the
review to untested configurations. Historical automated results remain unchanged.

Separate serial OpenGL/Vulkan protocol diagnostics establish a 1.25 buffer/surface
mapping despite SDL's 800/800 drawable/window ratio. See the
[corrected scale explanation](evidence/performance-experiment/README.md).
Measurement implementation and runs are in progress; final results follow there.


## Approved recovery — 2026-11-06

The user explicitly instructed implementation of the supplied Feature 073 Recovery plan on 2026-11-06. This approves the recovery requirements, design and tasks recorded above before implementation. Existing dirty files and historical evidence are preserved. Recovery results will be recorded separately from planned checks.

Recovery preparation results (2026-11-06): 7,475 surviving files snapshotted and
all member hashes verified under `build-ci/073-performance-recovery`. Pinned Skia
`00227028b0c3cb3052432a456af2027a49e598b2`, all three dependency revisions and GN
identity were restored. All five patched source hashes, three counter source
hashes and surviving diagnostic executables match historical identities. Six
rebuilt upstream archives are byte-identical to the historical libraries. Original
build records remain unchanged; relocation and rebuilt prerequisites are recorded
in a separate recovery manifest. Original `/tmp` logs remain lost.

The clean prior directory contains 59 validated comparisons and 17 terminal
outcomes, leaving 116 variant/cases. Its interrupted mixed-480-65536 zero-padding
instrumented operation restarted successfully into a new output directory; its
unacknowledged original bytes remain preserved. No completed CPU/native matrix
was rerun. Independent inspection confirms complete measurement windows in all
323 completed original native processes and zero non-resize measured layout calls.
Current font hashes and the six checked system-library versions match available
historical records; facts collected now are explicitly post-reboot observations.

Recovery final result (2026-11-06): **measurement work and reporting complete;
performance is not qualified.** All 192 patched diagnostic variant/cases are
accounted for: 151 exact instrumented/headline comparisons, 23 timeouts and 18
skips. All 59 clean prior comparisons and 17 prior terminal decisions were reused;
116 remaining cases were accounted for without repeating the completed CPU or
native matrices. All 18 upstream counter comparisons, 82 variant source-mapping
comparisons, shaping reuse/invalidation checks and rebuilt control geometry gates
pass. No diagnostic RSS ceiling was reached.

The two serial post-reboot scale traces reproduce all retained protocol facts and
historical SDL fields. The sole native replacement (typical, padded, OpenGL,
settling, repetition 0, attempt 1) completes its full timing window with 300
presented frames, no feedback gap and zero measured layout calls. Its 16.799 ms
presentation p95 still fails the unchanged 16.6675 ms target. It remains a separate
post-reboot cohort; the original failure and four missing-feedback cases remain.
The corrected ten-paragraph, width-480 CPU smoke completes all three variants,
each with ten fresh-process samples and five discarded plus thirty warm samples.

Verification passed: nine recovery tests, eight measurement tests, nine supervisor
regressions, repository documentation/configuration/syntax checks and 87
Qt-independent tests. A full finished-phase resume with measurement/build calls
stubbed to fail passes without invoking either; indexes and comparisons remain
byte-identical. Tracked and private text-file whitespace checks pass. All six
regenerated chart PNGs, the original CPU summary and growth records are
byte-identical to their retained originals. All charts and representative typical
and span-dense captures were visually inspected.

Thirty-seven raw/build/identity archives and all 7,838 members passed independent
hash verification. Recovery records and logs are retained separately. The initial
chart generation failed because the temporary Matplotlib environment was lost;
the pinned tooling-only environment was restored and chart generation passed.
The first identity bundle preparation encountered the absent older adapter source
archive; that partial bundle is preserved and supplemented, not overwritten.
Five superseded v2 source-file byte sequences and original temporary logs remain
explicitly unavailable. Timed CPU source, boundary header, workload generator,
runner, executable identities and compiler/link build records are retained by
hash; unavailable historical source files were not invented or used for new runs.

See the [final performance report](evidence/performance-experiment/README.md),
[coverage audit](evidence/performance-experiment/recovery/diagnostic-summary.json),
[resume validation](evidence/performance-experiment/recovery/resume-validation.json)
and [archive verification](evidence/performance-experiment/recovery/archive-verification.json).
The CPU schema caveat, pacing misses, unavailable Vulkan GPU elapsed timing,
historical failures and independent Qt 6.8 Vulkan shutdown blocker remain. No
production API, dependency, rendering behavior, optimization, backend selection,
Qt integration, merge or publication change occurred.

Commit preparation: staged whitespace validation exposed Matplotlib trailing
spaces in six generated SVG files. Removed trailing spaces only and verified
equivalent normalized XML trees; chart PNGs and measurement evidence are unchanged.
Before/after hashes are retained in [SVG normalization](evidence/performance-experiment/svg-normalization.json).

## Phase diagnosis — planned checks (2026-11-06)

D1–D4 checks: all four operation mappings/geometries match; reflow/repeat reuse
shaping and metadata invalidates; counters reset independently of layout state and
exclude queries; interrupted-phase evidence survives new-directory resume;
historical complex-shaping/object/whitespace/zero-width/wrap gates pass; existing
measurement, recovery and supervisor regressions, task check and git diff --check.
Actual results will be recorded separately. Diagnostic clocks are observations,
not performance qualification or exact CPU-time shares.

## Phase diagnosis — actual results (2026-11-06)

D1–D4 completed in fresh durable workspace `build-ci/073-phase-workspace-v3`.
All 48 serial traces (24 instrumented/uninstrumented pairs, four phases each)
completed under the 60-second and 4 GiB ceilings, with exact paired geometry and
source mapping, width/repeat shaping reuse and metadata invalidation. Every repeat
has zero counted work. Counter reset preserves geometry and shaping generation.

Historical gates pass: 751 fixtures / 775 blocks, exact disabled equivalence and
zero failures; extent audit passes 14,272 vertical cells / 2,901 visual fragments.
Five focused phase/recovery tests, eight measurement tests, nine recovery tests
and nine supervisor tests pass. Completed new-directory resume preserves all raw
hashes and index bytes without remeasurement. Interrupted-phase unit tests preserve
partial records as terminal outcomes. Two preparation-only failures are retained;
neither reached measurements. Rebuilt headline libraries match historical hashes.

See [retained counts and provenance](evidence/phase-experiment/README.md) and the
[optimization proposal awaiting approval](optimization-design.md). Ownership
precomputation is selected as the first proposal; no optimization was implemented.
Historical gaps and commit `4b9fd4c` remain intact. No performance acceptance,
backend selection, merge or publication is claimed.

Repository `task check` and whitespace validation passed. No shipping inputs changed.

## Ownership optimization actual result — 2026-11-06

O-T1–O-T4 are implemented and verified in the isolated ownership experiment.
The owner-approved run revision/table mechanism retains metric-only reset/reflow
state, releases invalidated storage and falls back conservatively for unsplit or
stale entries. All 48 traces / 192 operation records match both peers and archived
baseline geometry/source data; historical boundary and extent gates pass. Existing
phase counters are unchanged. Ownership work totals 78,702,579,266 legacy comparisons
versus 1,756,528 build examinations and zero fallback comparisons. Width/repeat
have zero rebuilds; largest table payload is 9,800 bytes. Zero-padding paths add
preparation work where the baseline had no ownership searches; no benefit is claimed
for those paths. Detailed counts, memory limits, tests, retained preparation failure
and qualification boundaries are in [ownership evidence](evidence/ownership-experiment/README.md).
No speedup or production-performance acceptance is implied.

## Ownership runtime comparison — actual results, 2026-11-06

The owner-approved RT1–RT4 study is complete. Both identical-driver uninstrumented
Release builds passed 48 separate correctness processes (192 full geometry/shaping
records) and historical boundary/extent gates. All 480 timing processes completed:
24 variants × ten adjacent process pairs, alternating baseline/optimized first
by pass. All cells have ten pairs; none are incomplete, failed, limited or retried.
Shaping-reuse and metadata-invalidation guards passed throughout.

Dense-480-4096 padded initial medians are 2.781 s baseline / 0.437 s optimized,
median paired ratio 6.368; mixed is 5.934 s / 0.927 s, ratio 6.425. Single-span
improvements are smaller (long initial paired ratio 1.039; spans-1 1.020).
Zero-padding metadata has measured regressions, including dense-480-4096
+44.46 µs [20.38, 70.32] and mixed +177.54 µs [27.51, 376.53]; many other
zero-padding intervals include zero. All 24 repeat ratios are timer-limited and
suppressed; absolute timings remain. No aggregate speedup or performance threshold
is used. Per-cell medians/differences/ratios and descriptive 95% paired-bootstrap
intervals use 10,000 resamples and fixed seed 73073.

The completed-run resume initially exposed a mutable command-journal checkpoint:
gates appended commands after build checkpointing. Original runner, identities,
checkpoints and failure log are preserved; separate exact build/gate journals and
a focused regression correct this evidence-bookkeeping issue. Driver/CMake code,
binaries, libraries, fonts, fixture/schedule identities and raw measurements did
not change. Corrected new-directory resume validates without launching any sample;
its index and all 528 raw sample files match the completed run exactly.

Final audit checks paired/archived geometry, flags, dependency libraries,
instrumentation exclusion and all identities. Runtime harness tests (11), existing
measurement/recovery (17), supervisor (9), repository lightweight checks (87 tests)
and whitespace checks pass. Historical gates: 751 fixtures / 775 blocks exact,
775 scan blocks with zero failures; 14,272 vertical cells and 2,901 visual fragments
pass extent audit. Max sampled RSS 50,053,120 bytes; max process high-water RSS
90,292,224 bytes, neither an ownership-table allocation measurement.

Recommendation: retain the optimization as measured and target the remaining
candidate-scan/layout cost through a separate approved design, retaining the
zero-padding regressions as a constraint. Ten pairs on one non-isolated host give
descriptive uncertainty only; filesystem caches/frequency/background load are
uncontrolled. No independent warm-cache/native/Qt/backend qualification, further
optimization, merge or publication occurred. Earlier ownership diagnostics and
prior uncommitted work remain intact.

See [runtime evidence and limitations](evidence/runtime-comparison/README.md),
[all per-operation results](evidence/runtime-comparison/report.md), and
[verified archive manifests](evidence/runtime-comparison/archives.json).

## Candidate-scan proof actual result — 2026-11-06

C-T1–C-T5 are complete in the isolated sibling experiment. Final v3 has 72 fresh
serial diagnostic processes / 288 operation records with exact archived-runtime
and oracle geometry/glyph/slice/mapping/shaping equivalence. Shadow proves all
44,946,905 hypothetical skips fail. There are no endpoint/geometry counterexamples.
Initial padded long-span evaluations fall 3,842,986 → 16,852 and spans-32
3,346,678 → 16,468, with unchanged exhaustive preprocessing costs and up to 524,320
bytes of layout-local scratch. There is no speedup or performance acceptance claim.

56,235,842 exhaustive short-sequence checks, 10,000 seed-73074 numeric cases,
129 focused layouts/checks, ownership lifecycle tests, historical boundary/extent
gates and harness/recovery regressions pass. Final completed-run resume validates
all identities/checkpoints/raw comparisons into a new directory without processes.
Historical tolerances/near-edge observations remain unchanged. The preparation
failure and full v2 guard-audit history are retained separately from final v3.

See [actual evidence](evidence/candidate-scan-experiment/README.md),
[experiment/proof](demonstrator/candidate-scan-experiment/README.md) and
[separately approvable recommendation](candidate-scan-recommendation.md).
No retained optimization, runtime matrix, ownership change, Qt integration, merge
or publication is included. Existing zero-padding metadata regressions remain a
constraint for future implementation. Archive, lightweight and whitespace check
results are recorded with the final evidence.

## Retained candidate implementation — 2026-11-06

The owner-approved R1–R5 / R-T1–R-T5 isolated implementation is complete and ready
for review. [Clean patch and tools](demonstrator/candidate-scan-implementation/README.md)
and [evidence](evidence/candidate-scan-implementation/README.md) accompany the
[updated recommendation](candidate-scan-recommendation.md). No shipping source,
ownership implementation, proof experiment or previous archive was changed.

- Final v4: 96 fresh serial processes, 384 operation records; exact archived/oracle
  geometry, glyph positions, slices, mappings and shaping deltas match. Supported
  fixtures have no unexpected fallback. Zero padding has no cutoff guards,
  preprocessing, trimming or scratch allocation.
- Numeric: 56,235,842 exhaustive checks and 10,000 seed-73074 cases pass. All 134
  focused cases/checks and inherited ownership lifecycle tests pass. Actual
  unsupported C/MXCSR rounding, FTZ/DAZ, negative/nonfinite padding and envelope
  overflow use exhaustive fallback; fast-math and representation rejection pass.
- Clean source/symbol audit passes; only TextWrapper.cpp and added ScanBound.h
  differ from the verified ownership baseline. Both patches apply with zero fuzz
  and reproduce built sources byte-for-byte.
- Historical gates: 751 disabled-equivalence fixtures / 775 blocks, 775 scan blocks,
  14,272 vertical cells and 2,901 visual fragments pass. Containment tolerance stays
  0.05; the same 95 near-edge snap observations remain.
- Work reduction is retained on padded long-span (3,842,986 → 16,852 evaluations)
  and spans-32 (3,346,678 → 16,468), with maximum paragraph scratch payload 524,320
  bytes. Across retained operations: 60 padded guard-block visits, 45,220,272 cheap
  endpoint visits, 15,853,892 trimming subtractions and 45,213,218 suffix comparisons.
  Guard blocks perform platform checks, one span-padding pass and envelope checks.
- Harness/recovery/supervisor/ownership/runtime Python regressions: 40 pass and one
  optional prior-schema attribution test is skipped. Preparation v1 failed its
  diagnostic-removal audit; v2 exposed NaN JSON self-inequality in the new
  nonfinite-padding test; v3 had a test-message type error. All remain terminal,
  preserved separately. V4 checks exact serialized nonfinite geometry, matching
  the trace contract; no scanner algorithm correction or alternate optimization
  was substituted.

Completed-run resume, repository lightweight checks, whitespace validation and
verified archive details are recorded in the linked evidence. Runtime effect is
unmeasured and requires a later approved plan. No merge or publication was done.

## Candidate-scan runtime study — terminal outcome, 2026-11-06

The supplied-plan implementation request approved CR1–CR5 / CR-T1–CR-T5 before
coding. The sibling runtime harness, fixtures, fixed schedule, fresh uninstrumented
builds, audits, reporting and tests are implemented. **Retention is not established.**

48 historical-fixture correctness processes / 192 operations match paired and
archived exact geometry, glyphs, slices, mappings and shaping. Process 49, the
exhaustive small-prose-64 zero-padding baseline, exits 2 with `shaping invariant
failed`. The initial-generation assertion assumes every paragraph shapes; source
inspection shows identical metadata-free paragraphs can restore from shared
ParagraphCache without incrementing that generation. The aborted output does not
identify the exact triggering paragraph/delta. No runtime or fixture adjustment,
retry or additional samples followed. Thirty-nine correctness and all 1,760 timing
processes remain unlaunched. Benefit, regression and all performance gates are
unevaluated; the report lists every incomplete cell without estimates.

Archived proof/cutoff hashes and live input identities pass. Independent historical
boundary/extent gates pass at unchanged tolerances (751/775 disabled fixtures/blocks,
775 scan blocks, 14,272 vertical cells, 2,901 fragments, 0.05 tolerance and 95 snap
observations). Ten new tests and 40 existing tooling tests pass, with one optional
historical attribution skip. Terminal resume rejects under fatal process-launch
sentinels; completed-run resume and full validator are unavailable because no
completed run exists. Separate journals, raw failure, reports, sources and hashes
are retained in [runtime evidence](evidence/candidate-scan-runtime/README.md).
Repository lightweight checks, whitespace and full archive readback are recorded
with that evidence. No production/API/integration, ownership, merge or publication
change was made.

## Cache-aware runtime continuation — 2026-11-07

Owner-approved CC1–CC4 continuation uses sibling `candidate-scan-continuation`
with fresh `build-ci/073-candidate-continuation` and
`build-ci/073-candidate-continuation-evidence`. The terminal predecessor is intact.

Both diagnostic arms reproduced small-prose-480-64/zero: initial paragraph 0
reported missingParagraph/addedParagraph and shaping delta 1; paragraphs 1–31
reported foundParagraph and delta 0. All 128 paragraph/phase observations per arm
are retained, including metadata presence. Normal cache behavior explains the
original assertion failure. A diagnostic journal formatting error occurred after
the baseline completed; its raw outputs were preserved and verified without
rerunning the process. The retained diagnostic completed normally.

Focused Python suite: 11 tests pass. Real corrected driver: all 22 fixtures ×
zero/padded (44 processes) pass cache/lifecycle assertions, including metadata
identity invalidation and repeated identical paragraphs. Historical geometry and
extent gates pass. Recovery: 9 tests; supervisor/performance: 8 tests; repository
`task check`: 87 tests and documentation/configuration checks pass. Initial C++
compilation exposed a string/const-char assertion-message mismatch, corrected
before these successful checks; failed build logs are retained. A premature
regression invocation could not launch the missing binary; successful regression
evidence uses a separate directory. No study processes had started at that point.

The fresh study schedule was persisted before building: 88 correctness followed
by 1,760 timing processes; the closed outcome is recorded below.

Fresh study actual outcome: all 88 correctness and 1,760 timing processes completed
without retries. Completed validator passes 924 adjacent pairs, 252 null metadata
records and symmetric uninstrumented flags/libraries. Historical geometry gates
pass 751 fixtures / 775 blocks and 147 transition probes; extent audit passes
14,272 cells / 2,901 fragments. Peak sampled RSS is 43.94 MiB; maximum wall time
is 5.17 seconds. Exact paired output, historical traces, lifecycle assertions and
timing/correctness shaping signatures all pass.

The 10,000-resample seed-73075 report gives 162 pass / 8 regression-budget fail /
6 inapplicable cells. All six primary padded operations pass, with approximately
60% (long) and 92% (spans-32) median improvements. Forty-two nonprimary cells are
timer-limited; absolute overhead gates still apply. All applicable small cells
pass. Eight nonprimary confidence upper bounds exceed their overhead limits;
their intervals include zero. **Retention is not established**, despite primary
benefits. No additional samples or tuning. Full values and limitations are in the
[continuation evidence](evidence/candidate-scan-continuation/README.md).

Completed-run resume passes with fatal sentinels on supervisor and subprocess
launch APIs: all 1,848 records restored, zero new process launches, and build,
execution, gate and index journals byte-identical. The destination is the separate
`build-ci/073-candidate-continuation-resume` directory.

Archive closure: 4,343 files packed into three independent tar.gz archives; every
member was read back and hashed, every archive SHA-256 verified, and all original
source/build identities rechecked before packaging. The repository-owned
[archive index](evidence/candidate-scan-continuation/archives.json) retains the
full per-file/archive hashes. The complete report and summary are also directly
reviewable beside the archives. Final lightweight checks pass (87 tests plus
documentation/configuration); whitespace validation passes. No shipping renderer
or public API files changed. CC-T1–CC-T4 are complete; qualification is rejected
under the unchanged gates. No production integration, merge or publication ran.

## Candidate-scan diagnosis (2026-11-08)

CD1–CD4 implemented under the owner's supplied-plan approval. The new sibling
`demonstrator/candidate-scan-diagnosis` only reads the closed study and existing
binaries. No renderer changes, benchmarks, rebuilds, instrumentation, new samples,
threshold changes, integration, commits, merges or publication were performed.

- Integrity passes: three continuation archives / 4,343 members; all 2,605
  existing identities plus supplemental identities; the unchanged generated
  schedule matches 1,848 completed records, adjacent alternating pairs and every
  raw/fixture hash. Published and archived reports agree. The unchanged reporter's
  10,000-resample output agrees byte-for-byte for both summary.json and report.md.
- Complete outputs: 176 cells / 3,520 pairs, including six null metadata cells;
  original gates remain 162 pass / eight overhead failures / six inapplicable.
  All eight failed intervals include zero. Forty-two applicable cells remain
  timer-limited, with original ratio suppression. All observations are preserved.
- Binary inspection: nm/objdump versions, commands, symbol sizes and disassembly
  retained. False-padding wrapping still has an additional padding predicate,
  scratch-state initialization/cleanup check and a 48-byte larger stack frame.
  No heap scratch allocation on that path is inferred. These compiled differences
  establish plausible added work, not measured causal attribution. Prose remains
  metadata-free even in the nominal padded variant. Order/parity and drift remain
  confounded exploratory comparisons.
- Nine focused tests pass, covering pairing/alternation, missing/duplicate pairs,
  identity/raw/archive rejection, nulls, normalization, timer suppression,
  constant/extreme samples, order/halves and summary disagreement. `task check`
  passes. Matrix construction and report rendering are deterministic. Final
  checks revalidate all 4,343 historical evidence members and 2,605 identities
  unchanged. Existing uncommitted continuation work is preserved; only diagnosis
  sections are appended to its feature documents.

The [diagnosis report](evidence/candidate-scan-diagnosis/report.md) preserves
**retention not established**. Proposed follow-up: separately approved path
isolation with baseline-versus-baseline calibration controls before attribution
or another full qualification. Neither experiment was run. Reproduction commands
are in the [analysis README](demonstrator/candidate-scan-diagnosis/README.md).
Analysis sources, input hashes, machine-readable outputs, source copies and binary
inspection evidence are packaged separately; historical raw archives are unchanged.

Diagnosis final verification: `git diff --check`, explicit new-source whitespace
checks and final documentation/local-link checks pass. Independent packaging with
stable metadata produced byte-identical archives, and each archive/member hash
was read back and verified. The archive index records the final bytes; the output
is separate from all historical archives. No acceptance label was changed.

A fresh-directory execution of the final analysis tool also passes: all matrix,
original-report, presentation, integrity and binary-inspection outputs are
byte-identical to the initial analysis. Final packaging verifies 52 members in
one archive, independently generated twice with identical bytes. SHA-256:
`ad2417fd6463923540c6bef7a818826330bbb791035908777abb105038c0a496`.
Focused tests: 9 passed; repository lightweight tests: 87 passed, plus
configuration and documentation checks. Logs are included in the diagnosis archive.


## Candidate-scan calibration — 2026-11-08

The owner's supplied-plan implementation request explicitly approved CB1–CB4
requirements/design and CB-T1–CB-T5 tasks before implementation. The sibling
[calibration tool](demonstrator/candidate-scan-calibration/README.md) uses only the
existing continuation baseline executable, with identical commands/environment
for both logical arms. Renderer sources, historical tools and archives were not
modified; existing uncommitted continuation/diagnosis work was preserved.

- Preflight verified continuation (4,343 members) and diagnosis (52 members)
  archives, current recorded source/binary identities and exact fixture bytes.
  The authoritative schedule, runtime identity, commands, inherited environment
  fingerprints, source/input hashes and exact archived limits were persisted
  before launch. No build or input regeneration occurred.
- Exactly two correctness and eighty timing processes completed, in the fixed
  seed-73076 balanced adjacent-pair schedule. Correctness geometry/shaping matched
  archived baseline records, timing shaping matched current correctness, and
  lifecycle, timer, floating-point and affinity checks passed. All four operations
  were preserved, including null prose metadata. No failed/ambiguous launch,
  retry, extra sample, warm-up, cache flushing or affinity change occurred.
- Fourteen focused tests passed before measurement, including deterministic
  balance/adjacency/commands, single-reference lifecycle, nulls/normalization,
  timer suppression, extremes, percentile/envelope boundaries, missing/duplicate
  pairs, altered identity/raw/stderr/fixture/schedule/command rejection, synthetic
  prefix recovery and terminal interruption before output creation.
- `task check` passed (87 repository Python tests plus configuration/documentation
  checks), as did whitespace validation. Actual completed-run resume into a new
  directory validated 82 records with fatal supervisor/Popen sentinels: zero
  launches. Recomputed summary, observations and report were byte-identical.
- Supervised process duration totaled 10.104 seconds; maximum sampled RSS was
  34,697,216 bytes and maximum reported high-water RSS was 74,305,536 bytes.
  No deadline or sampled RSS ceiling was exceeded.

Calibration is **unresolved**. Prose reflow's median B-minus-A overhead was
−414.5 ns/paragraph, with 95% interval [−6,296.5, 2,560.0], outside the exact
archived ±4,819.525000000001 envelope. Dense metadata's median was −1,289.5,
with interval [−26,529.5, 13,065.5], within ±101,542.07500000001. Neither target
was timer-limited. Both unchanged-repeat cells were timer-limited and retained
ratio suppression; all eight cells and 160 paired operation observations remain
visible in the machine-readable outputs.

**Retention remains not established.** This result neither changes historical
gates nor establishes equivalence or permits subtracting control estimates from
historical timings. Propose a separately approved measurement-design revision
before path isolation or further qualification: review timing-window resolution
and variability using controls, with new requirements/design/tasks and a fixed
budget before any additional measurement. No such revision is implemented here.
Twenty pairs are not a power guarantee; CPU placement, frequency, thermals and
scheduling remain unmeasured, and order/chronological summaries are exploratory.


Calibration delivery verification: the separate [evidence archive](evidence/candidate-scan-calibration/README.md)
contains 4,589 hash-verified members, including tool/source snapshots, frozen
schedule and identities, historical inputs, environment, attempts/completion
journals, raw stdout/stderr, all observations/reports and verification logs.
Read-back verified every member and archive hash, then rechecked historical
identities unchanged. Archive SHA-256:
`8936d4c066669376cfce4bdacc0bee8dd7f1803d867ea9b0f9a90f755002864b`.
All CB-T1–CB-T5 delivery tasks are complete. No commit, merge or publication occurred.

## Calibration revision execution (MR1–MR4, 2026-11-08)

The owner's supplied-plan implementation request approved requirements, design
and MR-T1–MR-T5 before coding. Historical tools and existing uncommitted work were
preserved. No renderer, build, API, candidate, commit, merge or publication changes.

- Preflight verified continuation, diagnosis and calibration archives and member
  hashes, current source/binary identities, fixture hashes and exact archived
  target limits. No rebuilds or input regeneration were performed.
- All 322 scheduled processes completed once: two correctness processes followed
  by forty contiguous ABBA/BAAB groups per fixture. Each four-group block has two
  of each order; seed 73078 and the complete schedule were frozen before launch.
  Commands/environment were identical between arms. Parent monotonic timestamps,
  sampled RSS, durations and all raw outputs were retained.
- Summed supervised duration: 39.696 seconds. Maximum sampled RSS: 34,672,640 bytes;
  maximum raw high-water RSS: 80,932,864 bytes. No deadline/RSS/lifecycle failures,
  retries, tuning, warm-ups or additional samples.
- Fifteen focused tests passed before launch: deterministic schedule and all 322
  commands, balance/contiguity, constant offset and linear-position drift,
  normalization/nulls, group timer thresholds, extreme values, percentile and
  inclusive envelope boundaries; lifecycle/reference checks; corrupted identities,
  fixture/raw/stderr hashes, missing/duplicate/misordered records, failed and
  interrupted attempts, complete no-launch and valid-prefix recovery.
- Repository lightweight checks passed (87 Python tests plus documentation and
  configuration/evidence syntax checks). Initial whitespace check passed.

Outcome: **unresolved calibration; retention remains not established**. Prose
reflow interval [−7,180, 1,856.75] exceeds the exact archived envelope
±4,819.525000000001 ns/paragraph; median −3,493.5. Dense metadata interval
[−21,943.75, 21,681.25] fits ±101,542.07500000001; median 5,137.25.
Neither target is timer-limited. Both repeat cells are timer-limited and prose
metadata remains null. All eight cells and 320 group-operation observations are
retained. Whole-group bootstrap uses 10,000 resamples, seed 73079 reset per fixture,
shared indices across operations and historical percentile indexing.

Stop here and reconsider measurement procedure before another separately approved
experiment. No historical gates were revised, runs pooled, calibration estimates
subtracted or historical variability claimed explained. Balanced groups cancel
ideal equally spaced linear-position effects, not necessarily real elapsed-time
drift. Forty groups are a fixed budget, not a power/independence guarantee; CPU
placement, frequency, thermals and scheduling remain unmeasured.

Delivery checks completed: independent reanalysis produced byte-identical
observations.json, summary.json and report.md. Completed-run resume validated all
322 records with fatal supervisor/Popen sentinels and zero launches. The
[revision archive](evidence/candidate-scan-calibration-revision/README.md) contains
9,658 members; each was read back and hash-verified. Archive SHA-256:
`b92337dc58d390564870aa74b08b5b426a046daea3819dc7dd56406a4fe6e983`.
Historical archive, source, binary and fixture identities revalidated unchanged.
Check logs, reproducible sources, schedule, commands, fingerprints, parent timestamps,
raw outputs, observations and journals are archived. Final documentation and
whitespace checks cover the delivery records as well as the sibling tool.

## Workstation measurement procedure update (2026-11-08)

Documentation-only update requested by the owner after accepting characterization,
with the correction that all experiments run on this non-dedicated workstation.
Added measurement-procedure.md and appended MP requirements, design direction,
tasks and recommendation. Historical protocol sections, tools and archives remain
unchanged. The procedure records ordinary shared use, available telemetry and its
limitations, genuine reflow batching, separate fresh-process observations, fixed
pilot selection and whole-group analysis. Exact execution parameters remain planned.

Verification: `python3 scripts/check-docs.py` and `git diff --check` passed;
the new procedure also passed an explicit trailing-whitespace check. No runtime
tests, builds or measurements were needed or launched for this prose-only change.

## Offline workstation characterization — 2026-11-08

The owner's supplied-plan request approved WC1–WC4 and WC-T1–WC-T4 before
implementation: existing-evidence analysis and a decision-complete draft pilot only.
Added the isolated candidate-scan-characterization tool and batching-pilot-proposal.md.
Existing uncommitted prior work was preserved. No public API/renderer change, build,
new measurement, commit, merge or publication was performed.

Verified continuation, diagnosis, calibration and revision archives and member
hashes, original pinned identities, schedules, raw records and closed journals.
Validated 1,848 continuation, 82 calibration and 322 revision records; verified all
9,658 revision archive members. Missing or inconsistent inputs stop analysis without
regeneration. Reconstructed all eight archived revision cells, group differences,
summary, observations and Markdown report exactly before adding diagnostics.

The complete JSON/CSV output contains eight cell summaries, 320 group-operation
rows and 1,280 process-operation rows. It includes process duration/midpoint,
chronological position, group span/gaps, B−A midpoint imbalance, normalized timings,
order/half summaries and descriptive Spearman correlations with average tied ranks.
No operation timestamps are inferred. Prose metadata remains null and both repeat
cells remain timer-limited. No significance tests, sample exclusions or causal
attribution were added. Retention remains not established.

Twelve focused tests passed: timestamp arithmetic/unequal spacing, group identities,
normalization/nulls, tied/constant correlations, synthetic offsets and real-time
versus ordinal drift, full matrix/timer flags, all 297 draft commands and balanced
contiguous groups, identical A/B commands/environments, missing/duplicate/misordered
records, altered identities/raw/fixture/schedule/member/archive hashes, failed and
ambiguous journals, and fatal supervisor/process-launch sentinels. An initial test
fixture omitted environment.json; it was corrected before the successful run.
Analysis and focused tests use fatal launch sentinels and perform zero launches.

The draft fixes sizes 1/16/64/256, off/on operation telemetry and a separate original
reference, seed 73080, nine correctness plus 288 timing processes, shared-cache
preparation and the smallest-enabled-size duration/MAD/resource selection rule.
MP-T2 is complete; the proposal portion of MP-T3 is complete, but its approval gate
and MP-T4/MP-T5 remain pending. The new harness and all future measurements require
separate approval. A later calibration must use a separate approved dataset.

Repository lightweight checks (`python3 scripts/check.py`) passed: 87 Python tests,
documentation/local links and configuration/evidence syntax. `git diff --check`
and explicit whitespace checks on new tool/proposal files passed. All 49 files in
the four preexisting historical source/evidence directories match the pre-edit
SHA-256 snapshot; full pinned archive/source/binary/fixture identities also passed
validation. Check logs and the preservation snapshot accompany the analysis output.

Independent full reanalysis passed under fatal launch sentinels: matrix JSON,
process/group/cell CSV, report, pilot schedule, draft protocol, input identities
and integrity record were byte-identical (nine deliverables). Both analyses also
reproduced the archived revision summary, observations and report byte-for-byte.
The 297 proposed commands were serialized and tested only; none were executed.

Delivery: the analysis archive was read back and every member hash verified;
historical identities were revalidated unchanged after packaging. The
[characterization evidence](evidence/candidate-scan-characterization/README.md)
provides the final archive manifest/hash, diagnostic JSON/CSV, report, draft
schedule and read-back log. Archive self-verification is recorded alongside the
archive (not inside itself); the archive contains the completed focused,
lightweight, whitespace, reanalysis and historical-preservation checks. Feature
requirements, design, tasks and verification snapshots are retained with sources.

## Fixed prepared-state batching pilot — actual results (2026-11-08)

The supplied-plan request approved the requirements/design/tasks and exactly one
297-process pilot. The sibling `demonstrator/candidate-scan-batching-pilot` tool
built only its Release `prepared-reflow` target with two workers against existing
baseline archives. Build flags, definitions and include paths matched the original
baseline exactly. Historical validators, supervisor and recovery tools were imported
unchanged. Preflight verified the continuation/diagnosis/calibration chains (6,988
pinned historical identities), plus revision and characterization archive/member
hashes and identities. No dependency regeneration, benchmark smoke or warm-up ran.

Ten focused Python tests passed under fatal launch sentinels, including all 297
command identities, order/balance/contiguity, synthetic orchestration, selection
boundaries, normalization/nulls, offsets/drift, telemetry failures, timer boundaries,
malformed records/hashes/journals and completed resume without launches. `task check`
passed. Real lifecycle acceptance used the nine declared correctness processes.

All **297/297** processes completed validly: nine correctness followed by 288 timing,
with eight contiguous four-process groups per configuration, seed 73080. All nine
correctness outputs passed original-reference geometry, cache and shaping checks;
new configurations passed independent-instance mutation isolation. There were no
retries or extra processes. Affinity and frequency policy were inherited; no tuning,
cache flushing, unrelated workload termination or quiet-period selection occurred.
The first-to-last parent-observed span was 125.464 seconds; summed supervised process
durations were 109.856 seconds. Maximum parent process duration was 2.41336 seconds.

The deterministic result is **no size selected**:

| Enabled size | Median batch ns | Unscaled MAD / median | Maximum parent ns | Maximum sampled RSS bytes | Exclusion |
| --- | ---: | ---: | ---: | ---: | --- |
| 1 | 133551 | 0.272843 | 71761998 | 28893184 | Duration below 10 ms; MAD ratio above 10% |
| 16 | 2287205 | 0.0286448 | 178398411 | 94572544 | Duration below 10 ms |
| 64 | 9521602 | 0.0116175 | 625396158 | 302641152 | Duration below 10 ms |
| 256 | 37507841 | 0.00512577 | 2413357677 | 1127469056 | Sampled RSS at/above 1 GiB |

All process resource evidence includes correctness. The 4 GiB terminal ceiling and
60-second deadline were respected. RSS observations cannot establish unobserved peaks.
The original's initial/reflow/repeat/metadata cells remain separate; metadata is null.
The report's reference row displays its initial-operation median, not batch reflow.

Raw and normalized process timings, all A/B group means and differences, medians,
unscaled MAD/range/signs, order and first-four/last-four summaries, matched-round
telemetry effects and collection durations are retained in `analysis.json`. The
median enabled-minus-disabled matched-round mean batch differences for sizes
1/16/64/256 were −3727.625/−40531/−19349.125/−37393.125 ns. These are descriptive,
non-simultaneous observations without causal, significance or calibration claims.
Parent snapshots and operation-boundary telemetry retain their sampling limits.

Retention remains **not established**. No provisional throughput size was selected;
this pilot neither qualifies historical results nor supplies a subsequent calibration
dataset. Later calibration requires a separately approved protocol and budget.

Completed-run resume and full reanalysis passed under fatal supervisor/Popen/OS
launch sentinels: **zero launches**, all 297 records revalidated, and `analysis.json`,
`selection.json` and `report.md` were byte-identical. Final lightweight and whitespace
check logs accompany the archive. Archive member read-back and final preservation
results are recorded in the pilot evidence directory's README.

Archive read-back completed successfully: **631 member SHA-256 values verified**,
followed by revalidation of historical pinned identities. See the
[evidence record](evidence/candidate-scan-batching-pilot/README.md) for the archive,
manifest, observations, no-selection decision and read-back log. All pre-existing
historical tools/evidence remained unchanged; specification updates append this
approval and the actual pilot outcome. No subsequent sampling is authorized.

## Fixed batching refinement actual verification — 2026-11-08

Approval: the supplied-plan implementation request, recorded in requirements,
design, tasks and [protocol](batching-refinement-protocol.md) before implementation.
Changed only the sibling `demonstrator/candidate-scan-batching-refinement/`, this
protocol, Feature 073 approval/recommendation/tasks/verification records and the
separate `evidence/candidate-scan-batching-refinement/` delivery. No historical tool,
archive, renderer or public interface was changed.

- Historical archive/member and pinned baseline source/library/binary/fixture/font/
  runtime identities passed pre-build verification, including the completed pilot.
- Isolated Release harness configured and built successfully with two workers and
  baseline compile definitions. Existing baseline dependencies were not regenerated.
  Existing header deprecation/ignored-attribute warnings remain in the build log.
- Ten inherited/adapted synthetic tests and three refinement tests passed under
  fatal launch sentinels. Schedule has exactly 231 unique identities, seven
  correctness first, balanced halves and contiguous four-process groups. Tests cover
  thresholds, normalization, drift/offsets, lifecycle/cache/timer source boundaries,
  optional telemetry, mixed ratio suppression, records/hashes/journals and recovery.
- Additional zero-launch checks rejected altered operation-boundary deltas, accepted
  explicit unavailable raw values/deltas, and archived an ambiguous journal plus
  partial output with incomplete/no-selection reporting. All 23 synthetic forensic
  archive members passed read-back hashes. Test source and logs are retained.
- The single fixed seed-73081 run completed **231/231 validly**: all seven full
  geometry/cache/mutation correctness processes passed before 224 timing launches.
  No smoke, warm-up, retry, replacement or adaptive measurements occurred.
- Selection is **96**, provisional prepared-state throughput among tested sizes.
  Median 14170985.5 ns; unscaled MAD 174587.5 ns; MAD/median
  0.012320068; maximum parent duration 907446534 ns;
  maximum sampled RSS 440905728 bytes, including correctness.
  Correctness and timing resource maxima are separately recorded in analysis.json.
- Full validation and completed-run resume into a fresh directory passed under
  fatal launch sentinels: zero launches and byte-identical analysis.json,
  selection.json and report.md. Every raw output, telemetry snapshot, attempt,
  command/environment identity, normalized observation and descriptive group/order/
  half/matched-round summary is retained in the archive.
- `task check` and `git diff --check` passed before delivery. Final archive read-back
  and post-delivery lightweight results are recorded beside the archive manifest.

Retention remains **not established**. The historical 297-process no-selection
result remains unchanged. No calibration, candidate comparison, commit, merge or
publication was performed or authorized. The work stops at evidence delivery.

Final archive delivery passed: **500/500 member hashes** read back successfully;
historical identities remained unchanged. The archive contains the specification
snapshot immediately before final packaging; this final checkpoint is recorded in
these repository records and the adjacent delivery log. Completed-run resume
launched nothing and reproduced all three deliverables byte-for-byte.
