# Fixed batching pilot

Completed 297/297 processes. Selection: None (no size selected).

Retention remains **not established**. These descriptive observations cannot qualify historical results or supply a later calibration dataset.

Raw timing, normalized timing, all group/arm estimates, order/half summaries and matched-round telemetry comparisons are in analysis.json. Correctness and original-reference records remain distinct.

| Configuration | Median batch ns | MAD ns | Maximum parent ns | Maximum sampled RSS bytes |
| --- | ---: | ---: | ---: | ---: |
| original-reference | 7927599.5 | 3449129.5 | 72118472 | 29143040 |
| batch-1-disabled | 160854.5 | 63535.5 | 71372312 | 26918912 |
| batch-1-enabled | 133551.0 | 36438.5 | 71761998 | 28893184 |
| batch-16-disabled | 2257079.0 | 62980.0 | 199886070 | 94576640 |
| batch-16-enabled | 2287205.0 | 65516.5 | 178398411 | 94572544 |
| batch-64-disabled | 9530663.5 | 98526.0 | 625604587 | 303054848 |
| batch-64-enabled | 9521602.0 | 110617.0 | 625396158 | 302641152 |
| batch-256-disabled | 37556576.0 | 227930.0 | 2371587111 | 1127485440 |
| batch-256-enabled | 37507841.0 | 192256.5 | 2413357677 | 1127469056 |

Sampled RSS cannot establish unobserved peaks. Parent snapshots and operation-boundary observations do not establish frequency throughout the loop or rule out migration. Matched configurations are not simultaneous. Instrumentation durations are never subtracted.
