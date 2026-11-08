# Candidate-scan diagnosis

CD1–CD4 approved by the owner's supplied-plan implementation request on 2026-11-08.
Read-only analysis of the completed continuation. No benchmark binaries are
executed, rebuilt or instrumented. No new dependencies or changed gates.

From the repository root, using a fresh output directory:

```sh
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-diagnosis/test_analysis.py
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-diagnosis/analyze.py build-ci/073-candidate-diagnosis-new
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-diagnosis/package.py build-ci/073-candidate-diagnosis-new /tmp/073-diagnosis-archive-new
```

Python standard library plus the existing `nm` and `objdump` are required.
Existing historical absolute source/binary paths must still match identities.
Missing or inconsistent inputs raise an error before statistical analysis; partial
outputs are not qualifying evidence. Do not rebuild or repair inputs implicitly.
The unchanged original reporter runs on a private extracted copy. Its summary and
Markdown report must match the archive byte-for-byte, with eight failures.

`cells.json` contains all 176 cells and 3,520 pairs, including null operations.
`original` preserves the unchanged report and gates. Descriptive differences and
overhead retain all observations; MAD is unscaled. Order medians use recorded
first arm; halves use samples 0–9 and 10–19. Null descriptions remain null. Timer
suppression remains the original reporter's; no new ratios are calculated.
`failed-pairs.md` exposes every failed-cell observation; `context.md` exposes all
original cell gates. `binary/` retains tool versions, commands, symbols and
address-bounded disassembly. `sources/` preserves affected verified source copies.
`integrity.json` identifies every input member and source/binary; `preservation.json`
records final historical hash checks and deterministic presentation.

The archive packages analysis sources, outputs, input hashes, disassembly and any
verification logs in `checks/`, using stable tar/gzip metadata and read-back hash
verification. Raw historical inputs remain in the unchanged continuation archives;
the diagnosis archive references them through the verified member and index hashes.
The output directory's `inputs/` and `recomputed/` are private copies, not replacements
for historical evidence. There is no commit, merge or publication step.
