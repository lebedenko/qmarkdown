# Candidate-scan diagnosis evidence

**Retention remains not established.** See the [diagnosis report](report.md).
The unchanged qualification has eight failed overhead confidence-bound gates;
every failed interval includes zero. Added compiled work on false-padding paths
is plausible, but the timing samples do not establish causal attribution.

The [archive index](archives.json) identifies the separate diagnosis archive and
every member by SHA-256. Extract the archive to inspect:

- `cells.json`: all 176 cells and 3,520 pairs, including null operations, raw timings,
  normalization, order, timer limitations, process duration/RSS and descriptive
  distributions, order medians and first/last-ten medians.
- `failed-pairs.md`: every observation from all eight failed cells.
- `context.md` and `recomputed/`: complete-matrix context and byte-identical original
  summary/report, preserving every original gate.
- `binary/`: nm/objdump versions, commands, symbol sizes and relevant disassembly.
- `sources/`, `analysis/`, `checks/`, `integrity.json`, `preservation.json`: verified
  affected source copies, reproduction tooling, verification logs and input hashes.

Raw historical inputs stay in the unchanged [continuation archives](../candidate-scan-continuation/README.md).
The diagnosis references their exact index/member hashes rather than duplicating
historical inputs. All 4,343 members and 2,605 existing identities were checked
before analysis and again afterward. No benchmark was launched or rebuilt.

The [analysis README](../../demonstrator/candidate-scan-diagnosis/README.md) gives
reproduction commands. Existing source/binary paths must still satisfy their
recorded identities; missing or mismatched evidence must be repaired explicitly,
never regenerated silently. Follow-up experiments are proposals only.
