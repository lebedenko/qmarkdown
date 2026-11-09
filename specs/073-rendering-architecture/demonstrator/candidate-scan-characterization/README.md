# Offline workstation characterization

Approved 2026-11-08 for existing-evidence analysis and a draft pilot proposal only.
No harness, build, new measurements or historical modifications are authorized.

From the repository root:

```sh
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-characterization/test_characterization.py
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-characterization/analyze.py build-ci/073-candidate-characterization
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-characterization/verify.py build-ci/073-candidate-characterization
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-characterization/package.py build-ci/073-candidate-characterization specs/073-rendering-architecture/evidence/candidate-scan-characterization
```

Analysis requires the original pinned local identities; missing/inconsistent inputs
stop the tool, never trigger regeneration. Output must be fresh. Historical sources
are imported unchanged and all analysis runs under fatal launch sentinels. The
complete matrix has 1,280 process-operation rows, 320 group-operation rows and eight
cells, exported as JSON plus three CSV tables. CSV nulls are literal `null`; nested
cell values are JSON. Process timestamps never imply operation start/end timestamps.

The [pilot proposal](../../batching-pilot-proposal.md) and 297-command specification
are drafts requiring separate implementation/execution approval. The new executable
path is a literal placeholder. This tool never executes a pilot command. Retention
remains not established. See the [evidence](../../evidence/candidate-scan-characterization/README.md)
for actual verification and observations.
