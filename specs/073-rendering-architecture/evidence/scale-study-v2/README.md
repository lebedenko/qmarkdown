# Corrected complete scale study

> Storage cleanup: generated artifacts linked below were removed from Git history.
> See the [retention policy](../../../../docs/artifact-storage.md); historical results remain, raw-data revalidation requires regenerated inputs.

**Fresh matrix complete; live pass terminated by a confirmation-file race.**
Original scale 125% is restored and verified. Its approved replacement live pass
is now complete: see the [consolidated report](../../scale-study-results.md).
The terminal account below remains the historical v2 result.

The owner approved a new complete study after correcting the output validator.
The [original failed study](../scale-study/README.md) remains immutable. Total
budget is 691 launches: the original one plus this complete 690-process schedule.
See the [amendment](../../scale-study-protocol.md#corrected-complete-scale-study--approved-2026-11-09).

Eighteen offline tests pass, including the actual archived trace, complete offline
revalidation, conflicting/unbound/wrong output cases, durable failure metrics and
terminal no-launch recovery. No viewer/library changes or rebuild occurred.
[Preflight](preflight.json), [freeze](freeze.json), [schedule](schedule.json) and
[original output](original-output.json) record the inputs. Original scale is 1.25.

## 100% set — completed

All 170 scheduled cells are accounted for: 166 launches, **136 passes, 30 failures,
4 skipped dependent document recreations**. All 30 failures are Qt 6.8 Vulkan
(distance/curve × fifteen fixtures), with signal 11 after the shutdown record.
The failed initial document attempts make their two later recreations ineligible.
Both Qt 6.8 OpenGL modes, all four Qt 6.12 configurations and both Skia APIs pass.

[Cell summary](sets/100/summary.json), [set validation](sets/100/set-validation.json),
raw archive (removed artifact: `sets/100/study.tar.gz`), [archive hashes/read-back](sets/100/archive.json),
[member hashes](sets/100/members.json), and document contact sheet (removed artifact: `sets/100/document-contact-sheet.png`).
Capture hashes and frozen inputs pass validation. Agent inspected the contact
sheet; this is not owner acceptance. Sampled metrics and process exits are retained
for every launched attempt, including Vulkan failures.

Representative stationary Qt 6.8, Qt 6.12 and Skia document windows/capture buffers
are 800 × 600, zoom 1, with 800 × 600 viewport destinations and fractional scale
120/120. These observations establish 1:1 mappings for those cases, not universal
pixel equality or later-scale comparability. The later fractional-scale findings are recorded below.

## 125% set — completed

All 170 cells are accounted for: 166 launches, **136 passes, 30 failures and four
skipped dependent document recreations**. Failures remain confined to Qt 6.8 Vulkan
shutdown (signal 11); both Qt 6.8 OpenGL modes, all Qt 6.12 configurations and both
Skia APIs pass their correctness checks.

**Physical-size comparability fails.** In all ten stationary document cases, Qt's
nominal 800 × 600 window maps to an 800 × 600 surface and 1000 × 750 buffer.
Skia maps the same window coordinates to a 640 × 480 surface and 800 × 600 buffer.
At verified desktop scale 1.25 and document zoom 1, the nominal 22-unit font maps
to 27.5 physical pixels in Qt and 22 in Skia. The exact integer dimensions exclude
buffer rounding as an explanation. No font or rendering normalization was applied.
This finding blocks equal-size cross-engine visual/performance comparisons, while
the independent correctness results remain useful.

[Summary](sets/125/summary.json), [set validation](sets/125/set-validation.json),
raw archive (removed artifact: `sets/125/study.tar.gz`), [archive read-back](sets/125/archive.json),
[physical mappings](sets/125/physical-mappings.json),
[read-only review script](sets/125/review_mappings.py), and
contact sheet at a common pixel scale (removed artifact: `sets/125/document-contact-sheet.png`).
The agent inspected the contact sheet; no owner visual acceptance is inferred.
Surface IDs are linked to their viewports. SDL explicitly unsets viewport source
cropping with (-1,-1,-1,-1); the analysis retains these requests and treats them
as unset, not as an applied crop. First/restored destinations agree.

At this checkpoint the 150%, 200% and persistent-transition sets were untested;
subsequent results follow. The planned sequence
continues to characterize these mappings and transition behavior before deciding
what coordinate correction is needed; the study does not change rendering behavior.

## 150% set — completed

All 170 cells are accounted for: **136 passed, 30 Qt 6.8 Vulkan shutdown failures
and four dependent document recreations skipped**, from 166 launches. Qt 6.8
OpenGL, all Qt 6.12 configurations and both Skia APIs pass their correctness checks.
Frozen identities, capture hashes, journal results and archive member read-back pass.

**Physical-size comparability fails again.** Across the ten stationary document
cases, Qt's nominal 800 × 600 window uses an 800 × 600 surface and 1200 × 900
buffer; Skia uses a 533 × 400 surface and 800 × 600 buffer. Desktop scale 1.5,
zoom 1: nominal 22-unit font height maps to 33 physical pixels in Qt and 22 in Skia.
SDL's rounded surface width maps to 799.5 physical pixels, a 0.5-pixel horizontal
rounding difference. The vertical dimensions are exact; the 50% size ratio cannot
be explained by rounding. Rendering/font behavior remains unchanged.

[Summary](sets/150/summary.json), [set validation](sets/150/set-validation.json),
raw archive (removed artifact: `sets/150/study.tar.gz`), [archive read-back](sets/150/archive.json),
[physical mappings](sets/150/physical-mappings.json),
[review script](sets/150/review_mappings.py), and
contact sheet at a common pixel scale (removed artifact: `sets/150/document-contact-sheet.png`).
The agent inspected the contact sheet; owner visual acceptance is untested.
At this checkpoint the 200% set and live transitions were pending; subsequent results follow.

## 200% set — completed; live pass started

The final fresh set has **136 passes, 30 Qt 6.8 Vulkan shutdown failures and four
skipped dependent recreations**, from 166 launches. Across all four scales:
**544 passed, 120 failed, 16 skipped**, accounting for all 680 fresh cells.
The same eight nonfailing configurations pass at each scale. All 120 failures
are Qt 6.8 Vulkan process exits after shutdown; dependent recreations remain skipped.

At 200%, all ten stationary document mappings confirm Qt nominal font height
44 physical pixels versus Skia 22. Qt: 800 × 600 surface, 1600 × 1200 buffer.
Skia: 400 × 300 surface, 800 × 600 buffer. Desktop scale 2, document zoom 1;
all dimensions are exact integers. Equal-physical-size comparability fails.

[Summary](sets/200/summary.json), [set validation](sets/200/set-validation.json),
raw archive (removed artifact: `sets/200/study.tar.gz`), [archive read-back](sets/200/archive.json),
[physical mappings](sets/200/physical-mappings.json),
[review script](sets/200/review_mappings.py), and
common-pixel-scale contact sheet (removed artifact: `sets/200/document-contact-sheet.png`).
The agent inspected the contact sheet; owner visual acceptance remains untested.

All ten persistent bidi-objects windows started at 200% and reached checkpoint
live-0 with passing inverse-coordinate, containment and bidirectional-source checks.
They remain open awaiting the 150% handoff (live-1). Live transitions and repeated
scale states are not yet qualified. No additional windows or retries are scheduled.

## Live-pass terminal account

The supervisor terminated at live-5 after observing a newly created confirmation
file before its JSON content was written (`JSONDecodeError` at line 1 column 1).
The final handoff file is valid; the publication race is reproduced deterministically
in the separate repair tests. The supervisor killed all ten windows; their failure
results retain exit, elapsed time, sampled RSS and capture hashes. These are harness
termination failures, not ten newly demonstrated renderer failures.

[Terminal 690-cell summary](terminal/summary.json): **544 passed, 130 failed,
16 skipped**, with 674 launches. Of the failures, 120 are fresh Qt 6.8 Vulkan
shutdown failures and ten are interrupted live windows. Original initial failed
study adds one historical launch: 675 total so far. All four fresh sets remain
complete and useful; they will not be rerun for this repair.

[Checkpoint review](terminal/live-checkpoints.json) retains 50 of 70 scheduled
window checkpoints (live-0 through live-4), with stable window IDs, source-copy,
inverse-coordinate and containment checks passing. All ten repeated-125% captures,
geometry and mappings match. The final repeated-150% and 200% checkpoints are
untested (twenty window checkpoints); normal live shutdown was not reached.

The live windows retained their initial fractional preference of 240/120. Qt
retained DPR 2; the SDL surface destination remained 400 × 300. At the lower scales,
SDL live nominal font height is inferred as 16.5, 13.75 and 11 physical pixels at
150%, 125% and 100%, unlike the 22-pixel fresh-window result. Eight collected SDL
checkpoints differ from their fresh-window nominal physical size. Qt nominal
physical sizes match the corresponding fresh windows, while its buffer resolution
remains that of the initial 200% state. This is a measured mapping/lifecycle gap;
no compositor sampling or renderer-specific cause is claimed.

The owner restored 125%, and the complete output read-back matches the original.
Terminal archive (removed artifact: `terminal/study.tar.gz`) and [read-back](terminal/archive.json)
preserve all attempts and restoration. The [live-only repair proposal](../scale-study-live-recovery/README.md)
contains the tested atomic publisher and a frozen ten-window schedule. It subsequently received explicit replacement-scope approval and completed;
original failed attempts remain unchanged. See the consolidated report above.

The exact executed v2 supervisor sources are preserved in
frozen-supervisor.tar.gz (removed artifact: `frozen-supervisor.tar.gz`), with
[hashes and member read-back](frozen-supervisor-archive.json). The unchanged
viewers, libraries and fonts are preserved in the original study's frozen-input
archive. The completed replacement pass and final findings are summarized in the
[consolidated report](../../scale-study-results.md).
