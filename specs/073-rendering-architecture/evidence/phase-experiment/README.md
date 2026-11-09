# Phase diagnosis evidence — 2026-11-06

All 24 zero/padded variant cases completed both instrumented and uninstrumented
four-operation traces (48 processes, 192 phase records). Geometry/source mappings
and shaping deltas match after every operation. No timeout or RSS termination;
maximum sampled process RSS was 51,040,256 bytes. These are diagnostic observations,
not a performance pass. All historical qualification gaps remain.

## Initial padded work counts

Fitting calls equal candidate visits in every row. Reorder calls also equal
candidate visits for these initial padded fixtures. Zero variants have no candidate
fitting; prose has no metadata and both variants follow the same unpadded path.

| Case | Lines | Candidates | Trim visits | Run visits | Owner comparisons | Slices |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| prose-480-4096 | 105 | 0 | 0 | 0 | 0 | 0 |
| dense-240-4096 | 238 | 488,376 | 84,847 | 173,861,856 | 7,838,182,974 | 58,461,854 |
| dense-480-1024 | 30 | 15,720 | 2,670 | 1,383,360 | 16,227,360 | 485,235 |
| dense-480-4096 | 119 | 245,229 | 42,602 | 87,301,524 | 3,937,311,708 | 29,414,317 |
| dense-960-4096 | 60 | 123,630 | 21,480 | 44,012,280 | 1,986,992,250 | 14,891,880 |
| mixed-480-4096 | 92 | 157,152 | 37,401 | 192,511,200 | 8,321,863,490 | 64,768,310 |
| long-480-16384 | 469 | 3,842,986 | 0 | 3,842,986 | 3,842,986 | 3,842,986 |
| objects-480-4096 | 91 | 136,679 | 28,485 | 64,375,809 | 2,290,923,045 | 21,760,707 |
| spans-480-1 | 406 | 3,533,922 | 1,766,961 | 7,067,844 | 5,401,698 | 5,401,698 |
| spans-480-8 | 406 | 3,358,294 | 1,679,147 | 53,732,704 | 134,516,378 | 21,282,390 |
| spans-480-32 | 407 | 3,346,678 | 1,673,339 | 214,187,392 | 1,820,981,766 | 74,864,630 |
| document-480-10 | 60 | 6,660 | 1,110 | 179,820 | 486,090 | 74,370 |

## Normalization and phase attribution

All phases, both variants, both call buckets, per-candidate and per-result-line
normalizations and adjacent-size ratios are in [summary.json](summary.json).
Undefined ratios (zero denominator) are null, not zero. Line denominators include
reused repeat lines. The raw archive retains individual paragraph counters.

| Initial padded case | Candidates/line | Run visits/candidate | Owner tests/candidate | Owner tests/line |
| --- | ---: | ---: | ---: | ---: |
| dense-240-4096 | 2,052.00 | 356.00 | 16,049.48 | 32,933,541.91 |
| dense-480-1024 | 524.00 | 88.00 | 1,032.27 | 540,912.00 |
| dense-480-4096 | 2,060.75 | 356.00 | 16,055.65 | 33,086,653.01 |
| dense-960-4096 | 2,060.50 | 356.00 | 16,072.09 | 33,116,537.50 |
| mixed-480-4096 | 1,708.17 | 1,225.00 | 52,954.23 | 90,455,037.93 |
| long-480-16384 | 8,194.00 | 1.00 | 1.00 | 8,194.00 |
| objects-480-4096 | 1,501.97 | 471.00 | 16,761.34 | 25,174,978.52 |
| spans-480-1 | 8,704.24 | 2.00 | 1.53 | 13,304.67 |
| spans-480-8 | 8,271.66 | 16.00 | 40.05 | 331,321.13 |
| spans-480-32 | 8,222.80 | 64.00 | 544.12 | 4,474,156.67 |
| document-480-10 | 111.00 | 27.00 | 72.99 | 8,101.50 |

Unchanged repeat has zero counted work for every fixture, and reflow/repeat have
zero shaping deltas. Metadata changes advance shaping once for every paragraph
with metadata. Prose metadata is a no-op. Dense width 480 has identical initial,
reflow and invalidation work in these inputs; width 960 changes from 60 to 55 lines
after +17 reflow, demonstrating that attribution follows actual work rather than
dividing aggregate counts by three. All operations remain separately recorded.

Dense 1 Ki → 4 Ki at width 480 increases initial candidate visits 15.600×, shaped-run
visits 63.108×, owner comparisons 242.634× and slices 60.619× for 4× input size.
This supports a repeated nested-scan explanation, not an asymptotic proof. The
1/8/32 independent-span cases retain similar candidate counts while owner tests
increase from 5.40 million to 134.52 million to 1.82 billion. Long-span ownership
is already one comparison per candidate; ownership precomputation alone cannot
remove its 3.84 million initial candidate visits.

Final-formatting owner tests are 58,117 versus fitting's 3,937,311,708 for dense
4 Ki/480; mixed has 151,164 versus 8,321,863,490. These count comparisons do not
measure CPU-time shares. Shaping itself, allocation and other uncounted loops
remain outside this attribution. Instrumentation overhead and one trace per case
preclude headline timing or noise qualification.

## Validation and provenance

- Historical boundary gates: 751 fixtures / 775 blocks, exact disabled equivalence,
  zero failures; boundary minimal and scan gates pass, including complex shaping,
  objects, whitespace, zero width and wrap transitions.
- Extent audit: 14,272 vertical cells and 2,901 visual fragments pass; 95 historical
  near-edge snap differences remain within the existing interpretation/tolerance.
- Runtime reset checks compare geometry before/after counter reset and preserve
  shaping generation; repeat attribution excludes preceding geometry-query work.
- Five phase/recovery tests, eight measurement tests, nine recovery tests and nine
  supervisor tests pass. Completed-evidence new-directory resume validates hashes
  and checkpoints without rerunning traces. Partial-phase tests preserve bytes and
  terminal outcomes, including repeated resume.
- Rebuilt headline libraries are byte-identical to the historical libraries. Pinned
  Skia `00227028b0c3cb3052432a456af2027a49e598b2` and all dependency revisions and
  recovered control identities verified. Source, executable and library hashes
  are retained in `identities.json`, plus the exact diagnostic patch.

Two preparation failures precede measurements: a source-only recovered tree lacked
build prerequisites; then preserved source timestamps caused Ninja to retain the
control library and linking failed. Both logs are retained. A new complete source
copy with freshly written patched sources rebuilt correctly. Neither failed
preparation reached any trace. No failed measurement was retried.

Raw traces, fixtures, identities, build/gate/test logs and interrupted/preparation
records are retained in the archive indexed by [archives.json](archives.json).
Every archive member was extracted in memory and SHA-256 compared to its source.
The durable local workspace is `build-ci/073-phase-workspace-v3`; primary raw
evidence is `build-ci/073-phase-evidence-v3`.

See the [approval-ready optimization proposal](../../optimization-design.md).
Optimization implementation, integration, merge and publication remain unauthorized.

Repository `task check` and whitespace validation passed. No shipping inputs changed.
