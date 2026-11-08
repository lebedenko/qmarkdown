# Measurement procedure on the development workstation

## Status and scope

The owner accepted the measurement-characterization direction on 2026-11-08 with
this correction: there is no dedicated benchmark machine; these experiments will
always run on this machine. This document records that accepted procedure update
within Feature 073. It does not reopen the completed calibration runs.

The direction below is approved. Exact pilot parameters and the resulting fixed
calibration protocol still need requirements/design/tasks approval before harness
implementation or new measurements, following the repository development workflow.
No new benchmark has been run under this procedure. Retention remains not established.

## Machine and execution conditions

- Use this development workstation for every experiment. A dedicated machine,
  exclusive host access or an idle desktop is not a prerequisite. Ordinary shared
  use is part of the execution environment.
- Record a stable, non-secret workstation identifier and CPU model/topology, RAM,
  kernel, toolchain, runtime libraries, source/binary hashes and fixture hashes.
  Record software or hardware changes between runs; the same physical machine
  does not imply the same execution conditions or comparable binaries.
- Inherit the existing affinity and frequency policy by default. Record allowed
  CPUs, observed CPU placement at process/operation boundaries, governor and boost
  policy where readable. Boundary observations cannot rule out migrations between
  them. Any process pinning must be predeclared and checked for validity on this
  workstation; it is not assumed to provide an isolated core.
- Record available frequency/thermal readings and system load before and after
  each process, plus process CPU time and voluntary/involuntary context-switch
  deltas around operation batches. Keep telemetry outside timed regions and record
  its sampling scope. Frequency snapshots do not measure the frequency of every
  instruction. Record unavailable counters explicitly, without inventing values
  or requiring privileged monitoring to proceed.
- Do not change the system governor, disable boost, terminate unrelated workloads,
  flush caches or require the owner to stop using the machine. Record observable
  interference; retain affected samples. Do not select favorable periods, remove
  slow samples or rerun groups based on observed performance.
- Preserve serial benchmark launches, the execution lock, 60-second process
  deadlines, 4 GiB sampled RSS ceiling, immutable inputs and terminal journals.
  Failed or ambiguous attempts remain terminal. Resume only a validated completed
  prefix into a fresh directory; never repeat an attempted launch.

## Characterize before selecting a fixed protocol

1. Inspect existing revision observations against parent monotonic timestamps,
   ABBA/BAAB order, arm durations and chronological position. Report descriptive
   patterns without attributing them to CPU, scheduling or thermal causes that
   were not recorded. This stage needs no new benchmark execution.
2. Design a bounded baseline-only pilot focused on prose reflow. Predeclare the
   finite batch-size set, process count per size, schedule/seed, resource bounds,
   duration/precision objectives and deterministic selection or stop rule before
   launching. Never select a batch size because its A/B difference happens to
   pass an envelope. Do not keep increasing the budget until a result passes.
3. Assess genuine reflow batching, lifecycle correctness, duration, variability,
   memory use and telemetry overhead. Preserve all pilot samples and unsuccessful
   sizes. Pilot data selects a protocol; it is not the subsequent calibration data.
4. Freeze the selected batch size, preparation/cache policy, telemetry, schedule,
   process budget, normalization, statistical procedure and decision criteria for
   a new baseline/self calibration. If the pilot does not support a suitable
   protocol within its fixed budget, report that limitation and stop.

Exact pilot sizes/counts, minimum timed duration, selection thresholds and the next
calibration budget are deliberately not supplied by this direction-only approval.
They belong in the implementation plan, not in runtime adaptation or hidden defaults.

## Time genuine operations and preserve their meaning

Prepare multiple independent paragraph instances outside the timed region. Bring
each to the prescribed initial width and verify the required lifecycle. In a reflow
batch, time exactly one width-changing layout to width+17 for each instance. Do not
substitute repeated layout calls at the already-current width: those measure the
unchanged-layout path. Verify that reflow does not reshape and that all instances
actually traverse the intended state transition.

Define and test font/paragraph-cache sharing and preparation order. Identical text
in multiple instances can share cache state; batching is not presumed equivalent
to independent fresh processes. Record batch size and total applicable paragraphs,
and normalize batch elapsed time by that total. Include the fixed iteration loop
in the timing boundary; do not subtract a measured empty-loop cost.

Keep fresh-process, single-operation observations separately visible as a distinct
measurement mode with its own declared budget. Label the batched measurement as
prepared-state throughput; do not call it cold-start latency. Do not pool these
modes or silently replace the historical single-operation metric. If other
operations are later batched, specify their lifecycle independently: unchanged
repeat, initial layout and metadata invalidation are different state transitions.
Prose metadata remains null. Geometry queries, cache checkers, assertions and
telemetry collection stay outside timed regions; correctness runs validate the
same state transitions separately.

## Grouping, statistics and decisions

Retain fresh independent process launches and balanced randomized contiguous ABBA
or BAAB groups, with identical baseline executable, arguments and environment for
both logical arms. The number of groups and random seeds must be frozen before
launch. Inner batch operations are not independent statistical samples.

For a fixed batch size, require equal total applicable paragraph counts N across
all four processes. Each arm's group timing is its two-process mean. The normalized
group difference is ((B1+B2)−(A1+A2))/(2×N). Report the median of group differences,
whole-group bootstrap intervals, raw observations, unscaled MAD, ranges, signs,
per-arm timings, order summaries and chronological summaries. Retain the historical
percentile-index convention and 100× clock-period/empirical-resolution ratio rule;
passing that timer rule does not establish adequate statistical precision.

Keep historical gates and archived exact reference budgets unchanged. State whether
a new metric is sufficiently comparable to use those budgets as diagnostic targets;
a passing batched result cannot retroactively qualify historical fresh-process
results. No pooling runs, subtracting control estimates, retrospective gate changes
or claim that historical variability has been explained.

Run the newly frozen baseline/self control before candidate comparisons. Only if
its predeclared precision criteria resolve may a separately approved path-isolation
experiment follow, with baseline, unchanged candidate, isolated candidate and
contemporaneous controls under the same new procedure. Otherwise stop and report
the attainable precision on this workstation. A fixed budget is neither a power
guarantee nor proof that neighboring groups are statistically independent.

## Verification and evidence

Before launching a pilot, test batch lifecycle/state transitions, cache behavior,
normalization, nulls, schedule balance/contiguity, telemetry-unavailable handling,
timer suppression, synthetic offsets/drift, missing/duplicate/misordered records,
identity/hash rejection and terminal recovery. Quantify telemetry overhead in the
predeclared characterization design rather than assuming it is negligible.

Archive source/binary identities, machine/environment records, the frozen pilot
and calibration protocols, commands/schedules, raw outputs and telemetry, journals,
analysis, selection decision and checks in separate evidence directories. Verify
member hashes, deterministic reanalysis and completed-run resume with fatal launch
sentinels. Preserve historical evidence and record actual results separately in
[verification.md](verification.md).

The previous [revision report](evidence/candidate-scan-calibration-revision/report.md)
remains unresolved. This procedure update introduces no renderer/API changes,
measurements, candidate comparison, commits, merges or publication.
