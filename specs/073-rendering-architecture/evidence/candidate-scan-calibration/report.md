# Candidate-scan calibration

Calibration: **unresolved**. Retention remains **not established**.

Propose a separately approved measurement-design revision before path isolation or qualification.

| Fixture | Operation | Median B − A ns/paragraph | 95% interval | Target envelope ±ns/paragraph | Timer limited |
| --- | --- | ---: | --- | ---: | --- |
| prose-480-4096 | initial | -16764.0 | [-74012.0, 44924.0] | — | False |
| prose-480-4096 | reflow | -414.5 | [-6296.5, 2560.0] | 4819.525000000001 | False |
| prose-480-4096 | repeat | 6.0 | [-8.5, 19.5] | — | True |
| prose-480-4096 | metadata | None | None | — | False |
| dense-240-4096 | initial | -3913.0 | [-113841.5, 39244.0] | — | False |
| dense-240-4096 | reflow | -2764.0 | [-4313.0, -41.0] | — | False |
| dense-240-4096 | repeat | -0.5 | [-13.0, 9.5] | — | True |
| dense-240-4096 | metadata | -1289.5 | [-26529.5, 13065.5] | 101542.07500000001 | False |

Twenty adjacent pairs per fixture; 10,000 whole-pair resamples, seed 73077 reset per fixture, shared indices across operations. Percentile indices are int(p × (N − 1)). Envelopes use exact archived numeric limits with inclusive boundaries. This diagnostic does not establish equivalence, change historical gates or justify subtracting control estimates from historical timings.

Each process retains initial, width+17 reflow, unchanged repeat and metadata. Prose metadata is null. observations.json retains all raw operation values, duration and RSS; summary.json includes arm medians, unscaled MAD, range, signs, order and early/late summaries. Ratios are suppressed when either arm median is below 100× the larger clock period/empirical resolution.

82 processes completed; summed supervised duration 10.104 seconds; maximum sampled RSS 34697216 bytes; high-water RSS 74305536 bytes.

Twenty pairs are a fixed diagnostic budget, not a power guarantee. Order and chronology are exploratory. Actual CPU placement, frequency, thermals and scheduling are unmeasured; caches are uncontrolled. No warm-ups, retries, extra samples, affinity changes or builds.
