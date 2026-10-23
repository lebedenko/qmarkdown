# Actual verification

Recorded 2026-10-23. Approval is the user's Feature 066 implementation request; this record distinguishes local measurements from final-head hosted evidence.

- `python3 scripts/check.py`: passed, including documentation links, YAML/Python/shell/configuration/evidence syntax and 87 Python tooling tests. Fixtures cover profile escalation/mixed changes, both rename paths/deletions, nested CMake and benchmark inputs, optional features, Full-only release schema and stale/mismatched/incomplete evidence, source snapshots, pushed tips/deletions/multiple refs, missing ancestry/Qt, locking, foreign hooks, compressed JSONL, timeouts and cancellation.
- `python3 scripts/git-hooks.py pre-commit`: passed against both the initial staged snapshot (84 tests) and final implementation snapshot (87 tests). The isolated snapshot fixture also verifies unchanged timestamps and excludes unstaged edits.
- Shell syntax, Python compilation and Git whitespace checks passed. Existing public source APIs, package versions and SameMajorVersion policy were not changed.
- The first container attempts failed before Qt verification because local Docker lacked Buildx and rejected `--progress`; removing that unnecessary output-format flag enabled the available builder. Preparation failure evidence was retained, without claiming test success.

## Exact-Qt Core measurements

Both runs used fresh build directories, initially empty compiler-cache namespaces, two compiler workers, offscreen/Fusion rendering and serial CTest. They ran concurrently on this local host, so these measurements are not hosted runner predictions. Image preparation used the legacy local Docker builder. Source snapshots and full logs remain in the ignored run directories listed below.

| Qt | Preparation | Verification | Total | Compiler hits/calls | Result |
| --- | --- | --- | --- | --- | --- |
| 6.8.0 | 242.855 s | 278.309 s | 521.338 s | 17/272 (6.25%) | Passed |
| 6.12.0 | 308.915 s | 291.783 s | 600.870 s | 17/273 (6.23%) | Passed |

Each passed all 29 runtime/CommonMark tests, QML lint, symbol privacy, installation/relocation checks and all 30 separate current-version shared-linked/shared-plugin consumer processes. These were Core runs; static and optional configurations are not implied by these results.

Retained local runs: `build-ci/6.8.0/20261023T114846Z-h9_5d7pa` and `build-ci/6.12.0/20261023T114851Z-afo3mpm3`, with `result.json`, `phases.txt`, `ccache-stats.txt`, provenance and logs. These are working-source measurements, not hosted release-gate evidence. Classifier edits change the existing toolchain-context fingerprint; subsequent timings must identify whether that namespace was cold or warm rather than assuming a previous run warmed it.

## Delivery evidence

Final-head hosted Full results, total duration and cache conditions are recorded in the optimization PR after checks finish, without evidence-only commits that would restart verification. A warm measurement is reported there only after verifying cache reuse. The 8–12 minute Core target remains a performance goal; the local measurements above do not establish a hosted guarantee. Leave the PR open and unmerged with auto-merge disabled. PR 49 remains unchanged.
