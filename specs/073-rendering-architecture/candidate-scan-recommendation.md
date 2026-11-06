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
