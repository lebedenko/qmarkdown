# Prepared-state path isolation

The [approved protocol](../../prepared-path-isolation-proposal.md) fixes one
2,001-process experiment and its terminal stop rules. Retention is never established
by this diagnostic experiment. Historical directories are immutable inputs.

`verify_inputs.py FRESH_PREFLIGHT` reproduces the initial historical archive/input
checks. `prepare_build.py` makes the exact `split.py` transformation and builds matched
Release drivers with two workers. `audit.py` checks final disassembly; `diagnostics.py`
builds separate instrumentation with the archived scanner/numerical tests and a
late-fallback injection. `freeze.py` verifies input identities and measured false
padding paths. `test_isolation.py` runs offline under fatal process-launch sentinels.

The executed preparation uses sibling workspace
`build-ci/candidate-scan-prepared-isolation`; `run.py run OUTPUT` freezes all 2,001
commands and context before launching. It journals every attempt, validates the
81-result gate and runs serially under the historical lock with 60-second and 4 GiB
limits. `run.py resume FRESH --from-directory PRIOR` accepts only a validated
completed prefix with unchanged frozen context. Never repeat a failed or ambiguous
attempt, and never run these commands as an additional experiment.

`report.py OUTPUT` applies seed 73085 and the archived exact tolerances.
`check_resume.py OUTPUT FRESH` verifies completed recovery and byte-identical
reanalysis with fatal launch sentinels. `package.py OUTPUT FRESH_EVIDENCE` archives
both successful and terminal evidence and reads back every member hash. The
archive retains historical references instead of duplicating extracted archives.

The delivered evidence exposes `report.md`, `decision.json`, a compressed
`analysis.json.gz` sidecar and the full archive. The sidecar is byte-identical after
decompression to `analysis.json` inside the archive. Frozen tool bytes in the
archive are authoritative for strict identity checks; any later presentation-only
source cleanup is identified in the adjacent delivery checkpoint.

`partition_archive.py SOURCE_EVIDENCE FRESH_DESTINATION` applies the historical
64 MiB member grouping convention to a verified single archive, preserving every
original member byte and adding the partition tool. Final delivery uses ten parts.
For strict frozen-input replay, restore the archived `tool/test_isolation.py` in
an isolated copy of the recorded input environment; the delivered file differs
only by removal of trailing spaces on two blank lines, as recorded in
[the cleanup hashes](../../evidence/candidate-scan-prepared-isolation/source-cleanup.json).
