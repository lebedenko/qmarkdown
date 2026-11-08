# Draft: prepared-state prose reflow batching pilot

## Approval boundary

**Draft requiring separate requirements/design/tasks approval for implementation
and execution. No harness has been built and no pilot measurement has run.**
The 2026-11-08 supplied-plan request approves offline characterization and delivery
of this proposal only. Retention remains **not established**. Historical protocols,
gates, sources, executables and evidence remain unchanged.

## Target, configurations and exact budget

Use the existing immutable `prose-480-4096` fixture, width 480, zero padding. Its
single paragraph block is replicated into independent paragraph instances. Batch
size is the number of instances: 1, 16, 64 or 256; applicable paragraph count N is
therefore the batch size. Time genuine width-changing reflow from 480 to 497.

Nine configurations, in generator order:

1. Unchanged original fresh-process baseline executable (separate reference).
2. Batch 1, operation telemetry disabled, then enabled.
3. Batch 16, operation telemetry disabled, then enabled.
4. Batch 64, operation telemetry disabled, then enabled.
5. Batch 256, operation telemetry disabled, then enabled.

The original reference retains its existing four operations: initial, reflow,
unchanged repeat and metadata (prose metadata null). It retains its existing
instrumentation and command contract. The new executable measures only prepared-state
reflow, including batch size 1; this is a distinct configuration from the reference.
A single new harness binary supports the telemetry switch; neither arm selects
candidate code. No renderer or public API changes are proposed.

Exactly **297 processes**: first one correctness process per configuration in the
above order (nine total); then eight four-process timing groups per configuration
(288 total). Every configuration runs its entire declared budget unless a terminal
failure stops the pilot. Correctness processes are excluded from timing statistics,
but included in per-configuration resource maxima and terminal supervision.
There are no warm-up processes, retries, replacements or automatic extra samples.

## Frozen schedule and command contract

Use Python `random.Random(73080)`. In configuration order, independently shuffle
`[ABBA, ABBA, BAAB, BAAB]` twice per configuration and concatenate the blocks.
After all order sequences are generated, for rounds 0 through 7 shuffle a fresh
copy of the nine configurations using the same RNG. Emit the configuration's four
commands contiguously in its round's order. Each configuration has four ABBA and
four BAAB groups; each four-round half has two of each. A/B labels exist only in
parent journals; commands and environments within a configuration are identical.
All launches are serial under the existing execution lock.

The offline [schedule generator](demonstrator/candidate-scan-characterization/analyze.py)
writes 297 data-only command specifications. The original executable/fixture paths
are the pinned continuation paths. The literal `<draft-prepared-reflow-executable>`
is a non-executable placeholder for the proposed harness, whose CLI is:

```text
<draft-prepared-reflow-executable> FIXTURE zero correctness|timing --batch-size N --operation-telemetry disabled|enabled
```

Before future execution, build only after approval, pin the harness source/binary,
compiler/runtime/fonts/fixture identities and runtime/generator versions, substitute
the new absolute binary path in every draft command, and freeze the complete command
list, schedule and environment fingerprint. No command/environment difference may
encode the logical A/B label. The placeholder is not an existing binary or permission
to implement one. Preserve the original reference binary byte-for-byte.

## Preparation, cache policy and timing boundary

Each process creates one font collection using the original loader, explicit font
provider and disabled fallback. Keep the existing shared paragraph/font-cache policy;
do not disable, clear or prime the cache separately. Construct independent paragraphs
in increasing instance index, then initially lay out all instances at width 480 in
increasing index. Retain all instances until after the timed batch. Shared text/cache
state is intentional: this measures prepared-state throughput, not fresh-process
latency. Record effective cache settings and instance/paragraph counts.

Record each instance's shaping generation and initial state outside the timer.
One steady-clock interval surrounds a single increasing-index loop that calls
width-497 layout exactly once on every instance. Include the iteration loop; do not
subtract empty-loop overhead. No repeated unchanged-width layouts, setup, geometry
queries, assertions, CPU/resource sampling, serialization or cache inspection
belong inside this interval. After the interval, verify unchanged shaping generation
for every instance and the expected final width/state. Record raw elapsed ns, N,
clock period, empirical positive-delta resolution and elapsed ns/N.

The separate correctness process performs the identical preparation/transition and
checks every instance's geometry against the original reference reflow, zero padding,
instance independence and generation invariants. Validate cache behavior outside the
timer using available cache inspection and a separate correctness-only mutation of
one instance after comparison; verify other instances are unchanged. Do not assume
that shared-cache hits imply shared mutable paragraph state. Timing processes retain
lightweight generation/state checks outside the timer; full geometry is correctness-only.
A failed lifecycle/cache/geometry assertion is terminal, never an excluded sample.

## Telemetry comparison and same-machine policy

Use this same development workstation under ordinary shared use. Inherit affinity,
frequency/governor and boost policy. No pinning, governor changes, disabling boost,
cache flushing, unrelated workload termination, exclusive access or quiet-period
selection. Record stable non-secret machine identity, CPU/topology, RAM, kernel,
allowed CPUs and available policy readings. Record environment values by safe
allowlist plus hashes; never archive secrets.

Both new modes and the reference use identical parent-side supervision and snapshots:
before and after each process, record available per-CPU frequency, thermal sensors
and load averages, monotonic timestamps, collection durations and sampling scope.
Continue sampled process RSS monitoring and parent start/end/duration records.
Snapshot failures produce explicit unavailable fields with reasons, not zero values.
These snapshots are outside the process operation timer, but may affect its environment.

Enabled mode additionally samples CPU placement (`sched_getcpu`), process CPU time
and voluntary/involuntary context switches (`getrusage`) at both operation boundaries,
outside the timer. Pre-sampling completes before the timer starts; post-sampling
begins after it stops. Timestamp each collection and record its duration; retain
raw boundary values and deltas. Unavailable fields are explicit null plus reason.
CPU-time/context-switch deltas cover the boundary window including sampling/timer
bookkeeping, not exactly the timed loop. Boundary placement cannot rule out intervening
migration. Frequency/thermal snapshots do not measure every instruction's conditions.
Disabled mode has no operation-boundary sampling; it still has parent snapshots,
timer-resolution metadata and outside-timer lifecycle checks. Instrumentation
collection duration is descriptive and is never subtracted from measured time.

For each batch size and round, compare enabled and disabled four-process mean batch
times and ns/paragraph: enabled minus disabled and relative difference when the
100× clock-period/resolution rule permits. Summarize the eight matched-round
differences with median, unscaled MAD and range. Also report timing medians/MAD,
process duration/RSS and available telemetry/collection-duration summaries per
configuration. Pair by round and size, not A/B envelope or exact clock coincidence;
randomized configuration order means observations are not simultaneous. Report
instrumentation effects separately, descriptively, with no significance tests,
causal attribution, baseline/self calibration claim or pooling with the reference.

## Selection, stopping and interpretation

For each enabled size, use all 32 timing processes' unnormalized batch elapsed ns.
Compute median and unscaled MAD (median absolute deviation from that median).
Choose the **smallest** enabled size satisfying all of:

- Median timed batch duration is at least 10,000,000 ns (10 ms), inclusive.
- MAD / median is at most 0.10, inclusive.
- Maximum parent-observed process duration, including correctness, is strictly
  below 30 seconds.
- Maximum sampled RSS, including correctness, is strictly below 1,073,741,824 bytes
  (1 GiB).

Otherwise select no size and stop. The selection occurs only after all 297 processes
complete validly. A terminal failure means an incomplete pilot, no selection and no
automatic continuation. Passing/failing an A/B envelope never selects or rejects a
size. Disabled configurations and the original reference cannot be selected.
Missing optional telemetry does not remove samples or veto selection; a missing
required timer, lifecycle or resource record is terminal. Sampled RSS is explicitly
not a guarantee about unobserved peaks. No early stop for favorable selection results.

Preserve the 60-second per-process deadline and 4 GiB terminal sampled RSS ceiling
with the historical supervisor's boundary semantics. The stricter 30-second/1-GiB
criteria affect selection only. Durable attempted-launch journals precede every
launch; failed/ambiguous attempts remain terminal and cannot be retried. Any allowed
resume validates a completed prefix into a fresh output directory and an identical
frozen command/environment/affinity context before continuing unattempted entries.
A completed-run resume must use fatal launch sentinels and launch nothing.

Report all eight groups/configuration, raw process records, A/B arm means, normalized
D = ((B1+B2)−(A1+A2))/(2×N), medians, unscaled MAD/range/signs, order and first-four/
last-four summaries. These are descriptive pilot outputs; no new bootstrap or
acceptance envelope is used. Keep reference operation cells separate, preserving
metadata nulls and timer-limited ratio suppression (100× maximum period/resolution).
Inner instances are not independent statistical samples.

A selected size is **provisional prepared-state throughput**. It neither qualifies
historical results nor supplies the subsequent calibration dataset. Any later
baseline/self calibration must freeze a separate protocol and budget and receive
separate approval; no new sampling follows automatically from this pilot.

## Future implementation and verification tasks (not authorized)

- Implement the isolated harness/telemetry and journal integration only after
  approval; test genuine width changes, instance independence, shared caches,
  geometry/generation, normalization, unavailable telemetry and timing boundaries.
- Test selection at inclusive/exclusive thresholds, no qualifying size and terminal
  failure; test matched-round effects, ties/nulls, synthetic offsets/drift and
  timer suppression. Verify the frozen 297 commands and balance/contiguity.
- Test missing/duplicate/misordered records, source/binary/raw hash changes,
  failed/ambiguous journals and no-retry prefix recovery with fatal launch sentinels.
- Only after approved preflight checks, execute exactly the declared budget once;
  retain all observations and separately label correctness, reference and batch modes.
- Verify deterministic reanalysis/completed resume; archive all identities,
  sources, raw data, environment/telemetry, journals, schedule, selection and check
  logs with per-member read-back hashes. Seek separate calibration approval afterward.
