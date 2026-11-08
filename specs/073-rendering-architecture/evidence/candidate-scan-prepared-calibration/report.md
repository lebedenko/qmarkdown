# Prepared-state calibration at size 96

Calibration: **resolved-prepared-state-control**. Retention remains **not established**.

Recommend designing a separately approved prepared-state path-isolation experiment with contemporaneous controls.

| Configuration | Operation | Median D ns/paragraph | 95% interval | Diagnostic tolerance ±ns/paragraph | Envelope fit | Timer limited |
| --- | --- | ---: | --- | ---: | --- | --- |
| original-reference | initial | -114948.75 | [-939673.5, 256053.0] | None | None | False |
| original-reference | reflow | -4908.5 | [-26266.75, 6452.0] | 4819.525000000001 | False | False |
| original-reference | repeat | -15.5 | [-37.25, 18.0] | None | None | True |
| original-reference | metadata | None | None | None | None | False |
| batch-96-enabled | prepared-reflow | -326.82552083333337 | [-756.8932291666666, 673.1276041666667] | 4819.525000000001 | True | False |
| original-dense | initial | 844.5 | [-17291.0, 30889.5] | None | None | False |
| original-dense | reflow | 1044.75 | [-41.75, 2408.0] | None | None | False |
| original-dense | repeat | -5.5 | [-6.5, 1.5] | None | None | True |
| original-dense | metadata | 15477.5 | [3664.0, 25446.0] | 101542.07500000001 | True | False |

Original prose is descriptive and cannot veto or establish resolution. Its historical prose tolerance is a diagnostic target for a different measurement mode, not proof of metric equivalence.

D = ((B1+B2) − (A1+A2))/(2×N). A/B are two-process means. Median of forty groups; 10,000 whole-group resamples, seed 73083 reset per configuration and shared across operations; percentile indices int(p×9999). Inclusive envelopes and the historical target timer rule apply only to the two required targets.

analysis.json retains every raw/normalized timing, arm mean/difference, unscaled MAD/range/signs, ABBA/BAAB and first20/last20 summaries, correctness/timing resources, null reasons and ratio counts. Ratios suppress any group containing a raw timing below 100× its maximum clock period/resolution and are never bootstrapped. Raw telemetry, parent snapshots and journals remain in process files/index.jsonl.

Size 96 is fixed. Pilot thresholds add no calibration gates. No data pooling, calibration subtraction, altered historical gates or explanation of historical variability. Forty groups give no power or independence guarantee. Sampled RSS cannot establish unobserved peaks; snapshots cannot establish continuous frequency or exclude migration. No outcome authorizes another measurement.
