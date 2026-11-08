# Cache-aware runtime continuation — retention not established

CC1–CC4 approved 2026-11-07 by the owner's supplied-plan implementation request.
The terminal predecessor and its archives remain unchanged (all 2,605 source/build
identities verified). The harness correction is isolated in the sibling
[candidate-scan-continuation](../../demonstrator/candidate-scan-continuation/README.md).

Both diagnostic arms confirm normal paragraph-cache reuse: small-prose initial
paragraph 0 misses and shapes once; paragraphs 1–31 hit and restore shaping with
delta zero. Correctness now verifies those cache events; timing has no checker or
geometry queries and matches the correctness shaping signature. Metadata-bearing
initial layout and identity changes still shape exactly once; width/repeat reuse
shaping. No renderer patch, fixture, operation boundary or threshold changed.

All 88 correctness and 1,760 timing processes completed, with no retries. Exact
paired geometry/shaping and archived historical comparisons pass. The completed
validator confirms 924 adjacent pairs, symmetric uninstrumented builds/libraries
and 252 null metadata records. Maximum sampled RSS: 43.94 MiB; maximum process
wall time: 5.17 seconds (limits: 4 GiB and 60 seconds).

The fixed seed-73075, 10,000-resample report has complete sampling: 162 passing
cells, eight overhead-budget failures and six inapplicable metadata cells. All
six primary padded long/spans-32 operations pass their 10% improvement/positive
95% lower-bound gates. Long initial/reflow/metadata improve 60.04% / 60.57% /
60.20%; spans-32 improve 92.35% / 92.64% / 92.41%. All applicable small-fixture
cells pass. Forty-two nonprimary cells are timer-limited; their ratios are
suppressed and absolute overhead gates remain active. No primary is timer-limited.

**Retention is not established:** eight other cells exceed the upper confidence
bound allowed for per-paragraph overhead. Their intervals also include zero, so
this is a failure to establish the declared regression budget, not proof of a
nonzero slowdown. No extra samples, tuning or alternative optimization were run.

| Fixture | Padding | Operation | 95% upper overhead (µs/paragraph) | Limit (µs/paragraph) |
| --- | --- | --- | ---: | ---: |
| prose-480-4096 | zero | reflow | 4.945 | 4.820 |
| prose-480-4096 | padded | reflow | 5.890 | 4.914 |
| dense-240-4096 | zero | metadata | 112.100 | 101.542 |
| long-480-16384 | zero | initial | 223.194 | 221.091 |
| long-480-16384 | zero | reflow | 29.686 | 20.465 |
| long-480-16384 | zero | metadata | 176.127 | 169.049 |
| spans-480-1 | zero | reflow | 20.267 | 19.757 |
| spans-480-1 | zero | metadata | 201.596 | 164.677 |

The [full report](report.md) and [machine-readable summary](summary.json) preserve
all 176 cells, paired absolute differences, per-paragraph estimates, intervals,
timer resolution and process RSS. No pooled headline speedup is claimed.

Verification: 11 focused Python tests, 44 real-driver fixture/padding checks,
9 recovery tests and 8 supervisor/performance tests pass. Historical geometry
checks cover 751 fixtures / 775 blocks, 41 brackets / 147 transition-probe blocks;
extent checks cover 14,272 cells and 2,901 fragments with unchanged tolerances.
Repository lightweight checks pass (87 tests plus documentation/configuration).

Completed-run resume and archive read-back outcomes are recorded in the feature
[verification record](../../verification.md). Archives include diagnostic and
regression evidence, source identities, renderer sources/patches, fixed schedule,
build and execution journals, raw outputs and reports. Build-only errors and the
diagnostic journal formatting error are retained separately from the study.

This result concerns only the isolated candidate-scan cutoff. Visual acceptance,
fractional DPR, Qt 6.8 Vulkan shutdown diagnosis, production integration, merge
and publication remain separate and unperformed.

Archive read-back completed: **4,343 files in three archives**, with every member
and archive hash verified. The [archive index](archives.json) contains all hashes.
Completed-run resume restored 1,848 records with zero process launches and
byte-identical journals under fatal launch sentinels.
