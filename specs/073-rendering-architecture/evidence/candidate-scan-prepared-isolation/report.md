# Prepared-state path-isolation experiment

Outcome: **mixed diagnostic support**. Retention remains **not established**.

Stop this iteration. No further measurements authorized.

| Configuration | Operation | Median D ns/paragraph | 95% interval | Diagnostic tolerance ±ns/paragraph | Envelope fit | Timer limited |
| --- | --- | ---: | --- | ---: | --- | --- |
| prepared/baseline-self | prepared-reflow | -369.27864583333337 | [-627.0260416666667, 417.0364583333333] | 4819.525000000001 | True | False |
| prepared/candidate-self | prepared-reflow | 929.3203125 | [246.47395833333331, 1588.0598958333333] | 4819.525000000001 | True | False |
| prepared/isolated-self | prepared-reflow | -35.747395833333336 | [-1024.828125, 699.9895833333333] | 4819.525000000001 | True | False |
| prepared/candidate-minus-baseline | prepared-reflow | -86.79427083333333 | [-1040.1953125, 826.4817708333333] | 4819.525000000001 | True | False |
| prepared/isolated-minus-baseline | prepared-reflow | 173.75520833333334 | [-743.6328125, 945.1171875] | 4819.525000000001 | True | False |
| prepared/isolated-minus-candidate | prepared-reflow | 317.5078125 | [-700.3125, 921.578125] | 4819.525000000001 | True | False |
| original/baseline-self | initial | -3871.25 | [-36809.75, 11537.75] | None | None | False |
| original/baseline-self | reflow | -686.25 | [-2508.75, 646.75] | None | None | False |
| original/baseline-self | repeat | -3.5 | [-6.0, 7.0] | None | None | True |
| original/baseline-self | metadata | 8014.75 | [-10439.25, 15366.25] | 101542.07500000001 | True | False |
| original/candidate-self | initial | -7414.0 | [-34867.5, 4344.25] | None | None | False |
| original/candidate-self | reflow | 450.25 | [-1485.75, 2014.25] | None | None | False |
| original/candidate-self | repeat | -10.75 | [-15.25, -3.5] | None | None | True |
| original/candidate-self | metadata | -12918.75 | [-22938.25, 15423.5] | 101542.07500000001 | True | False |
| original/isolated-self | initial | 2108.5 | [-12180.0, 15452.5] | None | None | False |
| original/isolated-self | reflow | -357.25 | [-1245.5, 1009.0] | None | None | False |
| original/isolated-self | repeat | -2.0 | [-8.0, 8.0] | None | None | True |
| original/isolated-self | metadata | -45.0 | [-9204.25, 14982.5] | 101542.07500000001 | True | False |
| original/candidate-minus-baseline | initial | 24162.25 | [-9146.5, 47740.0] | None | None | False |
| original/candidate-minus-baseline | reflow | 1087.75 | [-1318.5, 1754.75] | None | None | False |
| original/candidate-minus-baseline | repeat | -6.25 | [-11.5, -3.75] | None | None | True |
| original/candidate-minus-baseline | metadata | 13728.5 | [3489.25, 26119.75] | 101542.07500000001 | True | False |
| original/isolated-minus-baseline | initial | -8223.75 | [-37189.75, 21165.5] | None | None | False |
| original/isolated-minus-baseline | reflow | 612.25 | [-222.25, 1108.5] | None | None | False |
| original/isolated-minus-baseline | repeat | -5.75 | [-10.5, 4.5] | None | None | True |
| original/isolated-minus-baseline | metadata | -11001.5 | [-23239.0, -4792.75] | 101542.07500000001 | True | False |
| original/isolated-minus-candidate | initial | -38778.5 | [-61936.5, -13602.25] | None | None | False |
| original/isolated-minus-candidate | reflow | -1387.0 | [-4102.25, 192.25] | None | None | False |
| original/isolated-minus-candidate | repeat | 2.0 | [-7.25, 10.75] | None | None | True |
| original/isolated-minus-candidate | metadata | -48073.25 | [-63191.75, -24420.0] | 101542.07500000001 | True | False |

D = ((B1+B2) − (A1+A2))/(2×N); N is 96 for prepared prose and recorded applicability for original dense. Forty whole groups; 10,000 bootstrap draws, seed 73085 reset per configuration, percentile indices 249/9749. Intervals are diagnostic and have no multiple-comparison adjustment.

All six contemporaneous controls must resolve before isolation interpretation. Per-mode support additionally requires a strictly negative I−C upper bound, inclusive I−B envelope fit and resolved comparison timers. Zero-inclusive intervals do not establish no effect.

Historical observations are neither pooled nor subtracted. Prose uses a diagnostic tolerance from a different measurement mode; no equivalence or retention decision follows. C−B and dense initial/reflow/repeat are descriptive. Code layout and dispatch also changed; no stack-only causal attribution is possible.

analysis.json retains raw and normalized timings, arm means, differences, medians, unscaled MAD, ranges, signs, order/half summaries, ratio suppression reasons, nulls and separate stage resource maxima. Sampled RSS does not bound unobserved peaks; snapshots do not establish continuous frequency or exclude migration.

{"original": {"diagnosticSupport": true, "residualEnvelopeFit": true, "strictlyNegativeIsolationInterval": true, "timerLimited": false}, "prepared": {"diagnosticSupport": false, "residualEnvelopeFit": true, "strictlyNegativeIsolationInterval": false, "timerLimited": false}}
