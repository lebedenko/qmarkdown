# Ownership runtime comparison

Feature 073 RT1–RT4 / RT-T1–RT-T4, explicitly approved by the owner's instruction
to implement the supplied plan. This sibling experiment preserves phase/ownership
sources and evidence and makes no production change.

```sh
python3 specs/073-rendering-architecture/demonstrator/runtime-comparison/test_runtime.py
python3 specs/073-rendering-architecture/demonstrator/runtime-comparison/run.py build-ci/073-runtime-workspace build-ci/073-runtime-evidence
python3 specs/073-rendering-architecture/demonstrator/runtime-comparison/report.py build-ci/073-runtime-evidence
```

Both Release executables use identical `trace.cpp`, dependencies and font inputs,
without phase counters. The original boundary implementation is compared with a
fresh boundary copy plus the retained ownership patch. Archived phase and ownership
manifests, sources and library identities are checked before copying. Build commands,
verbose driver compile commands, toolchain/dependency versions, source snapshots,
Ninja flags, executable hashes and environment details are retained.

Correctness uses 48 separate processes and archived four-operation geometry/shaping
comparisons, followed by the historical boundary/extent gates. Timing then uses
480 fresh processes, serially: ten passes of 24 adjacent baseline/optimized pairs,
alternating which binary runs first each pass. The complete schedule is durable
before execution. Reflow and repeat reuse the process's paragraphs. No filesystem
cache or independently warmed cache claim is made.

Each operation has one clock interval across applicable paragraphs. Font loading,
paragraph construction, generation snapshots, metadata copying/identity preparation,
geometry, serialization and flushing stay outside that interval. Correctness extracts
full geometry after every operation; timing never extracts it. Metadata-free
paragraphs do not participate in metadata timing, and an empty operation is null.
The clock period and a 10,000-read minimum-positive-delta probe are recorded per
process. Timer overhead is not subtracted. Width/repeat reuse and metadata reshaping
are checked outside clocks in both modes.

The inherited exclusive lock is passed to children. Every process has a 60-second
wall deadline and 4 GiB sampled RSS ceiling. Raw/partial output and terminal status
are durable. A failed/interrupted/limited process stops remaining samples for that
fixture/variant, while independent cells continue. Correctness must finish before
any timing. Geometry or shaping failures stop the study. No automatic retries,
substitution or limit increases occur.

Resume requires the same workspace and a **new** output directory:

```sh
python3 specs/073-rendering-architecture/demonstrator/runtime-comparison/run.py build-ci/073-runtime-workspace build-ci/073-runtime-resume --resume-from build-ci/073-runtime-evidence
```

Input identities, schedule, checkpoints, acknowledged raw output, fixture hashes
and completed output invariants are revalidated. Unacknowledged partial raw files
become terminal interrupted records. A complete-run resume launches no samples.

Reports treat each fixture/padding/operation separately. Median difference is the
median signed absolute-time difference (optimized minus baseline, in nanoseconds),
not a difference of medians or an unsigned distance. Ratios are median paired
baseline/optimized ratios. Descriptive percentile 95% intervals bootstrap whole
process pairs 10,000 times with fixed seed 73073. Both medians, differences and
ratios receive intervals. Ratios are suppressed when either median is below 100
times the larger clock period/empirical resolution for that cell. Missing pairs
are excluded and listed; null metadata operations stay null. Process RSS is not
ownership-allocation attribution. There is no speedup acceptance threshold.

See [retained evidence](../../evidence/runtime-comparison/README.md) for actual
outcomes, zero-padding preparation overhead, single-span results and limitations.

After verification and reporting, archive the closed evidence set:

```sh
python3 specs/073-rendering-architecture/demonstrator/runtime-comparison/archive.py build-ci/073-runtime-workspace build-ci/073-runtime-evidence specs/073-rendering-architecture/evidence/runtime-comparison
```

The archiver rechecks immutable build identities, retains reproduction sources and
build recipes, creates independently readable parts, and verifies every archived
member and archive SHA-256. Reporting and test sources added after build preparation
have separate supplemental identities rather than changing the timing identities.
