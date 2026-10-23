# Whole-run software correctness gate

## Scope and provenance

Owner-approved bounded revision: 2026-10-23, W1–W7 in
[requirements](requirements.md), [design](design.md), [tasks](tasks.md).
[PR #49](https://github.com/lebedenko/qmarkdown/pull/49) closed without merging;
its diagnostic harness and historical evidence are recovered unchanged except
for an optional reverse-control order in the executable. Its original failures
and hosted timeout remain in [the historical report](diagnostic-verification.md).
[PR #50](https://github.com/lebedenko/qmarkdown/pull/50) merged at
`40c9f23ff6c4d9dfc521959fffeb34694db759ce`; this continuation branches from it.

Fresh exported source was `3bd978036be35b689876061672309736c9eb2459`, the local
PR #50 head. Both it and the actual merged main have identical Git tree
`970caef097d3abfe435719d2dde9742a7dd283a5`. No shipping inputs differ. Only the
recorded prepared-run snapshot patch is applied to that exported tree. Fresh
standalone Ninja configurations build against host exact Qt 6.12.0 and the local
CI image's exact Qt 6.8.0 SDK. Builds use two workers; math is disabled. No shipping
build, public API, package version or dependency changes are made.

[Identities](evidence/whole-run/identities.json) retain source/harness/font hashes,
toolchain/OS versions and Docker image identity. The explicitly loaded identical
files are DejaVu Sans Oblique and DejaVu Sans Mono. Prepared fonts must also
match the supplied file’s head/name/cmap/glyf tables. Code font style and weight
are explicit: normal/normal, avoiding inherited oblique style resolution. Historical legacy controls
continue using environment-resolved fonts and are not Qt-version comparisons.

## Execution and acceptance

See [reproduction instructions](retained-prototype/README.md) and each target's
`historical-summary.json`, `basic-summary.json`, and `extended-summary.json` under
[evidence](evidence/whole-run). Those summaries retain exact executable commands,
exit statuses and non-default environments; the runner fixes
`QT_QPA_PLATFORM=offscreen` and `QT_QUICK_BACKEND=software`. Raw compressed JSONL
contains runtime identity, font SHA-256, every run/glyph sequence, original
coordinates, physical bounds, colour and counts. This is QPainter's raster
engine on QImage, not a runtime-confirmed Quick scene-graph backend.

Each run tile keeps the entire prepared glyph list and original positions. All
controls disable decorations. Outward-rounded physical ink bounds receive a
four-pixel margin; drawing translates by an integer physical origin. Composition
uses integer physical coordinates without filtering or image scaling. Independent
full-canvas run crops preserve order. Exact premultiplied ARGB equality determines
acceptance; individual-glyph painting is diagnostic only. Full-canvas support
outside bounds and canvas-edge ink fail; tile-border ink is reported separately.

Each stage completes both control orders in fresh processes even on mismatch.
Both basic SDK matrices passed before either extended matrix started. The runner
checks distinct case counts, run/glyph counts, emission counts and order, and
actual first-glyph physical X/Y phases. The code fixtures assert one/two prepared
code rectangles, preventing a silently unpadded fixture from qualifying.

## Actual final results

Both exact SDKs pass every required case, in both forward and reverse orders:

| Stage | Cases per SDK per order | Tile/direct changed | Crop/direct changed | Cropped ink | Border ink |
| --- | ---: | ---: | ---: | ---: | ---: |
| Historical fX, DPR 1/1.25 | 2 | 0 | 0 | 0 | 0 |
| f/X/fX/fXfX, four DPRs, 16 independent phases | 256 | 0 | 0 | 0 | 0 |
| Code/adjacent code/colour/wrap/tiny width × three alignments | 960 | 0 | 0 | 0 | 0 |

Tile/crop also differs by zero pixels. All 4,872 required case records across
both SDKs/orders pass; final case records are identical between fresh orders.
Emission and occurrence checks pass. Prepared code uses Mono Book on both SDKs;
body uses Sans Oblique. Explicit-font individual-glyph diagnostic differences
are also zero here, without contributing to whole-run acceptance.

Environment-resolved controls reproduce DPR-1 success and DPR-1.25 failures on
two fresh repetitions: 306 changed pixels on Qt 6.8.0 and 379 on Qt 6.12.0.
The 258-case recovered diagnostic completes in each fresh forward/reverse
process and returns its expected mismatch status. Both orders report identical
comparison metrics: historical DPR-1.25 local/reference differs by 306 on
Qt 6.8.0 and 380 on Qt 6.12.0. Thus the prior 379-versus-380 isolated/multicontrol
discrepancy persists; reversing controls does not resolve it. No historical
failure is relabelled as a whole-run failure or silently discarded.

A representative DPR-1.25 reference/candidate/difference capture was inspected
on white; pixel comparisons, rather than visual inspection, determine acceptance.
`python3 scripts/ci-toolchain/qt_versions.py --sync-host /usr/bin/qmake6` reports
6.12.0 without changing the committed targets. `python3 scripts/check.py` and
`git diff --check` pass. These are lightweight repository checks, separate from
the exact-SDK executable results above.

## Initial harness correction

The first complete extended attempt had 64 failed cases per SDK/order, all
right-aligned width-1 cases. Its fixed canvas origin was too close to the left
edge for negative aligned run origins. The full-canvas-edge detector recorded
1,468 ink samples in aggregate; tile/direct differences totalled 842 pixels,
while crop/direct matched. This is a clipped harness canvas, not evidence of
uncropped run parity. The complete failed raw matrices and summaries remain in
`initial-canvas-attempt`; representative captures are retained. That attempt also
omitted the required Code format flag, so its code-span cases cannot qualify.

A subsequent evidence audit also caught inherited code-font style selecting
Oblique on the host versus Book in the container. Those provisional passes are
not the final qualifying matrix. Explicit normal code style plus font-table
identity checks now reject any such silent substitution.

The corrected harness reserves a larger integer canvas margin without changing
prepared run coordinates/phase, sets the Code flag as the renderer requires,
and asserts the prepared code rectangle count. Final basic and extended evidence
uses the corrected executable. No later rendering stage was attempted while the
harness failure remained unresolved.

## Limits and delivery

Exact software image equality would permit a later rendering design review only.
No scene-graph nodes, GPU backend, fractional scrolling, decorations/backgrounds,
broader shaping, lifecycle, resource retention, cost or performance are qualified.
Runs retain existing line/format/padding boundaries; there is no paragraph image
coalescing. No production-readiness or speedup claim follows.

The continuation PR must remain unmerged with auto-merge disabled. Final-head
hosted results are recorded in its description/checks, avoiding an evidence-only
follow-up commit. Hosted Light success cannot qualify this software experiment.
