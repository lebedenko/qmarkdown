# Workstation revision characterization

Retention remains **not established**. Offline analysis only; no new measurements.

All eight archived revision estimates, summaries and report bytes reproduced exactly before diagnostics. The prose reflow target remains unresolved. All 320 timing processes and 40 groups per fixture are retained.

| Fixture / operation | Timer limited | ρ timing / position | ρ difference / elapsed | ρ difference / imbalance | First 20 median ns/paragraph | Last 20 median ns/paragraph |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| prose-480-4096 / initial | False | 0.112932 | -0.0731707 | 0.0378987 | -101968 | -57770.8 |
| prose-480-4096 / reflow | False | 0.0684929 | 0.0512195 | -0.0136961 | -5075.5 | -1915.5 |
| prose-480-4096 / repeat | True | 0.0452519 | -0.236878 | 0.0152915 | 0.75 | -5.25 |
| prose-480-4096 / metadata | False | null | null | null | null | null |
| dense-240-4096 / initial | False | 0.0374165 | -0.250281 | 0.154972 | 8979.25 | -21189.5 |
| dense-240-4096 / reflow | False | 0.0500234 | -0.106567 | 0.285553 | 149.75 | -2804.5 |
| dense-240-4096 / repeat | True | 0.0801906 | 0.0708488 | 0.0729132 | 1.25 | -1.75 |
| dense-240-4096 / metadata | False | -0.0540124 | -0.221201 | 0.0628518 | 18258.5 | -17293.2 |

prose-480-4096: group span median 188.660 ms, range 172.091–228.156 ms; B−A process midpoint imbalance median -0.131 ms, range -12.382–19.296 ms.

dense-240-4096: group span median 955.436 ms, range 933.680–994.027 ms; B−A process midpoint imbalance median -0.117 ms, range -7.037–11.606 ms.

Observed patterns: operation timings and group differences vary with chronological position as quantified above; midpoint imbalances are nonzero and unequal spacing occurs. ABBA/BAAB symmetry in ordinal position therefore does not establish equal cancellation of elapsed-time drift. The complete JSON/CSV matrix includes every process timing, process duration, group span/gap, midpoint imbalance, order and half summary. Prose metadata remains null; both repeat cells remain timer-limited. Their raw absolute timings and descriptive correlations are retained without restoring suppressed ratios.

These are parent-observed process timestamps, not operation timestamps. Group elapsed time is the mean of four process midpoints relative to the first timing-process start; imbalance is mean B minus mean A midpoint. Operation elapsed time comes from the existing raw timer, with no inferred operation start time. Process duration includes supervision and setup. CSV null is the literal `null`; nested cell summaries use JSON fields.

Spearman correlations are descriptive rank associations (average ranks for ties; null for constant or inapplicable input), not significance tests or causal estimates. No samples were excluded and no runs pooled. No new intervals or inferential claims are added. The reproduced historical bootstrap remains historical evidence.

Hypotheses requiring future telemetry: CPU placement/migration, scheduling, frequency, thermal conditions or shared-cache effects might contribute to variation. This archive cannot distinguish them or explain historical variability. Future boundary telemetry will also have limited sampling scope.

Recommendation: review the separate draft batching pilot protocol. It proposes 297 processes on this same workstation under ordinary shared use, but authorizes no harness or execution. A selected size would be provisional prepared-state throughput, not calibration or historical qualification.
