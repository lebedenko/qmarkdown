# Bounded correctness evidence

The authoritative final records are [geometry-lines.json](geometry-lines.json),
[native.json](native.json), [lifecycle-native.json](lifecycle-native.json) and
[support.json](support.json). Full logs, commands, input/font/binary identities
and captures are in the matching archives. [archives.json](archives.json) records
archive hashes; [current-binaries.json](current-binaries.json) confirms the viewer
binaries remained unchanged by the final geometry-test refinement.

The native archive stores identical files as ordinary tar hardlinks. Extraction
with Python's data filter was verified against every original file:
[native-archive-check.json](native-archive-check.json). This preserves all captures
and their paths without storing repeated image bytes separately.

Baseline files and trace-before logs reproduce the original Qt error. The
standalone `*-after.jsonl` files and `geometry-summary.json` are exploratory runs,
not final acceptance evidence. In particular, `minimum-after.jsonl` was collected
before the minimum rebuild completed and still records the original failure.
The final exact-SDK results are in the authoritative geometry archives above.
`geometry-final.tar.gz` preserves the earlier fragment checker; geometry-lines
uses line membership and visual adjacency rather than fallback-font ink centers.

The interrupted native run predates ordered selection endpoints. Its selected
capture copied correctly but lacked a highlight; it is explicitly unaccepted.
The source snapshot, representative before/after images and interrupted sample
archive preserve that failure. The separate sandbox-denied lifecycle archive
records an unavailable Wayland connection, not a tested backend result.

See [verification](../../verification.md) for interpretation, actual checks and
capabilities required next. None of these records supplies owner visual acceptance,
performance qualification, production migration or backend selection.
