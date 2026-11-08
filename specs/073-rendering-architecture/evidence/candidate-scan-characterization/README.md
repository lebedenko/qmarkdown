# Offline candidate-scan characterization evidence

Retention remains **not established**. No new benchmark process or build was run.
The closed calibration revision remains unresolved; historical gates are unchanged.

The complete diagnostic matrix retains eight cells, 320 group-operation rows and
1,280 process-operation rows. Parent process timestamps are not operation timestamps.
All observations remain present, including prose metadata nulls and timer-limited
repeat timings. Rank correlations are descriptive, without significance tests or
causal attribution. No historical variability is claimed explained.

The [pilot proposal](../../batching-pilot-proposal.md) is a draft requiring separate
implementation and execution approval. It specifies nine configurations and 297
processes on this workstation under ordinary shared use. No harness exists from
this work and no pilot command was executed.

Reproduction uses the [offline tool](../../demonstrator/candidate-scan-characterization/README.md).
Original pinned local inputs must remain available; missing evidence stops analysis.
Archive input identities reference the preserved historical archives rather than
duplicating their contents. The analysis archive includes its own sources, all
input hashes, complete derived observations, exact reproduced revision results,
report, draft protocol/schedule and check logs. See verification.md in the feature
for actual checks and the archive read-back record here for delivery integrity.

## Delivered artifacts and checks

- [Concise diagnostic report](report.md).
- [Complete matrix JSON](matrix.json): all observations and eight cell summaries.
- CSV: [process timings](processes.csv), [group diagnostics](groups.csv),
  [cell summaries](cells.csv). Nulls are literal `null`; nested summaries use JSON.
- [Draft 297-command schedule](pilot-schedule.json): data only, never executed.
- [Analysis archive](073-candidate-characterization.tar.gz) and [member manifest](archives.json).
- [Archive read-back log](archive-verification.log).

Twelve focused tests and repository lightweight checks (87 Python tests plus
syntax/documentation checks) passed. Independent full reanalysis produced nine
byte-identical deliverables under fatal launch sentinels. All eight archived
revision estimates and the summary/observations/report bytes reproduce exactly.
All 49 preexisting historical source/evidence files match the pre-edit snapshot;
pinned source/binary/fixture identities also remain unchanged. No samples were
excluded and no historical protocol, gate or evidence was changed.

Final archive: 29 member hashes verified, including read-back of the delivered
archive. SHA-256:
`0812332010b6306d49897af3e67d2cacc8a623d65e9798da558ff1fb1b9794bb`.
The self-verification log is kept alongside the archive to avoid a self-referential
hash; completed analysis/check logs are archived within it. Historical inputs are
referenced by their immutable identities and existing archives, not regenerated.
