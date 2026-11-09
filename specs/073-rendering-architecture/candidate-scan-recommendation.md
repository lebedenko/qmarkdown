# Candidate-scan cutoff recommendation

Status: **runtime study terminated; retention is not established**.
The owner approved the supplied runtime plan on 2026-11-06.

## Runtime decision — 2026-11-06

Do not infer useful runtime benefit or regressions from this study. Both clean
Release builds and pre-execution audits pass, and all 48 historical correctness
processes match exact paired and archived traces. The first new small-prose
baseline process fails the inherited shaping-generation assertion. Shared paragraph
cache restoration is an identified source-level explanation for that driver
assumption; the failing paragraph/delta is not serialized. Thirty-nine correctness
and all 1,760 timing processes remain unlaunched. All primary/regression performance
gates are unevaluated, so the predeclared acceptance conditions are unmet.

The failure is preserved without retry, tuning, added samples or another
optimization. Historical boundary/extent gates and tooling tests pass independently.
Terminal resume rejection is verified with fatal launch sentinels; completed-run
resume cannot be verified without a completed run. See the
[runtime evidence](evidence/candidate-scan-runtime/README.md) and
[per-cell report](evidence/candidate-scan-runtime/report.md).

The verified scratch bound remains 524,320 bytes excluding header/allocator cost;
this run’s correctness-process RSS does not establish timing memory overhead.
Preprocessing/allocation runtime costs and measured speedup remain unknown.
Any future study needs a separately approved response to the driver cache assumption
and a new predeclared schedule. Shipping integration, ownership changes, further
optimization, merge and publication remain excluded.

The implementation/proof decisions below remain historical correctness and work
count evidence. This runtime decision supersedes their requests for a future study.

## Retained implementation decision

Accept the clean [candidate patch](demonstrator/candidate-scan-implementation/candidate.patch)
for review in the isolated Skia experiment. The separate
[diagnostic overlay](demonstrator/candidate-scan-implementation/diagnostic.patch)
is excluded from retained sources. The final 96-process / 384-operation schedule
has no correctness counterexamples against archived traces or its exhaustive
oracle. Zero padding has zero cutoff guard visits, preprocessing, trimming and
scratch allocation. Ownership sources, lifecycle, visual slices, bidi and shaping
remain unchanged. Unsupported arithmetic uses the original exhaustive scanner;
partial endpoint choices are cleared before fallback.

Initial padded long-span expensive evaluations fall from 3,842,986 to 16,852;
spans-32 from 3,346,678 to 16,468. These reductions retain full cheap enumeration
and trimming. Maximum scratch payload remains 524,320 bytes per paragraph,
excluding vector header and allocator overhead. Across retained diagnostic
operations there are 60 padded guard-block visits, 45,220,272 endpoint visits,
15,853,892 trimming subtractions and 45,213,218 suffix comparisons. Each guard-block
visit includes platform checks, a pass over span padding and the overflow envelope.
No timing or speedup inference follows from these work counts.

The [retained evidence](evidence/candidate-scan-implementation/README.md) records
focused/numeric/fallback, historical and harness checks, clean-source/symbol audit,
patch application, completed-run resume, preparation corrections and archive hashes.
Next decision: separately design and approve an uninstrumented runtime study with
identical ownership implementations. Shipping integration, ownership changes,
merge and publication remain excluded.

The original proof results and implementation proposal below are historical
context; the retained decision above supersedes their pending-approval wording.

## Result and proof

Recommend the guarded suffix lower bound for a separate retained implementation
proposal and subsequent uninstrumented runtime study. The final 72 diagnostic
processes / 288 operation records have no endpoint or geometry counterexamples.
Shadow validates all 44,946,905 hypothetical skips against exhaustive evaluation.
The exact proof, floating-point assumptions, fallback and counter definitions are
in the [experiment](demonstrator/candidate-scan-experiment/README.md). Its
[evidence](evidence/candidate-scan-experiment/README.md) includes raw traces,
source snapshots, guard-audit history, environment and verified hashes.

For finite exhaustive trimmed widths t[i], the suffix minimum m[i] is a lower
bound on every remaining t[j]. With nonnegative padding and the guarded arithmetic,
fl(t[j] + p[j]) ≥ t[j]. Strict m[i] > available excludes every remaining fit.
Equality cannot stop scanning. All eligible endpoints are enumerated before this
choice; legal preference, fallback and first indivisible/emergency choice are
therefore preserved. This remains valid when padding cost decreases after overflow.

## Measured work and memory

Initial padded layout on the pinned host:

| Fixture | Exhaustive padding evaluations | Prototype evaluations | Cheap endpoint visits | Trimming subtractions | Suffix comparisons | Reserved scratch |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| long-span | 3,842,986 | 16,852 | 3,842,986 | 0 | 3,842,517 | 524,320 bytes |
| spans-32 | 3,346,678 | 16,468 | 3,346,678 | 1,673,339 | 3,346,271 | 524,320 bytes |

Across all final prototype operations, 45,220,272 endpoint visits and 15,853,892
trimming subtractions prepare 45,213,218 suffix comparisons. The scan performs
273,367 expensive evaluations and skips 44,946,905. Shadow-validation work is
excluded from these proposed-path counts. Zero-padding variants perform zero
candidate preprocessing, trimming or scratch allocation, while retaining the
existing ownership preparation behavior. The diagnostic performs environment and
span-padding guards outside the scanner; do not carry that placement into a
retained implementation's zero-padding path.

The largest allocation is 16,385 entries × 32 bytes, including sentinel capacity;
maximum used size is 16,384. This excludes vector header/allocator overhead.
A single vector is local to wrapping, reused between lines and destroyed on
return. It neither persists across reflow nor grows with candidate ranges over
successive layouts. Whole-process sampled peak RSS is 51,335,168 bytes and does
not measure this allocation in isolation. Enumeration and trimming retain their
worst-case costs; there is no linear layout complexity or runtime-speedup claim.

## Separately approvable implementation scope

A future approval should cover all of these requirements and tasks together:

1. Retain the suffix mechanism only in the padded candidate scanner, conditional
   on nonzero applicable padding and validated arithmetic. Keep ownership, visual
   slices, bidi, Unicode boundary selection, shaping and zero-padding paths intact.
   Move guards inside that path so zero-padding adds no scan preparation. Existing
   zero-padding metadata regressions constrain every retained implementation.
2. Keep exact ordered scalar accumulation and trimming. Reserve one layout-local
   paragraph-cluster-bounded vector, determine first indivisible endpoint during
   enumeration, then compute suffix minima. Apply strict `>` only. Keep original
   exhaustive code as the fallback for nonfinite/negative padding, nonfinite inputs
   or intermediates, unsupported scalar/evaluation representation, fast math,
   nonnearest C/MXCSR rounding or FTZ/DAZ. Guard slice sums and final additions
   with the same conservative envelope. Add no persistent cache or public API.
3. Carry the focused, exhaustive, numeric, fallback, shaping/ownership and historical
   gates into that isolated change. Preserve current tolerances and exact geometry
   comparisons. Treat overflow/rounding assumptions as executable requirements.
4. After correctness, design and seek approval for an uninstrumented paired runtime
   study. Use identical ownership implementations in both arms, with only candidate
   scanning differing. Include padded long/spans-32 and zero-padding metadata cases,
   width/repeat/metadata operations, realistic small inputs, preprocessing/allocation
   costs and memory. Establish schedule, sample count and acceptance criteria before
   measuring; exclude counters and shadow validation from runtime binaries. Do not
   infer runtime acceptance from this diagnostic's durations or evaluation ratios.

No alternate optimization, production retention, runtime matrix, native/Qt
integration, merge, publication or performance acceptance is part of this iteration.
Existing backend, Vulkan shutdown and presentation qualification gaps remain open.

## Runtime continuation outcome — 2026-11-07

The approved cache-aware harness continuation completed all 88 correctness and
1,760 timing processes. Primary padded long/spans-32 operations show substantial
measured benefits, but eight nonprimary cells fail the predeclared overhead-budget
confidence-bound gate. **Retention is not established.** These intervals include
zero: they leave the regression budget unresolved rather than establish a nonzero
slowdown. No retries, additional samples or tuning were performed. See the
[continuation evidence](evidence/candidate-scan-continuation/README.md) and full
per-cell report. Production integration remains unauthorized and unperformed.


## Baseline/self calibration outcome — 2026-11-08

The approved 82-process calibration is unresolved: the prose reflow interval
[−6,296.5, 2,560.0] ns/paragraph exceeds its symmetric historical reference
envelope, although dense metadata fits. Neither target is timer-limited.
**Retention remains not established.** A separately approved measurement-design
revision is recommended before path isolation or further qualification. No new
samples, candidate patch, historical timing correction or integration are approved.
See the [calibration verification](verification.md#candidate-scan-calibration--2026-11-08).

## Calibration revision decision (2026-11-08)

The approved MR1–MR4 fixed experiment completed 322 processes: two correctness
and 320 timing, forty balanced four-process groups per fixture. Calibration remains
**unresolved; retention remains not established**. Prose reflow median overhead is
−3,493.5 ns/paragraph with 95% interval [−7,180, 1,856.75], outside its exact
archived ±4,819.525000000001 envelope. Dense metadata median is 5,137.25 with
interval [−21,943.75, 21,681.25], within ±101,542.07500000001. Neither target is
timer-limited. The fixed run is closed; reconsider the measurement procedure before
another separately approved experiment. Do not extend this run or proceed to path
isolation on this result. Historical gates and results remain unchanged; this
estimator is not pooled with or subtracted from historical paired estimates.

See the [revision tool](demonstrator/candidate-scan-calibration-revision/README.md)
for reproducible analysis and the verification record for actual checks.

[Decision report and verified evidence](evidence/candidate-scan-calibration-revision/README.md).

## Accepted next measurement direction (2026-11-08)

The owner accepted characterization before further calibration, with the correction
that all experiments use this machine and no dedicated benchmark host is available.
The [updated procedure](measurement-procedure.md) supports ordinary shared workstation
use, records available machine/execution telemetry, and characterizes genuine
reflow batches while preserving separate fresh-process observations. Balanced
four-process groups and whole-group analysis remain the intended statistical unit.

Next prepare the bounded characterization design using existing evidence. Freeze
pilot parameters and obtain implementation-plan approval before new measurements;
a later calibration uses a separately frozen protocol. The completed revision
remains unresolved, and retention remains not established.

## Offline characterization and draft pilot (2026-11-08)

Existing-evidence characterization is complete; **retention remains not established**.
All eight revision cells reproduce exactly. Unequal process spacing is observed:
prose group spans range 172.091–228.156 ms and B−A process midpoint imbalance ranges
−12.382–19.296 ms. Prose reflow Spearman correlations are 0.051220 against elapsed
time and −0.013696 against midpoint imbalance. First/last twenty medians are
−5,075.5/−1,915.5 ns per paragraph. These are descriptive observations, not an
explanation of historical variability or evidence for a particular machine cause.
Prose metadata nulls and both timer-limited repeat cells remain intact.

Review the [concrete draft pilot](batching-pilot-proposal.md): genuine prose reflow
at sizes 1/16/64/256 with operation telemetry off/on, plus the unchanged original
reference; 297 processes, seed 73080, ordinary shared use of this workstation.
Select the smallest enabled size meeting the predeclared duration, MAD and resource
criteria, or select none. Instrumentation effects stay separate and A/B envelope
success does not select a size. No harness or execution is approved by this update.
A selected size would be provisional prepared-state throughput; later calibration
requires a separate protocol, dataset and approval. Historical gates are unchanged.

See the [offline tool](demonstrator/candidate-scan-characterization/README.md) and
[verification record](verification.md#offline-workstation-characterization--2026-11-08)
for the complete matrix and actual checks.

## Fixed batching pilot outcome — 2026-11-08

The separately approved fixed pilot completed all 297 processes with no retry,
replacement or additional sample. All nine correctness processes passed. **No size
was selected**: enabled sizes 1/16/64 were below the inclusive 10 ms median target
(size 1 also exceeded the 10% MAD ratio); size 256 exceeded the strict 1 GiB sampled
RSS selection ceiling. Size 64's median was 9.521602 ms; size 256's maximum sampled
RSS was 1,127,469,056 bytes. These thresholds were applied without adjustment.

See [actual verification](verification.md) and the
[pilot tool](demonstrator/candidate-scan-batching-pilot/README.md). Retention remains
**not established**. Stop here: do not infer a qualifying size between tested sizes,
qualify historical results, reuse this dataset for calibration or run further
sampling without separately approved requirements, design and tasks.

## Fixed batching refinement — 2026-11-08

The separately approved [231-process refinement](batching-refinement-protocol.md)
completed all seven correctness and 224 timing processes validly. Select **96**,
the smallest telemetry-enabled size tested, for **provisional prepared-state
throughput among the tested sizes**. Its raw batch median is 14,170,985.5 ns,
unscaled MAD 174,587.5 ns (MAD/median 0.0123201), maximum parent duration
907,446,534 ns and maximum sampled RSS 440,905,728 bytes, including correctness.
All configurations completed regardless of provisional performance. No smoke,
warm-up, retry, replacement or adaptive processes ran. The historical 297-process
pilot and its no-selection result remain unchanged.

Retention remains **not established**. This neither establishes a globally minimal
batch size nor authorizes later calibration or candidate comparisons. A later
calibration requires separately approved protocol, budget and dataset. Evidence:
[decision](evidence/candidate-scan-batching-refinement/selection.json),
[report](evidence/candidate-scan-batching-refinement/report.md), and
[archive manifest](evidence/candidate-scan-batching-refinement/archives.json).

## Prepared-state calibration at 96 — 2026-11-08

The separately approved [483-process protocol](prepared-calibration-protocol.md)
completed with **resolved prepared-state control**. Both required diagnostic
intervals lie inclusively inside their exact historical tolerances and neither
required target is timer-limited:

| Target | Median D ns/paragraph | 95% whole-group interval ns/paragraph | Exact diagnostic tolerance ±ns/paragraph |
| --- | ---: | --- | ---: |
| Prepared prose reflow, 96, telemetry enabled | -326.82552083333337 | [-756.8932291666666, 673.1276041666667] | 4819.525000000001 |
| Original dense metadata | 15477.5 | [3664.0, 25446.0] | 101542.07500000001 |

Original single-instance prose reflow is descriptive: median -4908.5 and interval
[-26266.75, 6452.0] ns/paragraph, outside its diagnostic envelope. This cannot veto
or establish prepared-state resolution. The historical prose tolerance serves as
a diagnostic target for a different measurement mode, not proof of equivalence.
Reference repeat cells are timer-limited; prose metadata remains null. All nine
operation cells and per-group ratio suppression remain explicit in the evidence.

Recommend only **designing a separately approved prepared-state path-isolation
experiment with contemporaneous controls**. No experiment or further measurement
is authorized. Retention remains **not established**. Size 96 remains fixed; pilot
selection thresholds are not calibration gates. No pooling, calibration subtraction,
changed historical gates or explanation of historical variability is inferred.
The new tooling and evidence live at `demonstrator/candidate-scan-prepared-calibration`
and `evidence/candidate-scan-prepared-calibration`; historical evidence is unchanged.

Delivery evidence: [report](evidence/candidate-scan-prepared-calibration/report.md),
[decision](evidence/candidate-scan-prepared-calibration/decision.json) and
[verified archive manifest](evidence/candidate-scan-prepared-calibration/archives.json).
All 1001 archive member hashes passed read-back; completed-run resume launched
nothing and reproduced all three analysis deliverables byte-for-byte. Stop after
this delivery; retention remains **not established**.

## Next experiment design — draft, 2026-11-08

The owner approved drafting the next design and committing the calibration;
`f7879b3` records the calibration. The [prepared-state path-isolation proposal](prepared-path-isolation-proposal.md)
now specifies a private padded-only wrapper, baseline/unchanged/isolated comparisons,
contemporaneous self controls, a fixed proposed budget, analysis and stop rules.
This is a reviewable design, not approved implementation or execution. Retention
remains **not established**. No follow-up builds or measurements have run.

## Prepared-state path isolation — 2026-11-09

The separately approved 2,001-process experiment completed with **mixed diagnostic
support**. All six contemporaneous controls resolved. Dense metadata's I−C interval
is [-63191.75, -24420.0] ns/paragraph and its I−B interval is [-23239.0, -4792.75],
meeting both diagnostic criteria. Prepared prose's I−C interval is
[-700.3125, 921.578125], which includes zero; its residual I−B interval fits the
diagnostic envelope but improvement is not established. This does not prove no
effect. Historical measurements were neither pooled nor subtracted.

**Stop this iteration. Retention remains not established.** The result authorizes
no further samples, production integration or publication. See the
[report](evidence/candidate-scan-prepared-isolation/report.md),
[decision](evidence/candidate-scan-prepared-isolation/decision.json) and
[actual verification](verification.md). Earlier recommendations and approvals
remain historical records.

Delivery is complete: zero-launch completed recovery and byte-identical reanalysis
passed; 15 focused tests, repository Light checks (87 tests), documentation and
whitespace checks passed. All 4,257 members in the
[ten evidence archives](evidence/candidate-scan-prepared-isolation/archives.json)
passed read-back. The [full analysis](evidence/candidate-scan-prepared-isolation/analysis.json.gz)
is available compressed. Frozen source bytes are authoritative for strict replay;
the [delivery cleanup record](evidence/candidate-scan-prepared-isolation/source-cleanup.json)
identifies a subsequent whitespace-only offline-test edit. No executable or
measurement changed.
