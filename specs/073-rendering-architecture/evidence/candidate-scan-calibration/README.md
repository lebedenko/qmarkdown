# Candidate-scan calibration evidence

> Storage cleanup: generated artifacts linked below were removed from Git history.
> See the [retention policy](../../../../docs/artifact-storage.md); historical results remain, raw-data revalidation requires regenerated inputs.

Outcome: **unresolved calibration; retention not established**.
All 82 scheduled processes completed. Prose reflow exceeds its reference envelope;
dense metadata fits. Neither target is timer-limited. A separately approved
measurement-design revision is recommended before path isolation or qualification.

- [Decision report](report.md): all eight cells and interpretation.
- [Machine-readable summary](summary.json): intervals, descriptive statistics,
  timer limitations, order/early-late summaries, duration and RSS.
- [Archive index](archives.json): SHA-256 for the archive and all 4,589 members.
- Complete archive (removed artifact: `073-candidate-calibration.tar.gz`): raw operation observations,
  sources, fixtures, historical references, environment fingerprints, exact commands,
  authoritative schedule, durable attempt/completion journals and check logs.
- [Reproduction tool](../../demonstrator/candidate-scan-calibration/README.md).

Archive SHA-256: `8936d4c066669376cfce4bdacc0bee8dd7f1803d867ea9b0f9a90f755002864b`.
Every member was read back and hash-verified. Historical inputs remained unchanged.
Fourteen focused tests and repository lightweight checks (87 Python tests plus
configuration/documentation checks) passed. Analysis was byte-identical on repeat;
completed-run resume validated all records with fatal launch sentinels and zero
new processes. Existing renderer/build inputs and historical evidence are unchanged.

The archive contains `observations.json` with 160 paired operation observations
and `summary.json` with eight cells. Extract to a separate directory for inspection;
the tool validates pinned original paths before any execution or reanalysis.
No extra sampling, path-isolation patch, candidate qualification or integration
is authorized by this result.
