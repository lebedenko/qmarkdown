# Padded-layout optimization proposal

Status: **approved for implementation on 2026-11-06**. The owner explicitly
requested implementation of the supplied ownership plan; approval covers O1–O5
and O-T1–O-T4, with the corrections and execution contract below. This
proposal follows the approved phase diagnosis, not a new backend decision.
Historical qualification gaps, including Qt Vulkan shutdown and unavailable
presentation evidence, remain unchanged. No speedup or performance pass is claimed.

## Decision and scope

Select **precomputed shaped-run ownership** as the first bounded optimization.
Retain the exhaustive candidate scan, Unicode break priorities, trimming, bidi
reordering, emitted slices, line formatting and rendering paths. This removes a
repeated ownership search without changing which candidate wins. The measurement
record below supplies counts; they are not CPU-time percentages.

Rank the alternatives:

1. **Precompute run-to-span ownership.** The same immutable shaped runs and span
   intervals are searched repeatedly for each candidate's visual slices. A small
   run-indexed table can replace those repeated containment searches. It is local
   to a shaping generation and has a direct equivalence argument.
2. **Reuse fragment information.** Whole-range memoization would retain many
   candidate ranges and potentially large slice vectors; the scan examines many
   distinct ranges. Reflow can change endpoints, trimming and bidi order. Defer
   candidate-range caching. A future final-line cache would be bounded by visible
   lines, but this study's final-formatting work does not make it the first target.
   Any such cache must include exact range, shaping generation, metadata generation
   and formatting state, and be discarded on reflow/metadata mutation as applicable.
3. **Reduce candidate scanning.** Long spans expose large candidate counts even
   with one owner. This remains a separate bottleneck after ownership improvement.
   Padding can appear/disappear when visual fragments merge or trailing whitespace
   is trimmed; required widths are non-monotonic. Neither first-overflow termination
   nor binary search is justified. A future proposal needs a conservative proven
   bound or a semantics-preserving incremental evaluation, with counterexamples
   and differential tests before approval.

## Proposed requirements

- O1: Preserve exact geometry, glyphs, visual slices, source mappings, selection,
  hit tests, object placement, Unicode boundaries and overflow decisions.
- O2: Replace repeated ownership searches only when ownership is independent of
  the queried subrange; retain the legacy search for any unproven run.
- O3: Keep the table private, per paragraph and bounded by shaped run count. Do
  not populate shared paragraph caches with boundary metadata or add dependencies.
- O4: Preserve shaping reuse on width/repeat and existing metadata invalidation.
- O5: Verify reduced owner comparisons separately from clocks. Reuse this bounded
  diagnostic matrix and historical correctness gates; a speedup claim requires
  separately agreed uninstrumented timing evidence.

## Mechanism, lifetime and invalidation

Store one `size_t` entry per shaped run: an index into the paragraph's span vector,
`no owner`, or `use legacy search`. Do not store pointers into runs or spans. Resolve
identity and horizontal padding from the indexed span when producing each slice.
Preserve original first-match span ordering.

Build the table once after successful shaping has finalized all text, fallback and
placeholder runs and before line breaking can query visual slices. For each run,
scan spans once. Record the first fully containing span, or no owner. If any span
edge lies strictly inside the run's source interval, mark the run for the original
clipped-range search. This conservative fallback avoids depending solely on an
assertion that every future run is split at all boundaries. Construction is O(R×S)
using the existing containment comparisons; a sorted sweep is unnecessary for this
first change. The table eliminates repeated searches, not run intersection scans,
allocations, reorders or candidate visits.

Reuse across width changes and unchanged layout while runs and metadata are
unchanged. Clear before every run replacement/clear, shaping restart, cache restoration
and metadata replacement/removal; rebuild only after successful shaping.
Failed/empty shaping must leave no stale entries. Empty metadata can use a shared
logical no-owner result with no allocated table. An equal metadata setter retains
its existing no-op semantics. Audit cache restore paths explicitly: metadata-free
cache reuse must not inherit ownership from a prior metadata generation. Track
validity with the run generation and a private validity flag, not vector length
alone. Width-only layout must not rebuild this table.

Memory is R×sizeof(size_t), plus vector header and validity bookkeeping, with no
candidate-dependent growth. On this 64-bit host 356 runs require 2,848 payload
bytes. Capacity can be released on metadata removal and must not grow per reflow;
retained capacity policy and allocator overhead must be stated in verification.

## Correctness argument and limits

The boundary shaper limits input/context at span edges while preserving paragraph
bidi and Unicode break behavior. For a run with no internal span edge, every
nonempty intersection with a queried range has the same containing span (or no
span) as the complete run. Thus replacing the clipped-range containment search
with its precomputed result preserves identity and padding. An empty range inside
such a run has the same ownership; ranges exactly at run endpoints are excluded
by the existing intersection predicate. Preserve that predicate unchanged.

A run with an internal edge is explicitly sent through the existing search, so
placeholder/fallback or future unsplit-run behavior is preserved without relying
on the invariant. Nonoverlap and ordered source spans remain the boundary adapter's
validated input contract. Slice ordering, adjacent identity coalescing and before/
after padding remain byte-for-byte the existing algorithm. No monotonic-width
assumption enters this proposal.

## Approved implementation tasks

- O-T1 (O1–O3): Audit all run/cache/reset paths; add the private table and conservative
  fallback, with focused full-run/clipped/empty-range/adjacent-span/object tests.
- O-T2 (O3–O4): Test metadata identity/padding/range changes, removal/re-addition,
  equal setters, failed/empty shaping, width reuse and unchanged repeat. Verify
  stale generation rejection, bounded capacity and no per-reflow allocation.
- O-T3 (O1, O5): Run historical gates and the same 24 paired diagnostic variants;
  require exact geometry/source equivalence, unchanged candidate counts, reduced
  fitting owner comparisons and no fallback on the existing supported fixtures.
  Retain any fallback cases and their legacy equivalence evidence.
- O-T4 (O5): Record actual work-count reduction and memory observations, preserve
  all performance qualification gaps, run lightweight/regression checks, and
  present results. No Qt integration, backend selection, merge or publication.

Owner approval covers O1–O5, this mechanism and O-T1–O-T4. Metric-only
`resetContext()` must retain ownership. Use a private run revision and validity
flag; reject stale/invalid entries via legacy search. Allocate a fresh run-sized
vector per rebuild, release old storage and release on invalidation. Empty metadata
allocates nothing. Record actual capacity and payload bytes. Width/repeat must
perform no rebuild or allocation. Resetting diagnostic counters changes no state.

Execution: fresh hash-verified pinned copies and a repository-owned patch; separate
instrumented/uninstrumented binaries. Run twelve fixtures × zero/padded × two
binaries serially (48 processes), four operations each, comparing exact geometry,
glyph positions, slices and mappings to each other and hash-validated retained
phase traces. Preserve candidate/trimming/run/reorder/slice counts; supported
fixtures require zero fallback. Include build span examinations in search totals.
Retain lock inheritance, 60-second deadline, 4 GiB RSS ceiling, immutable evidence,
validated new-directory resume and terminal interruptions; no automatic retries.
Historical boundary/extent gates and focused lifecycle/recovery regressions precede
completion. Stop on unexplained invariants. Archive raw traces, patch, identities
and logs with hashes. No speedup qualification, fragment caching, candidate-scan
change, adapter/paint work, native matrix, Qt integration, merge or publication.

## Measured basis

The [phase evidence](evidence/phase-experiment/README.md) contains the complete
initial-work table and phase-normalized machine-readable counts. Dense 4 Ki/480
initial layout performs 245,229 candidate visits, 87,301,524 shaped-run visits and
3,937,311,708 owner tests; final formatting performs 58,117 owner tests. The 1 Ki
→ 4 Ki owner-test growth is 242.634×, compared with 15.600× candidate growth.
Independent span counts amplify ownership work while candidate counts stay close.
All 48 traces and historical correctness gates pass. These facts support the
selected narrow mechanism; they do not predict a speedup percentage.

## Ownership optimization actual result — 2026-11-06

O-T1–O-T4 are implemented and verified in the isolated ownership experiment.
The owner-approved run revision/table mechanism retains metric-only reset/reflow
state, releases invalidated storage and falls back conservatively for unsplit or
stale entries. All 48 traces / 192 operation records match both peers and archived
baseline geometry/source data; historical boundary and extent gates pass. Existing
phase counters are unchanged. Ownership work totals 78,702,579,266 legacy comparisons
versus 1,756,528 build examinations and zero fallback comparisons. Width/repeat
have zero rebuilds; largest table payload is 9,800 bytes. Zero-padding paths add
preparation work where the baseline had no ownership searches; no benefit is claimed
for those paths. Detailed counts, memory limits, tests, retained preparation failure
and qualification boundaries are in [ownership evidence](evidence/ownership-experiment/README.md).
No speedup or production-performance acceptance is implied.
