# Fixed prepared-state batching pilot

Approved by the owner's supplied-plan request on 2026-11-08. Scope is the isolated
Release harness and one fixed 297-process pilot. Retention remains not established.
No shipping renderer changes or later calibration are included.

Run `test_pilot.py` before execution. `run.py build OUTPUT` verifies historical
archives, member hashes and pinned inputs before building only `prepared-reflow`
with two workers. Existing baseline libraries are never regenerated. No benchmark
smoke run is allowed. `run.py run OUTPUT --from-directory BUILD_EVIDENCE` freezes
seed-73080 commands and the inherited environment, then runs nine correctness
processes followed by all 288 timing processes under the existing execution lock.

`run.py resume NEW --from-directory PRIOR` accepts only a validated completed
prefix and identical frozen inputs/environment for further launches. Attempted
processes are never repeated. Failure or ambiguity is terminal. No retries or
extra samples are supported. Completed resume launches nothing.

`report.py OUTPUT` produces deterministic observations and selection;
`check_resume.py OUTPUT FRESH_RESUME` proves byte-identical reanalysis and completed
resume under fatal launch sentinels. `package.py OUTPUT FRESH_EVIDENCE` archives
new evidence, source and build logs with every member hash read back. Existing
historical archives are referenced by identity rather than duplicated.

A selected size means provisional prepared-state throughput only. Parent snapshots,
operation boundaries, sampled RSS and inner instances have the limitations specified
in [the approved protocol](../../batching-pilot-proposal.md).
