# Fixed batching pilot

Completed 231/231 processes. Selection: 96 (provisional prepared-state throughput).

Retention remains **not established**. These descriptive observations cannot qualify historical results or supply a later calibration dataset.

Raw timing, normalized timing, all group/arm estimates, order/half summaries and matched-round telemetry comparisons are in analysis.json. Correctness and original-reference records remain distinct.

| Configuration | Median batch ns | MAD ns | Maximum parent ns | Maximum sampled RSS bytes |
| --- | ---: | ---: | ---: | ---: |
| original-reference / initial | 4458279.5 | 920248.5 | 71534556 | 33026048 |
| original-reference / reflow | 105469.0 | 11698.0 | 71534556 | 33026048 |
| original-reference / repeat | 158.0 | 41.0 | 71534556 | 33026048 |
| original-reference / metadata | None | None | 71534556 | 33026048 |
| batch-96-disabled | 14185401.5 | 147735.0 | 907336182 | 442675200 |
| batch-96-enabled | 14170985.5 | 174587.5 | 907446534 | 440905728 |
| batch-128-disabled | 18723222.5 | 123648.5 | 1211730269 | 578940928 |
| batch-128-enabled | 18846449.5 | 128234.5 | 1192543975 | 578658304 |
| batch-192-disabled | 28251785.5 | 147886.5 | 1780208157 | 855064576 |
| batch-192-enabled | 28103182.5 | 217293.5 | 1760592970 | 855265280 |

Sampled RSS cannot establish unobserved peaks. Parent snapshots and operation-boundary observations do not establish frequency throughout the loop or rule out migration. Matched configurations are not simultaneous. Instrumentation durations are never subtracted.
