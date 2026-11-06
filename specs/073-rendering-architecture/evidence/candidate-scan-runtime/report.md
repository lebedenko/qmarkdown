# Candidate-scan runtime study

Retention is not established.

Twenty adjacent fresh-process pairs per cell; descriptive 95% percentile whole-pair intervals, 10,000 resamples, seed 73075. No pooled speedup. Difference is retained minus baseline. Ratios are suppressed below 100 times recorded timer resolution. Absolute regression gates remain active. RSS is whole-process; preprocessing and allocation are included without separate attribution.

| Fixture | Padding | Operation | Pairs | Baseline ns [95%] | Retained ns [95%] | Difference ns [95%] | Improvement [95%] | Overhead ns/paragraph [95%] | Gate |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| prose-480-4096 | zero | initial | 0 | null | null | null | null | null | incomplete |
| prose-480-4096 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| prose-480-4096 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| prose-480-4096 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| prose-480-4096 | padded | initial | 0 | null | null | null | null | null | incomplete |
| prose-480-4096 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| prose-480-4096 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| prose-480-4096 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| dense-480-1024 | zero | initial | 0 | null | null | null | null | null | incomplete |
| dense-480-1024 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| dense-480-1024 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| dense-480-1024 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| dense-480-1024 | padded | initial | 0 | null | null | null | null | null | incomplete |
| dense-480-1024 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| dense-480-1024 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| dense-480-1024 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| dense-480-4096 | zero | initial | 0 | null | null | null | null | null | incomplete |
| dense-480-4096 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| dense-480-4096 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| dense-480-4096 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| dense-480-4096 | padded | initial | 0 | null | null | null | null | null | incomplete |
| dense-480-4096 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| dense-480-4096 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| dense-480-4096 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| dense-240-4096 | zero | initial | 0 | null | null | null | null | null | incomplete |
| dense-240-4096 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| dense-240-4096 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| dense-240-4096 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| dense-240-4096 | padded | initial | 0 | null | null | null | null | null | incomplete |
| dense-240-4096 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| dense-240-4096 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| dense-240-4096 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| dense-960-4096 | zero | initial | 0 | null | null | null | null | null | incomplete |
| dense-960-4096 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| dense-960-4096 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| dense-960-4096 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| dense-960-4096 | padded | initial | 0 | null | null | null | null | null | incomplete |
| dense-960-4096 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| dense-960-4096 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| dense-960-4096 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| mixed-480-4096 | zero | initial | 0 | null | null | null | null | null | incomplete |
| mixed-480-4096 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| mixed-480-4096 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| mixed-480-4096 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| mixed-480-4096 | padded | initial | 0 | null | null | null | null | null | incomplete |
| mixed-480-4096 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| mixed-480-4096 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| mixed-480-4096 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| objects-480-4096 | zero | initial | 0 | null | null | null | null | null | incomplete |
| objects-480-4096 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| objects-480-4096 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| objects-480-4096 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| objects-480-4096 | padded | initial | 0 | null | null | null | null | null | incomplete |
| objects-480-4096 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| objects-480-4096 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| objects-480-4096 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| long-480-16384 | zero | initial | 0 | null | null | null | null | null | incomplete |
| long-480-16384 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| long-480-16384 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| long-480-16384 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| long-480-16384 | padded | initial | 0 | null | null | null | null | null | incomplete |
| long-480-16384 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| long-480-16384 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| long-480-16384 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| spans-480-1 | zero | initial | 0 | null | null | null | null | null | incomplete |
| spans-480-1 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| spans-480-1 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| spans-480-1 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| spans-480-1 | padded | initial | 0 | null | null | null | null | null | incomplete |
| spans-480-1 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| spans-480-1 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| spans-480-1 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| spans-480-8 | zero | initial | 0 | null | null | null | null | null | incomplete |
| spans-480-8 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| spans-480-8 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| spans-480-8 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| spans-480-8 | padded | initial | 0 | null | null | null | null | null | incomplete |
| spans-480-8 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| spans-480-8 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| spans-480-8 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| spans-480-32 | zero | initial | 0 | null | null | null | null | null | incomplete |
| spans-480-32 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| spans-480-32 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| spans-480-32 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| spans-480-32 | padded | initial | 0 | null | null | null | null | null | incomplete |
| spans-480-32 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| spans-480-32 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| spans-480-32 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| document-480-10 | zero | initial | 0 | null | null | null | null | null | incomplete |
| document-480-10 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| document-480-10 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| document-480-10 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| document-480-10 | padded | initial | 0 | null | null | null | null | null | incomplete |
| document-480-10 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| document-480-10 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| document-480-10 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| small-prose-480-64 | zero | initial | 0 | null | null | null | null | null | incomplete |
| small-prose-480-64 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| small-prose-480-64 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| small-prose-480-64 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| small-prose-480-64 | padded | initial | 0 | null | null | null | null | null | incomplete |
| small-prose-480-64 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| small-prose-480-64 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| small-prose-480-64 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| small-prose-480-256 | zero | initial | 0 | null | null | null | null | null | incomplete |
| small-prose-480-256 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| small-prose-480-256 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| small-prose-480-256 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| small-prose-480-256 | padded | initial | 0 | null | null | null | null | null | incomplete |
| small-prose-480-256 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| small-prose-480-256 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| small-prose-480-256 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| small-dense-480-64 | zero | initial | 0 | null | null | null | null | null | incomplete |
| small-dense-480-64 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| small-dense-480-64 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| small-dense-480-64 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| small-dense-480-64 | padded | initial | 0 | null | null | null | null | null | incomplete |
| small-dense-480-64 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| small-dense-480-64 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| small-dense-480-64 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| small-dense-480-256 | zero | initial | 0 | null | null | null | null | null | incomplete |
| small-dense-480-256 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| small-dense-480-256 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| small-dense-480-256 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| small-dense-480-256 | padded | initial | 0 | null | null | null | null | null | incomplete |
| small-dense-480-256 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| small-dense-480-256 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| small-dense-480-256 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-64 | zero | initial | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-64 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-64 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-64 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-64 | padded | initial | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-64 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-64 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-64 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-256 | zero | initial | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-256 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-256 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-256 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-256 | padded | initial | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-256 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-256 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| small-mixed-480-256 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| small-long-480-64 | zero | initial | 0 | null | null | null | null | null | incomplete |
| small-long-480-64 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| small-long-480-64 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| small-long-480-64 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| small-long-480-64 | padded | initial | 0 | null | null | null | null | null | incomplete |
| small-long-480-64 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| small-long-480-64 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| small-long-480-64 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| small-long-480-256 | zero | initial | 0 | null | null | null | null | null | incomplete |
| small-long-480-256 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| small-long-480-256 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| small-long-480-256 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| small-long-480-256 | padded | initial | 0 | null | null | null | null | null | incomplete |
| small-long-480-256 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| small-long-480-256 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| small-long-480-256 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| small-objects-480-64 | zero | initial | 0 | null | null | null | null | null | incomplete |
| small-objects-480-64 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| small-objects-480-64 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| small-objects-480-64 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| small-objects-480-64 | padded | initial | 0 | null | null | null | null | null | incomplete |
| small-objects-480-64 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| small-objects-480-64 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| small-objects-480-64 | padded | metadata | 0 | null | null | null | null | null | incomplete |
| small-objects-480-256 | zero | initial | 0 | null | null | null | null | null | incomplete |
| small-objects-480-256 | zero | reflow | 0 | null | null | null | null | null | incomplete |
| small-objects-480-256 | zero | repeat | 0 | null | null | null | null | null | incomplete |
| small-objects-480-256 | zero | metadata | 0 | null | null | null | null | null | incomplete |
| small-objects-480-256 | padded | initial | 0 | null | null | null | null | null | incomplete |
| small-objects-480-256 | padded | reflow | 0 | null | null | null | null | null | incomplete |
| small-objects-480-256 | padded | repeat | 0 | null | null | null | null | null | incomplete |
| small-objects-480-256 | padded | metadata | 0 | null | null | null | null | null | incomplete |

| Small fixture | Padding | Operation | Baseline ns/paragraph [95%] | Retained ns/paragraph [95%] |
| --- | --- | --- | ---: | ---: |
| small-prose-480-64 | zero | initial | null | null |
| small-prose-480-64 | zero | reflow | null | null |
| small-prose-480-64 | zero | repeat | null | null |
| small-prose-480-64 | zero | metadata | null | null |
| small-prose-480-64 | padded | initial | null | null |
| small-prose-480-64 | padded | reflow | null | null |
| small-prose-480-64 | padded | repeat | null | null |
| small-prose-480-64 | padded | metadata | null | null |
| small-prose-480-256 | zero | initial | null | null |
| small-prose-480-256 | zero | reflow | null | null |
| small-prose-480-256 | zero | repeat | null | null |
| small-prose-480-256 | zero | metadata | null | null |
| small-prose-480-256 | padded | initial | null | null |
| small-prose-480-256 | padded | reflow | null | null |
| small-prose-480-256 | padded | repeat | null | null |
| small-prose-480-256 | padded | metadata | null | null |
| small-dense-480-64 | zero | initial | null | null |
| small-dense-480-64 | zero | reflow | null | null |
| small-dense-480-64 | zero | repeat | null | null |
| small-dense-480-64 | zero | metadata | null | null |
| small-dense-480-64 | padded | initial | null | null |
| small-dense-480-64 | padded | reflow | null | null |
| small-dense-480-64 | padded | repeat | null | null |
| small-dense-480-64 | padded | metadata | null | null |
| small-dense-480-256 | zero | initial | null | null |
| small-dense-480-256 | zero | reflow | null | null |
| small-dense-480-256 | zero | repeat | null | null |
| small-dense-480-256 | zero | metadata | null | null |
| small-dense-480-256 | padded | initial | null | null |
| small-dense-480-256 | padded | reflow | null | null |
| small-dense-480-256 | padded | repeat | null | null |
| small-dense-480-256 | padded | metadata | null | null |
| small-mixed-480-64 | zero | initial | null | null |
| small-mixed-480-64 | zero | reflow | null | null |
| small-mixed-480-64 | zero | repeat | null | null |
| small-mixed-480-64 | zero | metadata | null | null |
| small-mixed-480-64 | padded | initial | null | null |
| small-mixed-480-64 | padded | reflow | null | null |
| small-mixed-480-64 | padded | repeat | null | null |
| small-mixed-480-64 | padded | metadata | null | null |
| small-mixed-480-256 | zero | initial | null | null |
| small-mixed-480-256 | zero | reflow | null | null |
| small-mixed-480-256 | zero | repeat | null | null |
| small-mixed-480-256 | zero | metadata | null | null |
| small-mixed-480-256 | padded | initial | null | null |
| small-mixed-480-256 | padded | reflow | null | null |
| small-mixed-480-256 | padded | repeat | null | null |
| small-mixed-480-256 | padded | metadata | null | null |
| small-long-480-64 | zero | initial | null | null |
| small-long-480-64 | zero | reflow | null | null |
| small-long-480-64 | zero | repeat | null | null |
| small-long-480-64 | zero | metadata | null | null |
| small-long-480-64 | padded | initial | null | null |
| small-long-480-64 | padded | reflow | null | null |
| small-long-480-64 | padded | repeat | null | null |
| small-long-480-64 | padded | metadata | null | null |
| small-long-480-256 | zero | initial | null | null |
| small-long-480-256 | zero | reflow | null | null |
| small-long-480-256 | zero | repeat | null | null |
| small-long-480-256 | zero | metadata | null | null |
| small-long-480-256 | padded | initial | null | null |
| small-long-480-256 | padded | reflow | null | null |
| small-long-480-256 | padded | repeat | null | null |
| small-long-480-256 | padded | metadata | null | null |
| small-objects-480-64 | zero | initial | null | null |
| small-objects-480-64 | zero | reflow | null | null |
| small-objects-480-64 | zero | repeat | null | null |
| small-objects-480-64 | zero | metadata | null | null |
| small-objects-480-64 | padded | initial | null | null |
| small-objects-480-64 | padded | reflow | null | null |
| small-objects-480-64 | padded | repeat | null | null |
| small-objects-480-64 | padded | metadata | null | null |
| small-objects-480-256 | zero | initial | null | null |
| small-objects-480-256 | zero | reflow | null | null |
| small-objects-480-256 | zero | repeat | null | null |
| small-objects-480-256 | zero | metadata | null | null |
| small-objects-480-256 | padded | initial | null | null |
| small-objects-480-256 | padded | reflow | null | null |
| small-objects-480-256 | padded | repeat | null | null |
| small-objects-480-256 | padded | metadata | null | null |

Full per-cell timer periods, empirical resolution, baseline/retained per-paragraph intervals and individual process RSS are retained in summary.json. Small fixtures have 32 paragraphs; metadata uses only applicable paragraphs.

Verified layout-local Candidate vector reserved to paragraph cluster count; no zero-path scratch. RSS is whole-process, not scratch attribution.
