# Scale diagnosis and performance qualification

Measurement continuation approved by the owner on 2026-11-06. Recovery and
measurement reporting are complete; the results do not qualify performance. The owner's visual
review is accepted for the cases inspected, without extending acceptance to other
fixtures, APIs, outputs or scale configurations. Historical failures and the
independent Qt 6.8 Vulkan shutdown blocker remain in force.

## Corrected scale explanation

The original measured desktop was at **1.25**, on Samsung U32J59x / DP-1. Both native APIs
report SDL 2.32.74 (provided here by **sdl2-compat 2.32.74-1**),
an 800×600 window-coordinate extent, an 800×600 drawable and
horizontal/vertical drawable-to-window ratios of 1. This ratio does **not** mean
that the desktop scale is 1.

The separate Wayland traces establish the following mapping on both APIs:

| Quantity | Observed value |
| --- | --- |
| Output mode | 3840×2160 at 59.997 Hz |
| Output logical extent | 3072×1728 |
| Integer output scale event | 2 |
| Fractional preferred scale | 150/120 = 1.25 |
| Allocated buffer | 800×600 pixels |
| Viewport source | Unset (`-1` in all four fields): full buffer |
| Viewport destination | 640×480 surface units |
| Buffer pixels per surface unit | 1.25 horizontally and vertically |
| SDL coordinates per surface unit | 1.25 horizontally and vertically |
| Document zoom | 1 |

There is no explicit `set_buffer_scale` request in either trace. The core Wayland
protocol's initial buffer scale is 1; the SDL-owned viewport provides the
fractional mapping. Neither trace contains an explicit `xdg_surface.set_window_geometry`
request. Consequently, independent window-geometry/frame extents are not measured;
the surface extent follows the observed viewport destination.

The native renderer uses one document unit per SDL window coordinate at zoom 1.
Here that is one buffer pixel, or 0.8 surface units. A nominal 22-unit font is
therefore 22 buffer pixels, not 27.5 pixels as a surface-logical 22-unit font at
1.25 density would be. The 800/800 observation is explained by SDL's coordinate
compatibility-layer coordinate convention plus its viewport mapping, rather than a desktop scale of 1. Output
mapping is nominally 800 physical pixels for this 640-surface-unit content. The
trace does not establish the compositor's sampling filter or exact final pixel
coverage. No SDL upgrade, competing viewport object or compositor change was made.

The mapping permits comparisons **among these native variants on this output**.
It does not establish equal physical text sizing with Qt controls or qualify any
other desktop configuration. Historical references to “DPR 1” describe only the
observed SDL drawable/window ratio and should be read with this correction.

See [machine-readable scale facts](scale.json). Raw traces are retained separately
from all timing windows. Presentation uses the compositor-advertised monotonic
clock (clock ID 1), not swap-return timestamps. OpenGL advertises Ganesh elapsed
GPU timing support; Vulkan does not, so Vulkan GPU elapsed time is unavailable.

## CPU record schema correction

The retained CPU timing sweep uses the executable/source identities from
`073-performance-v2`. Its `shapingReused` field was initialized to true even for
uninstrumented upstream, where no assertion had been made. That field is **not
verification evidence for upstream** and is ignored by the summary generator.
The exact [timed CPU source](cpu-timed-source.cpp) and
[fixture generator snapshot](cpu-timed-workloads.py) are retained. The current adapter
reports null when it cannot check reuse; separate diagnostic builds provide the
actual checks. No raw timing samples were rewritten.

The current diagnostic adapter also changes the first metadata identity to a fresh
identity, instead of reusing identity 1. The timed fixtures separate all distinct
code spans with non-code text; they contain no adjacent code-span pairs. The
current checks use a fresh identity rather than relying on that fixture property. Metadata timings describe setter plus invalidated shape/layout, not the
setter alone. The original operation remains inspectable in the timed source.

## CPU results

All 288 variant/case combinations were accounted for: 178 completed the full
sampling protocol, 36 exhausted the 60-second batch deadline, and 74 larger cases
were skipped within the affected series. There were 206 complete ten-process
batches, 178 complete 30-sample warm batches, 2,095 retained fresh-process samples
and 5,546 retained post-warmup samples. No process reached the 4 GiB RSS ceiling.
A batch timeout is not a claim that one layout took 60 seconds.

Ten 256-unit object-bearing paragraphs, width 480, complete **warm** batches:

| Variant | Recorded samples | Construction median (ms) | Initial layout median (ms) | Reflow median (ms) |
| --- | ---: | ---: | ---: | ---: |
| upstream | 30 | 7.416 | 0.551 | 0.053 |
| zero | 30 | 7.319 | 0.921 | 0.058 |
| padded | 30 | 7.411 | 3.435 | 2.414 |

A single 4 Ki-unit span-dense paragraph, width 480, **fresh-process** batches:

| Variant | Recorded samples | Construction median (ms) | Initial layout median (ms) | Reflow median (ms) |
| --- | ---: | ---: | ---: | ---: |
| upstream | 10 | 195.598 | 4.840 | 0.103 |
| zero | 10 | 181.580 | 3.385 | 0.206 |
| padded | 7 (partial) | 197.895 | 2777.029 | 2769.050 |

The separate phase timings distinguish substantial adapter construction cost
from the padded candidate-fit/layout cost. The reused builder repeatedly visits
and validates span ranges while adding source characters; this path is present
in every variant. Padded reflow remains expensive even after shaping is reused.
Unchanged repeated layout is much cheaper; very small values include timer overhead.
No new initial-layout latency threshold is applied.

The 1,000-paragraph inputs repeat the same object-bearing 256-unit block. Upstream
can reuse its paragraph cache within that document; the boundary variants retain
the existing metadata-related cache bypass. These are controlled workloads, not a
claim about all real documents. RSS is the process high-water mark, including
fonts, allocator retention and library state; no renderer-only memory estimate or
memory acceptance threshold is invented.

[CPU summaries](cpu-summary.json) retain sample counts, completion status, median,
nearest-rank p95, maximum and RSS for every case and phase. [Adjacent-size growth
ratios](growth.json) retain partial/missing status and do not constitute an
asymptotic complexity proof.

Static curves use logarithmic axes and omit unavailable points:
[construction](constructionMs.svg), [initial layout](initialLayoutMs.svg),
[width reflow](widthReflowMs.svg), and [RSS](peakRssBytes.svg) show complete warm
batches. Fresh-process [construction](constructionMs-cold.svg) and
[initial layout](initialLayoutMs-cold.svg) additionally mark partial batches with
crosses and sample counts. All 21 workload/width series are shown; PNG companions
are retained for inspection.

These are measurements on the current shared desktop, without CPU affinity,
exclusive-host, thermal-state or cold-filesystem-cache guarantees. The main CPU
sweep and native timing matrix ran serially; a brief initial scale trace and light
tooling occurred during the CPU sweep. Protocol traces, PNG encoding and per-frame
console output were excluded from native timing windows. Available original records establish fonts, dependency versions and output mapping.
CPU, governor and driver facts collected during recovery are labeled post-reboot;
unavailable pre-reboot facts remain unknown.

## Native rendering, presentation and memory

The [per-process table](native-scenarios.md) keeps every scenario, repetition and
attempt separate. The [full native summary](native-summary.json) includes phase
median/p95/maximum, cache/RSS, scale, presentation flags, gaps and supervisor status.
Stationary, slow scrolling, flick/reversal, settling and selection changes trigger
**zero measured layout calls** in the completed original matrix; resizing performs
layout. No traversal optimization was introduced.

Representative stationary **repetition 0** results (milliseconds):

| Workload | Variant | API | Paint median | GPU elapsed median |
| --- | --- | --- | ---: | ---: |
| typical | upstream | OpenGL | 0.201 | 0.0124 |
| typical | zero | OpenGL | 0.204 | 0.0144 |
| typical | padded | OpenGL | 0.240 | 0.0123 |
| span-dense | upstream | OpenGL | 2.906 | 0.0205 |
| span-dense | zero | OpenGL | 1.857 | 0.0211 |
| span-dense | padded | OpenGL | 2.174 | 0.0286 |
| large-document | upstream | OpenGL | 15.708 | 0.2815 |
| large-document | zero | OpenGL | 8.305 | 0.3087 |
| large-document | padded | OpenGL | 10.769 | 0.2534 |
| large-document | upstream | Vulkan | 15.221 | unavailable |
| large-document | zero | Vulkan | 7.590 | unavailable |
| large-document | padded | Vulkan | 10.060 | unavailable |

These are render-thread wall durations. Background rectangle/fragment query paths
and padded geometry intentionally differ. The lower zero-variant paint cost in
the large document is not evidence that boundary layout is universally faster.
Typical upstream OpenGL repetition 0 has median submission 0.347 ms and swap
blocking 15.977 ms: waiting for presentation dominates that frame, while measured
GPU elapsed time is small. GPU timestamps cover Ganesh's supported flush work,
not compositor/display execution. Vulkan timing is unavailable in the pinned
Ganesh backend; this does not establish a hardware limitation.

Resize exposes a different bottleneck. Span-dense padded OpenGL repetition 0 has
median layout 127.581 ms and presentation p95 166.687 ms. Large-document padded
resize has median layout 183.0–190.0 ms across the six API/repetition combinations,
presentation p95 approximately 250 ms and maxima up to 266.697 ms. These are
observed stress limits, not newly invented layout acceptance thresholds.

OpenGL reports swap interval 1; Vulkan uses mailbox present mode. Typical Vulkan
stationary repetitions submit thousands of commits that are discarded while
about 300 are presented during five seconds. Submission throughput is not display
frame rate, and lower Vulkan paint timings are not an equal-scheduling backend
qualification. Every discarded and missing feedback result remains counted.

Presentation refresh is 16,667,500 ns (59.997 Hz), advertised clock ID 1 with
`clock_getres` of 1 ns. Observed interval granularity is commonly 1,000 ns; that
observation does not establish timestamp accuracy. The existing target is applied
literally: p95 within one reported refresh period and maximum within two, allowing
only the two-endpoint reported clock resolution. Even typical upstream OpenGL
stationary p95 values of 16.703/16.717/16.716 ms miss the first target. No jitter
allowance was invented to turn these into passes.

Four completed original processes have missing feedback: typical padded OpenGL
flick/reversal repetition 2, settling repetitions 1 and 2, and resize repetition 0.
The last has no presented measured commits; flick/reversal includes a 1.867-second
presentation gap. Their cause is not established. These observations and the
separate window-close failure prevent attributing that cluster solely to layout
or GPU execution. Missing feedback is a qualification gap, never a pass.

In the original completed matrix, process high-water RSS ranges from 282.0 to
490.3 MB for OpenGL and 216.5 to 402.7 MB for Vulkan (decimal MB). This includes
fonts, driver allocations, documents and buffered JSON samples. Skia GPU-cache
usage is separately 6.20–6.44 MB and 4.44–5.00 MB, respectively; it is **not total
GPU memory**. No native process hit the 4 GiB process ceiling.

## Follow-up proposals, not implemented

1. Reduce repeated padded candidate-fit work. Profile and then redesign repeated
   visual-fragment/run/span-owner scans, preserving the authoritative geometry
   and all boundary correctness gates. Padded width reflow and resize are the
   strongest observed layout bottlenecks; shaping reuse alone does not fix them.
2. Reduce adapter construction work through validated bulk input construction or
   precomputed span membership. Large construction cost is common to all variants;
   it must be separated from library layout when evaluating a future change.
3. Investigate large-document paint traversal and background-query cost, then
   propose visibility culling or caching with explicit source/selection invariants.
   The current full traversal remains unchanged and its cost is measured here.
4. Investigate scheduling and presentation feedback independently: mailbox commit
   discard volume, the missing-feedback cluster, and the strict pacing misses.
   Any frame scheduler, SDL migration or coordinate-contract change needs a new
   approved design. Preserve explicit document/surface/buffer units.
5. Obtain supported Vulkan GPU elapsed measurements before making GPU-cost or
   production-backend comparisons; retain the separate Qt 6.8 Vulkan shutdown
   blocker. A future metadata-aware cache design also needs its own correctness
   and invalidation review.

The diagnostic supervisor survived a server restart; an attempted continuation
briefly overlapped it. Those diagnostic files are retained as interrupted evidence
and excluded from reported counter conclusions. A clean isolated output uses the
same verified instrumented binaries and the pre-overlap geometry gates. Headline
CPU/native measurements finished before that overlap and are unaffected. The
later diagnostic sweep is not a timing source.

These are prioritized investigation proposals, not an optimization implementation,
backend selection, Qt integration, merge or publication decision.

## Recovery completion — 2026-11-06

**Measurement and reporting work is complete; performance is not qualified.**
The full CPU sweep, 323 completed original native processes, 18 captures, original
scale traces and historical correctness gates were preserved. No full matrix was
rerun. The overlapping diagnostic attempt is archived but excluded from conclusions.

The recovery imported all 59 clean comparisons and 17 terminal outcomes, then
accounted for the remaining 116 patched variant/cases. The interrupted
`mixed-480-65536 / zero / instrumented` operation restarted in a new directory;
its original unacknowledged output remains retained. Final diagnostic coverage is:

| Outcome | Patched variant/cases |
| --- | ---: |
| Completed, identical instrumented/headline geometry | 151 |
| Timed out at the existing deadline | 23 |
| Skipped larger cases in an affected series | 18 |
| Total | 192 |

All 18 planned upstream shaping-counter comparisons pass. All completed patched
counter checks establish width/repeat shaping reuse and the expected extra shape
generation on metadata invalidation; metadata-free cases have no invalidation
operation. All 82 available cross-variant source-mapping comparisons pass. Glyph
and line differences across variants remain recorded, rather than required to be
equal. See the [coverage and counter audit](recovery/diagnostic-summary.json).
Diagnostic durations are not headline performance samples.

Pinned Skia, dependencies, GN and patched/counter source identities were restored.
All six rebuilt upstream libraries match their historical hashes. The new upstream
counter adapter passes the retained regression-control and 751-width control
geometry comparisons. Existing patched executables were reused by hash. Original
build manifests remain intact; [the recovery manifest](recovery/recovery-manifest.json)
records relocation, rebuilt prerequisites and preparation failures separately.

Fresh serial OpenGL/Vulkan traces reproduce the original 1.25 mapping, output,
viewport and buffer facts. Available historical SDL fields, fonts and six checked
dependency versions match. See [post-reboot scale facts](recovery/scale.json),
[environment observations](recovery/post-reboot-environment.json) and
[comparison limits](recovery/environment-comparison.json). Pre-reboot driver,
governor, thermal state and full runtime-library hashes remain unknown.

Only the failed typical/padded/OpenGL/settling/repetition-0 process was repeated,
once, as **post-reboot attempt 1**. It completed the 2-second warmup and 5-second
measurement with 300 presented frames, no missing feedback and zero measured
layout calls. Presentation p95 was 16.799 ms and maximum 16.824 ms against a
16.6675 ms refresh period: the unchanged p95 target still fails. The original
window-close failure is retained. This attempt remains a separate cohort in the
[scenario table](native-scenarios.md) and [replacement summary](recovery/native-retry-summary.json).
The four original missing-feedback cases were not repeated and remain gaps.

The corrected CPU adapter smoke used ten 256-unit paragraphs at width 480, with
ten fresh processes and five discarded plus thirty recorded warm iterations for
each variant. All batches completed. Separate post-reboot warm medians (ms):

| Variant | Construction | Initial layout | Width reflow |
| --- | ---: | ---: | ---: |
| upstream | 7.352 | 0.499 | 0.052 |
| zero | 7.229 | 0.918 | 0.056 |
| padded | 7.435 | 3.391 | 2.401 |

[Smoke samples and summaries](recovery/cpu-smoke-summary.json) do not replace or
pool with the original CPU sweep. Its source/schema caveat remains in force.
Regenerated original CPU summaries, growth records and all six chart PNGs are
byte-identical to the retained originals. Charts and representative typical and
span-dense captures were inspected for legibility and integrity; this does not
extend the owner's visual acceptance.

Raw files and archive members were hash-verified. Archive indexes cover
[original measurements and excluded diagnostics](original-archives/archives.json),
[recovery measurements](recovery-archives/archives.json),
[executable/source identities](identity-archives/archives.json), and
[retained build records](build-record-archives/archives.json).
[Recovery logs and validation records](record-archives/archives.json) are archived
with independently verified member hashes. The first partial
identity bundle remains preserved and is supplemented by the complete identity
bundle. Archive environment metadata is explicitly post-reboot.

Original temporary logs and instrumented library archives remain lost. The older
`073-performance-v2` adapter source archive did not survive; exact bytes for five
superseded files are unavailable, listed in the
[source availability record](recovery/timed-source-availability.json). Its timed
CPU translation unit, boundary header, workload generator and runner are retained
by hash, together with executable identities and compiler/link build records.
Historical source-archive reconstruction stops at these gaps; no replacement
source bytes or blanket measurement rerun is substituted.

The strict pacing misses, four feedback gaps, unavailable Vulkan GPU elapsed time,
and independent Qt 6.8 Vulkan shutdown blocker remain. The follow-up proposals
above are unchanged. No optimization, backend selection, Qt integration, merge or
publication occurred.

Validation passes nine recovery tests, eight measurement tests, nine supervisor
regressions and all 87 repository Python tests, plus syntax, documentation and
whitespace checks. A [finished-phase resume](recovery/resume-validation.json)
passes with measurement/build calls disabled and preserves byte-identical indexes.
