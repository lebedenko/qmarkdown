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
