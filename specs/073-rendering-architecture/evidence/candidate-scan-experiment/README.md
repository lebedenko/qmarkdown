# Candidate-scan proof evidence — 2026-11-06

The approved isolated suffix-bound experiment is complete. Recommend a separately
approvable retained implementation and later uninstrumented runtime study; this
iteration retains no production optimization and makes no speedup claim. See the
[recommendation](../../candidate-scan-recommendation.md) and
[reproduction/proof](../../demonstrator/candidate-scan-experiment/README.md).

## Final differential evidence

`073-candidate-evidence-v3` is the final pinned run. Its schedule records twelve
fixtures × zero/padded × oracle/shadow/prototype = **72 fresh processes**, each with
initial, width +17, repeat and metadata mutation/layout: **288 operation records**.
Every mode matches the archived runtime study's 24 baseline correctness variants
exactly for line geometry, glyph positions, visual slices, source mappings and
shaping deltas after every operation, and matches its oracle peer. Formatting
counters are unchanged. Width/repeat have no reshaping or ownership rebuilds;
metadata retains the original invalidation behavior. Supported fixture scans have
zero arithmetic or ownership fallback. All recorded C/MXCSR rounding controls are
nearest, with FTZ/DAZ disabled.

Shadow separately confirms that all **44,946,905** hypothetically skipped candidates
fail to fit. Its validation work is excluded from proposed-path phase counters.
Across final prototype operations: 45,220,272 cheap endpoint visits, 15,853,892
trimming subtractions, 45,213,218 suffix comparisons, 273,367 expensive padding
evaluations and 44,946,905 eliminated evaluations. Initial padded long-span falls
from 3,842,986 to 16,852 evaluations; spans-32 from 3,346,678 to 16,468. Both also
reduce evaluation work on reflow and metadata layouts. Zero-padding scanner paths
have zero endpoint preprocessing, trimming and scratch allocation. Fixed diagnostic
arithmetic/span guard checks outside the scanner are not endpoint work and remain
a placement issue for any future retained implementation.

Largest scratch capacity is 16,385 entries, with maximum size 16,384; each entry is
32 bytes on this host. Reserved payload peaks at **524,320 bytes per paragraph**,
excluding vector header/allocator overhead. A local vector is reused per line and
released after wrapping. No persistent cache exists. Whole-process sampled RSS
peaks at 51,335,168 bytes. These work/storage facts do not establish runtime effect.

## Verification

- 56,235,842 synthetic checks exhaust short width/whitespace/endpoint-eligibility
  and legal-break patterns through length four, using widths {-1,0,1,2}, three
  representative nonnegative padding patterns and exact/adjacent equality widths.
  Two named checks cover padding decreases after overflow and trimming decreases.
- 10,000 numeric cases with seed 73074 cover random binary32 magnitudes, subnormal
  and extreme values, guarded infinities/NaNs, negative padding and rounding modes.
  Direct tests reject non-binary32 representation, fast math, FTZ, DAZ and mismatched
  MXCSR rounding. Equality uses strict `>` and preserves the exact-width candidate.
- 129 focused layouts/checks cover whitespace, hard breaks, combining graphemes,
  mixed bidi, adjacent identities, objects, zero-width text, oversized first units,
  missing legal breaks, vertical-only padding and fractional width boundaries.
  Actual unsupported-rounding and negative-padding fallback geometry matches oracle.
- Inherited ownership range/fallback/stale-entry/lifecycle/cache/failure/object
  tests pass. Historical gates pass 751 disabled-equivalence fixtures / 775 blocks,
  775 scan blocks, boundary-minimal cases, 14,272 vertical cells and 2,901 visual
  fragments. Original containment tolerance 0.05 and 95 near-edge snap observations
  are preserved; no historical observation is reclassified.
- Candidate harness rejection tests: 4 pass. Ownership verifier: 4 pass. Ownership
  recovery: 4 pass, 1 optional attribution test skipped because its 48-process schema
  belongs to the prior ownership experiment. Supervisor/recovery tests: 17 pass.
  Runtime harness regressions: 11 pass. Final differential report checks its own
  72-process schema and all counters directly.
- Completed-run resume validates into `073-candidate-resume-validation`, with all
  72 records preserved and no new diagnostic processes or command-journal changes.
  Repository lightweight checks and whitespace validation are recorded in the
  archived verification results.

## Preserved development history

The first preparation attempt failed before any trace because pinned Skia disables
C++ exceptions. Its sources/commands/logs are retained. Fatal private diagnostics
were corrected to use stderr plus abort, in a new workspace. This is a preparation
correction, not a trace retry.

`073-candidate-evidence-v2` then completed its 72-process matrix, gates and work
report without counterexamples. A subsequent guard audit identified the need to
check MXCSR rounding bits explicitly alongside `fegetround()`. Its complete source,
identity, raw and journal record is preserved under `history/`; no old file was
rewritten. V3 adds that conservative guard, a mismatched-MXCSR rejection test and
raw environment fields, and reruns the final fixed schedule in fresh directories.
Thus there are 144 diagnostic processes across the two completed development/final
runs; only v3 supplies the final matrix above. No failed geometry, shaping or cutoff
proof was retried or discarded, and no runtime matrix was run.

## Provenance

The [archive manifest](archives.json) lists independently readable tar parts,
SHA-256 values and per-file hashes. Archives contain raw outputs/counters, fixture
inputs, source snapshots, candidate patch, verified ownership baseline identities,
environment, commands, compiler/build configuration, reports, preparation history
and resume/verification results. Every member and archive hash was verified after
creation. Existing archives, uncommitted work and shipping sources are preserved.
No Qt integration, merge, publication or performance acceptance occurred.
