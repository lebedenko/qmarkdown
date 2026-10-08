# Design

Approved on 2026-10-08 by the user’s explicit instruction to implement the supplied Feature 013 plan. Approval covers these requirements, design, tasks and format preparation only; package/module remain 0.7.0/0.7.

A private `prepareFormatIntervals` helper accepts UTF-16 text length and QVariant formatting/link lists and returns start, length, supported flags and link presence. It decodes each map once, validates integer conversion and bounds using subtraction before addition, and emits start/end events even for valid spans with no supported flags (their boundaries remain observable).

Sort events by offset. At each distinct offset apply all deltas to separate emphasis, strong, code and link counts, then emit the interval to the next offset if any supported count is active. Counts preserve duplicates and nested/overlapping spans; grouping handles coincident starts/ends independently of sort tie order. No interval merging. The helper is hidden, has no installed header or QML registration, and is compiled directly into isolated tests/benchmark.

`FormattedText::updatePolish` converts prepared intervals into its existing QTextCharFormat objects. All subsequent layout and painting code remains in place. Temporary event and interval storage is O(S); sorting is O(S log S), sweep is linear.

Correctness uses a deterministic independent per-character oracle plus explicit boundary expectations and renderer format/live replacement assertions. The optional benchmark uses valid dense inputs generated outside timing and retains the old QVariant boundary-scan algorithm only in tests. It validates exact interval equivalence before timing and records counts, all samples, medians, maxima and environment. Existing full Release benchmark is captured before/after with fixed Noto fonts and 480-pixel viewport. Remaining native layout costs are documented, not optimized.
