# Atomic handoff and live-only continuation

The [bounded proposal](../../scale-study-protocol.md#atomic-live-handoff-repair-and-bounded-continuation-proposal--2026-11-09)
replaces only the ten interrupted persistent windows. Execution approval is pending
at preparation; repairing and testing the harness does not launch viewers.

`handoff.py` serializes and fsyncs a same-directory temporary file, publishes it
with an atomic no-overwrite hard link, then fsyncs the directory. The waiting reader
never sees a partial final record. A crash before publication may leave a hidden
temporary file; it cannot release a checkpoint. Duplicate publication fails.

`run.py` loads the frozen v2 supervisor into an isolated module and replaces its
JSON publication function in memory. Historical files, viewers and checks are
unchanged. A subclass enforces the ten-cell live-only schedule. Execution requires
a separately recorded approval whose scope and schedule hash match the proposal.
The preparation command revalidates the terminal journal, frozen inputs, completed
fresh sets and restoration. It reserves prior automated time conservatively.

```sh
python3 -m unittest discover -s specs/073-rendering-architecture/demonstrator/scale-study-live-recovery -p 'test_*.py'
python3 specs/073-rendering-architecture/demonstrator/scale-study-live-recovery/run.py prepare build-ci/073-scale-study-v2-run build-ci/073-scale-study-live-recovery-run
```

After approval, retain its exact scope and schedule hash in `approval.json`:
`approved: true`, `scope: "ten replacement live windows; no fresh rerun"`.
The coordinator then requests 200%, records `confirm DIRECTORY 2 live-0`, and runs
`live DIRECTORY`. Subsequent handoffs use live-1 through live-6 with scales
1.5, 1.25, 1, 1.25, 1.5, 2. The existing windows remain open until completion.
No fresh-window processes are included. Restore the recorded original scale after
completion or termination; preserve all attempts and publish no commits.
