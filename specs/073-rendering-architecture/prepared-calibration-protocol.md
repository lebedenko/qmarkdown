# Prepared-state calibration at 96 instances

Approved 2026-11-08 by the owner's supplied-plan implementation request, explicitly
covering these requirements, design and tasks, isolated tooling, one fixed execution
and evidence delivery. Excludes candidate comparisons, renderer/public API changes,
further sampling, commits, merges and publication. Retention remains not established
under every outcome. Historical results and tools remain immutable.

## Requirements and design

PC1: Reuse unchanged refinement and original reference executable bytes; no build.
Verify historical archives/members and pinned sources, binaries, static/runtime
libraries, fixtures and fonts before execution; missing/incompatible inputs stop.
Use original prose (prose-480-4096, zero), batch prose (same fixture, 96, telemetry
enabled), original dense (dense-240-4096, zero), in that generator order.
Original references preserve initial/reflow/repeat/metadata without added operation
telemetry. Batch preserves independent instances, shared font collection, all 96
constructed then laid out at 480; time only increasing-index layout at 497 once.
Retain full preparation/cache/geometry/mutation-isolation/generation checks.

PC2: Exactly 483 processes: three correctness then 40 contiguous four-process
groups per configuration. Validate all correctness before timing. No smoke runs,
warm-ups, retries, replacements or adaptive samples. random.Random(73082): generate
each configuration's orders by shuffling [ABBA, ABBA, BAAB, BAAB] ten times;
after all sequences, shuffle fresh configuration list each of 40 rounds. Each half
has ten of each order. A/B labels change no command/environment. Freeze commands,
identities, sources, protocol, software and inherited environment before launch.
Serial execution under existing workstation lock during ordinary shared use;
inherit affinity/frequency; 60-second deadline, terminal 4 GiB sampled RSS ceiling.
Identical parent snapshots/supervision for every configuration; durable attempted
journals; explicit unavailable telemetry reasons. Only fully validated completed
prefixes resume to fresh directories with unchanged frozen context. Failed or
ambiguous attempts are terminal and never repeated.

PC3: Require equal paragraph counts in all four processes per operation. A/B are
two-process means; D=((B1+B2)-(A1+A2))/(2*N), N=96 for batch or recorded applicable
paragraph count. Estimate median of 40 D. Whole-group percentile bootstrap: 10,000
resamples, random.Random(73083) reset per configuration, indices shared across
operations; percentile index int(p*(10000-1)). Load exact tolerances from verified
historical evidence (prose ~4819.525; dense metadata ~101542.075 ns/paragraph).
Required targets: batch prepared reflow and dense metadata. Inclusive entire
interval within symmetric tolerance AND smaller median arm timing >=100 times
maximum cell clock period/empirical resolution. Original prose reflow interval
and envelope fit are descriptive, cannot veto/establish resolution; its historical
tolerance is a diagnostic target for a different measurement mode, not equivalence.

PC4: After all 483 validate: resolved-prepared-state-control only if both targets
resolve; otherwise unresolved. Execution/identity/lifecycle/resource/evidence or
analysis failure means incomplete. Preserve every attempted launch, partial file,
validation error and completed observation in forensic reports/archives; no resolved
decision or retry permission. Keep size 96 fixed; pilot selection thresholds add
no gates. Never pool pilot data, subtract estimates, alter historical gates or claim
historical variability explained. Resolved permits only recommendation to design a
separately approved prepared-state path-isolation experiment with contemporaneous
controls. Unresolved/incomplete stops iteration. No outcome authorizes measurements.

PC5: Report raw and normalized observations, arm means/D, medians, unscaled MAD,
ranges/signs, order and first20/last20 summaries, resources separately by correctness
and timing, all reference operations and prose metadata nulls. Ratios descriptive:
suppress a group if ANY raw timing <100*maximum group period/resolution; preserve
null reasons/counts, never bootstrap ratios or omit timing/resources.

## Tasks and verification mapping

- PC-T1 (PC1–PC5): Record approval; inspect conventions and historical identities.
- PC-T2 (PC1–PC2): Add sibling runner with frozen schedule, supervision, strict
  lifecycle/telemetry validation and terminal recovery, reusing executable bytes.
- PC-T3 (PC3–PC5): Implement analysis/forensic packaging. Before execution test all
  identities/order/balance/contiguity/arm equivalence under fatal launch sentinels;
  lifecycle/cache/timing/normalization/optional telemetry/raw-delta invariants;
  reproducible bootstrap/percentiles, zeros/offsets/drift, inclusive boundaries,
  timer suppression, descriptive non-veto; altered hashes/missing/duplicate/
  misordered records, terminal/ambiguous/partial journals and prefix/completed
  recovery. Real lifecycle acceptance only in three budgeted correctness processes.
- PC-T4 (PC1–PC4): Verify pinned inputs; execute fixed 483 once after offline gates.
- PC-T5 (PC4–PC5): Byte-identical reanalysis and completed resume under fatal launch
  sentinels with zero launches; focused tests, repository lightweight and whitespace
  checks. Archive tooling, both executable bytes, frozen context, identities/raw/
  telemetry/journals/analysis/decision/check logs. Read back every member hash,
  revalidate historical preservation; update actual verification/recommendation
  and task states, including incomplete outcome if necessary. Stop after delivery.
