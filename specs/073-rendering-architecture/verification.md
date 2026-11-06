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
