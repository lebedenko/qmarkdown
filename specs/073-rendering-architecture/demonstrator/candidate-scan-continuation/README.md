# Cache-aware candidate-scan runtime continuation

CR1–CR5 / CR-T1–CR-T5, approved by the owner’s supplied-plan implementation
request on 2026-11-06; CC1–CC4 continuation approved 2026-11-07. Existing experiments and archives remain unchanged.
No shipping APIs, Qt integration, ownership changes, merge or publication.

```sh
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-continuation/test_runtime.py
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-continuation/run.py build-ci/073-candidate-continuation build-ci/073-candidate-continuation-evidence
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-continuation/validate.py build-ci/073-candidate-continuation build-ci/073-candidate-continuation-evidence
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-continuation/report.py build-ci/073-candidate-continuation-evidence
```

Fresh Release copies of verified ownership sources differ only by candidate.patch.
Copied build outputs are cleaned before compiling both arms. Source differences,
driver flags, Skia flags, dependency libraries and nm binary symbols are audited
before execution. Driver sources are identical. Affinity, load, tools, dependencies,
C rounding and MXCSR state are retained. System settings remain unchanged; caches
are uncontrolled and no independently cold-cache claim is made.

The persisted schedule contains 88 correctness then 1,760 timing processes, twenty
adjacent baseline/retained pairs for each of 22 fixtures × zero/padded, alternating
order each pass. Twelve historical manifests remain byte-identical. Ten new small
fixtures use the original generator, five kinds at 64/256 UTF-16 units and width
480, each containing 32 separately constructed paragraphs. All operations retain
shaping lifecycle checks. Correctness extracts exact geometry/glyphs/visual slices/
mappings after each operation; timing never queries geometry. Existing cases match
hash-validated archives; small cases match the exhaustive baseline.

Each initial, width +17, unchanged repeat and metadata identity mutation/layout
operation uses one steady-clock interval. Layout, shaping, cutoff guards,
preprocessing, scratch allocation/release and metadata application are inside.
Font loading, paragraph construction, metadata copies, generation snapshots,
geometry queries and serialization are outside. Metadata-free operations are null.

10,000 bootstrap resamples, seed 73075, use whole process pairs with identical
indices for all operations. Every fixture/padding/operation is reported separately.
Primary padded long and spans-32 initial/reflow/metadata require >=10% median
paired improvement and positive 95% lower bound. Every other applicable cell’s
95% upper median paired overhead per paragraph must be <=max(5% baseline median
per paragraph, 2 µs). Ratios below 100×max(period, empirical resolution) are
suppressed; timer-limited primary cells fail. Absolute regression gates remain.
Missing samples or correctness prevent acceptance. No additional samples or tuning.
RSS is whole-process evidence alongside the verified cluster-bounded scratch vector.

Inherited exclusive lock and child inheritance, 60-second deadlines and 4 GiB
sampled RSS ceiling apply. Any failure terminates the study without retries.
Build, execution and historical gate journals are separately checkpointed. Resume
requires a new output directory; all identities, raw hashes and exact comparisons
are revalidated. Completed-run resume is tested with fatal process-launch sentinels.
Archives retain sources, patch, configuration, identities, commands, raw outputs,
reports and failures, and read back every member/hash.

Actual results are recorded separately in Feature 073 verification and evidence.

Correctness installs the existing paragraph-cache checker. Metadata-free initial
layout must report a miss/add with delta 1 or a hit with delta 0; metadata-bearing
layout must shape once without cache events. Reflow/repeat reuse shaping;
metadata identity mutation reshapes affected paragraphs once. Timing installs no
checker and must match its correctness-run signature. Historical fixtures still
match archived geometry and shaping exactly. The terminal predecessor is immutable.
