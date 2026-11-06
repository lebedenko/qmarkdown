# Retained candidate-scanner evidence — 2026-11-06

The approved isolated implementation is complete and ready for review. See the
[clean patch and reproduction tools](../../demonstrator/candidate-scan-implementation/README.md)
and [recommendation](../../candidate-scan-recommendation.md). Runtime measurement,
shipping Qt integration, ownership changes, merge and publication were excluded.

Final run: `build-ci/073-candidate-implementation-evidence-v4`, workspace
`build-ci/073-candidate-implementation-v4`. The persisted serial schedule completed
**96 fresh processes / 384 operation records**: twelve fixtures × zero/padded ×
oracle/shadow/instrumented retained/clean retained. Each covers initial layout,
width +17, repeat and metadata mutation/layout. Exact geometry, glyph positions,
visual slices, mappings and shaping deltas match hash-validated archived traces and
the exhaustive oracle. Supported fixtures have zero unexpected fallback. Ownership
lifecycle and formatting guards pass. Clean sources and symbols contain no
scan/ownership diagnostic state or shadow evaluation; only TextWrapper.cpp and
added ScanBound.h differ from the ownership baseline. Both patches apply with zero
fuzz and reproduce the built sources byte-for-byte. Original proof source and
archive hashes were checked unchanged.

| Initial padded fixture | Exhaustive evaluations | Retained evaluations | Endpoint visits | Trimming subtractions | Suffix comparisons | Guard-block visits | Reserved payload |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| long-span | 3,842,986 | 16,852 | 3,842,986 | 0 | 3,842,517 | 1 | 524,320 bytes |
| spans-32 | 3,346,678 | 16,468 | 3,346,678 | 1,673,339 | 3,346,271 | 1 | 524,320 bytes |

Across retained diagnostic operations: 60 guard-block visits, 45,220,272 endpoint
visits, 15,853,892 trimming subtractions, 45,213,218 suffix comparisons, 273,367
expensive evaluations and 44,946,905 eliminated evaluations. Shadow validates all
44,946,905 hypothetical skips. A guard-block visit performs platform checks, one
span-padding pass and the overflow-envelope checks; it is not an instruction count.
Zero-padding paths have zero guard visits, endpoint preprocessing, trimming and
scratch allocation. Vertical-only padding keeps the original padded-scanner path.

The largest vector uses 16,384 entries and reserves 16,385 × 32 bytes = 524,320
bytes, excluding header/allocator overhead. One wrapping-local vector is reused
between lines and released after wrapping. Sampled whole-process RSS peaks at
51,384,320 bytes; this is not an isolated scratch-allocation measurement. Enumeration
and trimming retain their worst-case costs. No diagnostic duration or evaluation
ratio supports a runtime speedup claim.

Verification passed:

- 56,235,842 exhaustive short-sequence checks and 10,000 seed-73074 numeric cases;
  equality, decreasing padding costs, whitespace, endpoint eligibility and legal
  preference remain covered.
- 134 focused checks cover hard breaks, trimming, graphemes, bidi, adjacent
  identities, objects, zero widths, oversized first units, missing legal breaks,
  vertical-only padding and fractional widths. Actual scanner fallback matches
  exact serialized oracle geometry under C rounding, mismatched MXCSR rounding,
  FTZ, DAZ, negative/nonfinite padding and overflow-envelope rejection. Separate
  fast-math and representation rejection checks pass.
- Inherited ownership ranges, fallback, stale entries, lifecycle, cache, failure
  and objects pass. Historical gates pass 751 disabled-equivalence fixtures / 775
  blocks, 775 scan blocks, 14,272 vertical cells and 2,901 visual fragments. The
  0.05 containment tolerance and 95 near-edge snap observations remain unchanged.
- Python harness/recovery/supervisor/ownership/runtime regressions: 40 pass, one
  optional prior-schema attribution test skipped. Repository lightweight checks
  and whitespace validation are retained with the run.
- Completed-run resume into `073-candidate-implementation-resume-validation`
  preserves all 96 records and build/execution/gate journals. Fatal sentinels on
  both process-launch routes establish zero new process launches.

Preparation failures are retained separately without retries: v1 caught residual
counter code during clean-source generation; v2's new nonfinite-padding test used
in-memory JSON equality, where NaN differs from itself; v3 had a test-message type
error. V4 uses exact serialized geometry for the unsupported nonfinite case and
records that it matches. No scanner algorithm was changed after these failures,
and no alternate optimization was substituted. The fixed schedule ran only once,
in v4. Journals and available source snapshots for v1–v3 are archived separately.

[archives.json](archives.json) records every archived file SHA-256, archive size and
archive SHA-256. Archives retain both source copies, patches, reproduction tools,
configuration, identities, environment, separate build/execution/gate journals,
fixtures, raw outputs/counters, reports, audits, resume checks and preparation
history. Every archived file and archive hash was read back and verified. Inherited
exclusive/child locks, 60-second diagnostic deadlines, 4 GiB sampled RSS limits and
terminal failures remain enforced. Previous work and archives remain intact.
