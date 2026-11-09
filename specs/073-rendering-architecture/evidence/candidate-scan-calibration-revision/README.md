# Candidate-scan calibration revision evidence

Outcome: **unresolved calibration; retention not established**.
All 322 scheduled processes completed. Prose reflow fails its reference envelope;
dense metadata resolves. Neither target is timer-limited. The fixed run is closed:
reconsider measurement procedure before another separately approved experiment.

- [Decision report](report.md): all eight cells, intervals and interpretation.
- [Machine-readable summary](summary.json): group differences, per-arm timings,
  descriptive statistics, order/half summaries, intervals and decision.
- [Archive manifest](archives.json): archive and all 9,658 member SHA-256 values.
- [Complete archive](073-candidate-calibration-revision.tar.gz): 320 group-operation
  observations in observations.json, all process outputs, sources, authoritative
  schedule, commands, identities, fixtures, environment fingerprints, durable
  attempts/completion journals, parent monotonic timestamps and check logs.
- [Archive verification](archive-verification.log): member read-back and historical
  identity revalidation.
- [Reproduction tool](../../demonstrator/candidate-scan-calibration-revision/README.md).

Archive SHA-256:
`b92337dc58d390564870aa74b08b5b426a046daea3819dc7dd56406a4fe6e983`.
All members were read back and verified; historical identities remained unchanged.
Fifteen focused tests and repository lightweight checks (87 Python tests plus
documentation/configuration checks) passed. Reanalysis was byte-identical for
observations, summary and report; completed resume validated 322 records with fatal
launch sentinels and zero launches. Sources and inputs are pinned to their original
paths; extract into a separate directory for inspection.

The group estimator differs from historical paired medians. No runs are pooled,
gates revised, calibration estimates subtracted or historical variability claimed
explained. Forty groups are a fixed budget, not a power/independence guarantee.
No additional measurement, candidate comparison, integration or publication follows.
