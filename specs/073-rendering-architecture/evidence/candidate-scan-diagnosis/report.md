# Candidate-scan overhead diagnosis

**Retention remains not established.** The unchanged report reproduces exactly: 162 passing cells, eight overhead-budget failures, six null metadata cells, 42 timer-limited applicable cells and all six primary gates passing. Every failed overhead interval includes zero; failure to establish the budget is not demonstration of a nonzero slowdown.

Integrity: all 4,343 archive members, 2,605 identities, supplemental identities, 1,848 schedule records and raw/fixture hashes verified. No benchmark was launched, rebuilt or instrumented. The machine-readable cells.json retains 176 cells × 20 pairs, including raw timings, paragraph counts, nulls, order, timer records, duration and both RSS measures. RSS is whole-process and cannot attribute scratch costs.

All values below are µs/paragraph; signs refer to retained minus baseline. These eight cells each have one applicable paragraph. MAD is unscaled median absolute deviation.

| Cell | Median / MAD | Range | + / − / 0 | Original 95% interval | Limit | Upper excess |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| prose-480-4096 / zero / reflow | 2.635 / 4.907 | -13.630…16.144 | 12 / 8 / 0 | [-2.179, 4.945] | 4.820 | 0.125 |
| prose-480-4096 / padded / reflow | 2.652 / 3.670 | -94.017…138.362 | 14 / 6 / 0 | [-0.219, 5.890] | 4.914 | 0.976 |
| dense-240-4096 / zero / metadata | -19.914 / 84.871 | -622.268…224.123 | 8 / 12 / 0 | [-69.891, 112.100] | 101.542 | 10.557 |
| long-480-16384 / zero / initial | -2.422 / 252.042 | -700.100…531.324 | 10 / 10 / 0 | [-142.744, 223.194] | 221.091 | 2.103 |
| long-480-16384 / zero / reflow | 7.752 / 29.076 | -49.924…107.767 | 13 / 7 / 0 | [-6.326, 29.686] | 20.465 | 9.221 |
| long-480-16384 / zero / metadata | 26.995 / 203.230 | -543.204…1690.479 | 11 / 9 / 0 | [-145.730, 176.127] | 169.049 | 7.079 |
| spans-480-1 / zero / reflow | 8.361 / 22.012 | -66.407…195.541 | 12 / 8 / 0 | [-10.156, 20.267] | 19.757 | 0.510 |
| spans-480-1 / zero / metadata | 81.814 / 176.681 | -327.725…644.219 | 12 / 8 / 0 | [-98.513, 201.596] | 164.677 | 36.919 |

## Source and binary findings

Both verified source copies have identical ParagraphImpl.cpp/.h. The driver replaces manifest padding [6,3] with [0,0] for zero variants; boundary layout.h constructs metadata only for code spans. Prose has no metadata in either variant; dense has 178 code spans, long and spans-1 each have one. experimentalPadded() scans those spans for horizontal OR vertical padding: it returns false for all eight failing cells. Thus padded prose does not execute the padded candidate scan.

Initial layout shapes (or restores metadata-free cached shaping), then wraps. Width +17 reflow returns line-broken paragraphs to shaped state and wraps without reshaping. Metadata changes the first span identity, clears ownership/Unicode state and sets kUnknown through experimentalSetSpans(), then shapes and wraps again. Prose metadata operations are null. The unchanged-width repeat can reuse layout, so its passing gate is useful context but does not isolate the wrapper cost.

The retained TextWrapper.cpp adds a pre-loop experimentalPadded() check and a layout-local Candidate vector; guards, reserve, preprocessing and candidate scans remain conditional on positive padding. In binary/retained-00.asm the additional call at 0x47a1cb and false branch at 0x47a1d2 survive. False padding jumps to 0x47a330, initializing scratch/guard stack state before joining the loop. Cleanup tests the scratch pointer at 0x47b7bb and bypasses delete when null. This is no heap scratch allocation on the zero/metadata-free path, but it is not an instruction-identical path. The frame grows from 0x228 to 0x258 bytes (552 to 600); main wrapper symbol grows from 0x17ba to 0x1f63 bytes (6,074 to 8,035), cold part from 7 to 31 bytes. Layout, metadata setter and experimentalPadded retain equal symbol sizes (4,750 / 586 / 67 bytes). Commands, binutils versions, symbol sizes and address-bounded disassembly are retained under binary/. Source copies and fixtures are retained under sources/ and in the unchanged input archives.

These are compiled-code facts. Added calls, stack traffic and changed control flow make added cost plausible on all failed paths; they do not quantify cost or explain any particular observed excursion. Code placement, scheduling, frequency and thermal effects are unproven hypotheses. Recorded affinity allows CPUs 0–15; affinity and initial host load do not establish actual CPU placement or conditions during a sample.

## Per-failure interpretation

**prose-480-4096 / zero / reflow:** The early/late overhead median changes sign; the gate misses by only 0.125 µs. Baseline-first / retained-first overhead medians: 0.450 / 4.157; first-ten / last-ten: 4.591 / -2.179. The false-padding wrapper findings above apply; metadata cells additionally include unchanged invalidation and reshaping. Attribution remains unresolved. Evidence: the matching key in cells.json, all twenty rows in failed-pairs.md, sources/ and binary/{baseline,retained}-00.asm.

**prose-480-4096 / padded / reflow:** Both halves have positive medians, with large excursions of both signs; the nominal padded label does not enable padding without spans. Baseline-first / retained-first overhead medians: 2.652 / 2.795; first-ten / last-ten: 2.995 / 2.106. The false-padding wrapper findings above apply; metadata cells additionally include unchanged invalidation and reshaping. Attribution remains unresolved. Evidence: the matching key in cells.json, all twenty rows in failed-pairs.md, sources/ and binary/{baseline,retained}-00.asm.

**dense-240-4096 / zero / metadata:** The full median is negative despite the failed upper bound; early/late and order medians change sign. The additional padding predicate scans 178 zero-padding spans. Baseline-first / retained-first overhead medians: 40.355 / -36.366; first-ten / last-ten: -44.030 / 62.428. The false-padding wrapper findings above apply; metadata cells additionally include unchanged invalidation and reshaping. Attribution remains unresolved. Evidence: the matching key in cells.json, all twenty rows in failed-pairs.md, sources/ and binary/{baseline,retained}-00.asm.

**long-480-16384 / zero / initial:** Ten differences have each sign and the median is slightly negative; the gate misses by 2.103 µs. Baseline-first / retained-first overhead medians: -2.422 / 13.332; first-ten / last-ten: 42.327 / -69.377. The false-padding wrapper findings above apply; metadata cells additionally include unchanged invalidation and reshaping. Attribution remains unresolved. Evidence: the matching key in cells.json, all twenty rows in failed-pairs.md, sources/ and binary/{baseline,retained}-00.asm.

**long-480-16384 / zero / reflow:** Both halves and both order medians are positive, but seven differences are negative and the interval includes zero. Baseline-first / retained-first overhead medians: 16.044 / 3.821; first-ten / last-ten: 12.191 / 4.744. The false-padding wrapper findings above apply; metadata cells additionally include unchanged invalidation and reshaping. Attribution remains unresolved. Evidence: the matching key in cells.json, all twenty rows in failed-pairs.md, sources/ and binary/{baseline,retained}-00.asm.

**long-480-16384 / zero / metadata:** The maximum is +1,690.479 µs, with negative observations too; early/late medians change sign. No observation is removed. Baseline-first / retained-first overhead medians: 21.904 / 26.995; first-ten / last-ten: 26.995 / -12.332. The false-padding wrapper findings above apply; metadata cells additionally include unchanged invalidation and reshaping. Attribution remains unresolved. Evidence: the matching key in cells.json, all twenty rows in failed-pairs.md, sources/ and binary/{baseline,retained}-00.asm.

**spans-480-1 / zero / reflow:** Early/late and order medians change sign; the gate misses by 0.510 µs. Baseline-first / retained-first overhead medians: 12.223 / -2.460; first-ten / last-ten: 20.267 / -10.156. The false-padding wrapper findings above apply; metadata cells additionally include unchanged invalidation and reshaping. Attribution remains unresolved. Evidence: the matching key in cells.json, all twenty rows in failed-pairs.md, sources/ and binary/{baseline,retained}-00.asm.

**spans-480-1 / zero / metadata:** Early/late medians change sign; this cell has the largest upper-bound excess (36.919 µs). Baseline-first / retained-first overhead medians: 33.541 / 120.727; first-ten / last-ten: 125.449 / -31.517. The false-padding wrapper findings above apply; metadata cells additionally include unchanged invalidation and reshaping. Attribution remains unresolved. Evidence: the matching key in cells.json, all twenty rows in failed-pairs.md, sources/ and binary/{baseline,retained}-00.asm.

## Complete-matrix context and next experiment

The same fixtures contain passing operations (listed individually in context.md). The full matrix is retained without pooling, trimming, new labels or altered gates. All applicable small-fixture cells pass; the primary padded long and spans-32 operations pass, while these failures lie on false-padding paths. This association does not establish a mechanism.

Order and drift summaries are exploratory: alternating order is inseparable from sample parity, and early/late groups are chronological, not randomized interventions. Different signs across cells and halves prevent a single uniform slowdown explanation.

**Recommendation: a separately scoped path-isolation experiment.** Plausible added work is identified on affected paths. The smallest useful treatment is to move candidate scratch/guard setup into a padded-only helper, preserving the exhaustive false-padding wrapper path, then compare against the existing baseline in a new approved study restricted to false-padding prose reflow and zero-padding dense metadata (empty versus many-span predicate). Include interleaved baseline-versus-baseline calibration as a control before attributing residual differences or considering a full qualification. Predeclare pairs, gates and stop rules in that separate plan. The current evidence is mixed as to runtime causation, even though added compiled work is identified. No follow-up, extra samples or qualification reinterpretation is executed here.
