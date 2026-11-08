# Candidate-scan calibration revision

Calibration: **unresolved**. Retention remains **not established**.

Stop: reconsider the measurement procedure before another separately approved experiment. Do not extend this run.

| Fixture | Operation | Median B − A ns/paragraph | 95% interval | Target envelope ±ns/paragraph | Timer limited |
| --- | --- | ---: | --- | ---: | --- |
| prose-480-4096 | initial | -85508.25 | [-185241.5, 35624.75] | — | False |
| prose-480-4096 | reflow | -3493.5 | [-7180.0, 1856.75] | 4819.525000000001 | False |
| prose-480-4096 | repeat | -1.0 | [-15.75, 10.0] | — | True |
| prose-480-4096 | metadata | None | None | — | False |
| dense-240-4096 | initial | -5362.5 | [-35083.5, 36162.75] | — | False |
| dense-240-4096 | reflow | -335.25 | [-4438.25, 3032.5] | — | False |
| dense-240-4096 | repeat | -1.0 | [-4.0, 2.25] | — | True |
| dense-240-4096 | metadata | 5137.25 | [-21943.75, 21681.25] | 101542.07500000001 | False |

Forty four-process groups per fixture; D = ((B1 + B2) − (A1 + A2)) / (2 × applicableParagraphs). Central estimate: median of forty group differences. 10,000 whole-group resamples, seed 73079 reset per fixture, shared indices across operations. Percentile index int(p × (N − 1)); exact archived inclusive envelopes.

Each arm group timing is its two-process mean. Median relative improvement is (A − B) / A across groups; ratios are suppressed when either median arm timing is below 100× maximum recorded clock period/empirical resolution. All eight cells, raw observations, group differences, unscaled MAD, range, signs, per-arm timings, ABBA/BAAB and first-20/last-20 summaries are retained in observations.json and summary.json. Prose metadata stays null.

322 processes completed; summed supervised duration 39.696 seconds; maximum sampled RSS 34672640 bytes; high-water RSS 80932864 bytes.

This estimator differs from the earlier median paired difference. Runs are not pooled; historical gates are unchanged; no calibration estimate is subtracted and historical variability is not claimed explained. Balanced groups cancel an ideal linear effect at equally spaced execution positions, not necessarily real elapsed-time drift.

Forty groups are a fixed budget, not a power guarantee or proof of statistical independence. CPU placement, frequency, thermals and scheduling remain unmeasured. No warm-ups, cache flushing, retries, optional samples or environment tuning.
