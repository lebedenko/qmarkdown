# Corrected complete scale study

The [approved amendment](../../scale-study-protocol.md#corrected-complete-scale-study--approved-2026-11-09)
authorizes one new complete 690-process schedule, preserving the original failed
attempt: 691 total graphical launches. The same verified viewer binaries are reused;
rendering, layout, fixtures, tolerances and source-copy checks are unchanged.

`study.py` resolves both wl_output names and xdg-output names through their object
bindings. Missing/conflicting bindings and wrong output identities fail closed.
Process facts are durably recorded before validation, and exceptional failures
receive an immutable result and completion event before a terminal stop. The
original attempt reserves 120 seconds conservatively within the four-hour budget.

Eighteen offline tests include the actual archived Qt 6.8 trace, full offline
revalidation of its captured records, negative/mixed protocol cases, failure-fact
preservation and terminal rejection under a fatal process-launch sentinel.
Offline revalidation does not relabel the original failed attempt.

```sh
python3 -m unittest discover -s specs/073-rendering-architecture/demonstrator/scale-study-v2 -p 'test_*.py'
python3 specs/073-rendering-architecture/demonstrator/scale-study-v2/study.py freeze build-ci/073-scale-study-final build-ci/073-scale-study-v2-run
```

After each owner scale handoff, run `confirm DIRECTORY SCALE fresh-SCALE`, then
`fresh DIRECTORY SCALE` using this sibling study.py. The sequence remains 1, 1.25,
1.5, 2. `live DIRECTORY` retains ten windows through the seven original checkpoints.
The original output is recorded at freeze time and restored after completion/stop.
No additional retries, performance conclusions or backend selection are authorized.
