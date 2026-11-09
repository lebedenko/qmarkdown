# Ownership runtime comparison — 2026-11-06

**48 correctness processes and 480 timing processes completed successfully.**
All 24 fixture/padding cells have ten complete adjacent fresh-process pairs,
with five baseline-first and five optimized-first pairs. No samples failed,
timed out, exceeded the RSS limit, were interrupted, retried or substituted.
There are no incomplete comparisons. Both binaries use the same uninstrumented
Release driver; initial layout includes shaping and ownership construction.

[Full per-operation report](report.md) and [machine-readable results](summary.json)
include all 96 cells, paired absolute-time differences, paired ratios and descriptive
95% percentile-bootstrap intervals (10,000 whole-pair resamples, seed 73073).
Difference means optimized minus baseline, in nanoseconds; it is not the difference
of two medians. There is no performance acceptance threshold or pooled speedup.

## Effects and recommendation

Retain the ownership optimization as measured. Padded dense/mixed multi-span
initial layout shows substantial reductions: dense 4 Ki at width 480 has medians
2.781 s → 0.437 s and median paired ratio 6.368; mixed has 5.934 s → 0.927 s and
ratio 6.425. Dense width 240 has 5.524 s → 0.852 s (ratio 6.507). The full report
provides each case's uncertainty and reflow/metadata effects separately. These
observations do not establish a combined headline speedup or backend qualification.

Remaining padded layout costs are still large: spans-32 initial layout is 1.608 s,
mixed 0.927 s and the long single-span case 0.333 s after optimization. Prior work
counts identify candidate scanning/intersection/slice construction as remaining
costs; this uninstrumented study does not apportion those costs. A separately
approved candidate-scan investigation is the useful next step, with zero-padding
metadata regressions retained as a constraint. No further optimization is implemented.

## Zero-padding preparation overhead

The zero-padding baseline did no ownership search during layout. The optimized
build adds ownership preparation on initial/metadata layout, so these cases must
be assessed independently. Below are median paired differences in microseconds
with descriptive 95% intervals; negative values favor the optimization. They
include all layout/shaping work and host noise, rather than isolating table cost.

| Fixture | Initial difference µs [95%] | Metadata difference µs [95%] |
| --- | ---: | ---: |
| prose-480-4096 | -0.69 [-85.22, +217.88] | null (no metadata) |
| dense-480-1024 | +4.54 [-24.55, +141.28] | +1.32 [-7.11, +7.83] |
| dense-480-4096 | +15.63 [-55.32, +58.27] | +44.46 [+20.38, +70.32] |
| dense-240-4096 | -89.12 [-124.65, -46.44] | -22.16 [-72.67, +19.88] |
| dense-960-4096 | +51.77 [-3.88, +173.07] | +42.67 [+2.42, +81.23] |
| mixed-480-4096 | +58.84 [-129.36, +229.68] | +177.54 [+27.51, +376.53] |
| objects-480-4096 | +42.62 [-8.39, +180.47] | +52.63 [+11.41, +157.93] |
| long-480-16384 | -70.34 [-214.12, +208.75] | -71.34 [-191.69, +33.75] |
| spans-480-1 | -71.37 [-276.03, +208.59] | +68.83 [-93.08, +165.27] |
| spans-480-8 | +156.17 [-129.00, +356.65] | +1.63 [-153.35, +101.44] |
| spans-480-32 | +38.93 [-31.41, +305.13] | -82.41 [-224.12, +79.98] |
| document-480-10 | +42.22 [-1.88, +98.30] | +12.92 [-9.81, +53.64] |

For example, zero-padding dense-480-4096 metadata is +44.46 µs [20.38, 70.32]
and mixed metadata is +177.54 µs [27.51, 376.53]. Many other intervals include
zero, and dense width-240 initial happens to favor the optimized build despite
additional preparation. These descriptive results do not imply that table
construction improves an ownership-free path. The metadata-free prose control
allocates no table; its metadata timing is null in all 44 process records.

## Single-span cases and timer limits

Long padded initial layout changes 342.05 ms → 332.74 ms, with median paired
ratio 1.039 and difference −12.88 ms [−15.70, −7.80]. Its reflow difference is
−9.26 ms [−13.42, +14.76], which remains inconclusive. Spans-1 padded initial
changes 416.83 ms → 407.30 ms, ratio 1.020 and difference −8.17 ms
[−14.52, −6.53]. These small effects are distinct from dense multi-span benefits.
Zero-padding single-span initial/metadata intervals include zero.

The steady-clock period is 1 ns; empirical minimum positive deltas are 11–13 ns
across timing processes. All 24 unchanged-repeat cells fall below the 100×
resolution threshold. Their absolute timings and intervals remain in the report,
with ratios suppressed. Timer overhead was not subtracted.

## Verification, provenance and limits

All 192 correctness operation records exactly match paired and archived-baseline
geometry, glyph positions, visual slices, source mappings and shaping deltas.
Timing mode retains shaping-reuse/invalidation guards and performs no geometry
queries between operations. Historical gates pass 751 disabled-equivalence
fixtures / 775 blocks, 775 scan blocks with zero failures, boundary-minimal checks,
14,272 vertical cells and 2,901 visual fragments. Existing historical near-edge
observations are not changed or reclassified by this study.

The final audit confirms 264 adjacent process pairs (24 correctness + 240 timing),
identical compiler flags, identical shared dependency libraries and absence of
phase instrumentation. Focused runtime tests (11), measurement/recovery tests
(17), supervisor tests (9), repository lightweight checks (87 Python tests), and
whitespace validation pass. See archived logs and verification results for commands.

The first completed-run resume validation found that gate command recording had
changed the build command journal after its checkpoint. No timing data was affected.
The original identities/checkpoints/runner and failure log are preserved under
`as-run/`; the original full command journal is unchanged. Separate build/gate
journals and a focused regression fix the bookkeeping. The corrected checkpoint
references the exact original build-journal bytes. Both original and corrected
runner identities are retained. See archived `provenance-correction.md`.

Maximum sampled RSS is 50,053,120 bytes; maximum process high-water RSS is
90,292,224 bytes. These are whole-process observations, not ownership-table
allocation measurements. Clocks measure operation wall elapsed time, including
scheduling noise. Affinity spans the available host CPUs; frequency, load and
filesystem caches are uncontrolled. Brief independent harness regressions ran
during the early first pass; no system-exclusivity claim is made. Bootstrap
intervals are descriptive for ten pairs on this host, with no multiple-comparison
adjustment, cross-machine generalization or independent warm-cache qualification.

No native rendering, Qt integration, backend selection, merge or publication was
performed. Existing ownership diagnostics were not repeated. Raw outputs, partial
outcomes if any, source snapshots, driver/build sources, patches, input identities,
environment, commands, logs and reports are retained in independently readable
archives with verified per-file and archive hashes in [archives.json](archives.json).
