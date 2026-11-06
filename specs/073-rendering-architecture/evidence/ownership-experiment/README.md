# Ownership optimization evidence — 2026-11-06

All 48 processes / 192 operation records passed exact paired and archived-baseline
geometry, glyph positions, visual slices, source mappings and shaping-delta checks.
No deadline/RSS failures occurred. Candidate, trimming, run-visit, reorder and slice
counts match the baseline after every operation. No supported fixture used fallback.
Width-only reflow and unchanged repeat rebuilt/allocated no ownership storage.

Across all instrumented phases/variants, legacy ownership comparisons total
**78,702,579,266**; optimized build-time span examinations total **1,756,528** and
fallback comparisons total **0**. Table lookups remain separate in
[summary.json](summary.json), alongside per-operation counts. These are work counts,
not CPU percentages or a speedup qualification.

## Initial padded layout (fitting plus formatting)

| Fixture | Legacy comparisons | Build examinations | Table lookups | Capacity entries | Payload bytes |
| --- | ---: | ---: | ---: | ---: | ---: |
| prose-480-4096 | 0 | 0 | 0 | 0 | 0 |
| dense-240-4096 | 7,838,251,771 | 63,368 | 58,462,329 | 356 | 2,848 |
| dense-480-1024 | 16,230,946 | 3,872 | 485,338 | 88 | 704 |
| dense-480-4096 | 3,937,369,825 | 63,368 | 29,414,732 | 356 | 2,848 |
| dense-960-4096 | 1,987,039,865 | 63,368 | 14,892,236 | 356 | 2,848 |
| mixed-480-4096 | 8,322,014,654 | 166,600 | 64,769,489 | 1,225 | 9,800 |
| long-480-16384 | 3,843,455 | 1 | 3,843,455 | 1 | 8 |
| objects-480-4096 | 2,290,976,268 | 73,947 | 21,761,202 | 471 | 3,768 |
| spans-480-1 | 5,402,105 | 2 | 5,402,105 | 2 | 16 |
| spans-480-8 | 134,518,923 | 128 | 21,282,811 | 16 | 128 |
| spans-480-32 | 1,820,992,752 | 2,048 | 74,865,099 | 64 | 512 |
| document-480-10 | 487,980 | 2,430 | 74,660 | 270 | 2,160 |

Zero-padding variants previously performed **zero ownership comparisons during
layout**, because their layout path did not request padded slices. They now pay
for table construction on initial layout and metadata changes: 439,132 examinations
per operation across these fixtures. No reduction is claimed for that zero-work
baseline. Prose has no metadata, builds no table and allocates no ownership payload.
Single-span cases replace one comparison per slice with one table lookup; the work
counts alone do not establish a runtime benefit for them.

## Memory and correctness

Actual capacities equal run counts; the largest per-paragraph table is 1,225 entries
/ 9,800 payload bytes (mixed fixture). Dense 4 Ki uses 356 / 2,848. The ten-paragraph
document totals 270 / 2,160. Reflow/repeat capacity is unchanged and focused tests
check the allocation pointer as well. Invalidating/removing metadata releases vector
storage. A rebuild swaps in a fresh run-sized vector; production rebuilds follow
invalidation. The vector header, two size_t revisions and bool add implementation
bookkeeping (41 field bytes before alignment on this host). Allocator metadata and
peak allocation overhead were not measured. Maximum sampled trace process RSS was
**51,265,536 bytes**; this includes the entire process and is not table attribution.

Historical gates: 751 disabled-equivalence fixtures / 775 blocks exactly match;
775 scan blocks, zero failures; boundary-minimal tests pass. Extent audit passes
14,272 vertical cells and 2,901 visual fragments at unchanged 0.05 containment
tolerance; 95 historical near-edge snap differences remain observations.
Complex shaping, objects, whitespace, zero width, transition probes, forward/reverse
copy and authoritative hit tests are included in these gates.

Focused C++ ownership/lifecycle tests pass. Python ownership recovery/attribution
(5), differential-rejection (4), historical phase (5), measurement/recovery (17)
and supervisor (9) tests pass. Completed-run resume validates into a new directory
without launching another trace. Repository lightweight and whitespace checks are
recorded in the final verification log.

## Retained outcomes and limitations

The first preparation attempt failed compiling a test initializer with fields in
the wrong order; no trace ran. Its build logs and runner error are preserved.
Correcting the harness and starting a fresh v2 workspace produced the successful
result. No geometry/invariant failure was encountered or retried.

Remaining costs include exhaustive candidate scanning, run intersection scans,
per-call slice/index/padding allocation, bidi reorder and formatting. In particular,
long and span-count fixtures still perform millions of candidate visits. Fragment
caching and candidate-scan reduction require separate design/approval. No adapter,
paint, native timing matrix, Qt integration, backend choice, merge or publication
was performed. Historical qualification/visual-acceptance gaps remain unchanged.
Raw traces, logs, source snapshots, patches and identity records are archived with
per-file and archive hashes in `archives.json`. Original phase evidence and commit
`4b9fd4c` are preserved.
