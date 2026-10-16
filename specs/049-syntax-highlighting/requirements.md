# Requirements — syntax highlighting

Approval: approved on 2026-10-16 through the user instruction to implement the supplied “QMarkdown 1.11: syntax highlighting” plan. Approval covers requirements, design, mapped tasks, verification and one package 1.11.0 / QML 1.11 release PR. Merge, tagging and publication require separate authorization.

R1. Optional private KSyntaxHighlighting >= 6.10 backend; source default OFF, Arch ON. Public QML APIs exist in either build.
R2. Per-view opt-in defaults false. Read-only availability and replaceable SyntaxTheme with view-owned Light fallback, resettable role colors, persistent overrides and live edits.
R3. Fenced source only, case-insensitive first-token aliases then installed names/extensions; no content detection. Unknown/empty and indented code remain plain. Case-sensitive custom renderers retain precedence, selection/Find source can highlight, copy remains exact.
R4. Backend-neutral sorted nonoverlapping UTF-16 spans; stateful line processing; native layout/glyph colors preserve wrapping, alignment, empty sizing and terminal display-newline omission.
R5. One application-owned worker/repository; asynchronous complete results, generation/lifetime guards, cancellation, coalescing, deduplication and theme-independent LRU (128 entries / 8 MiB including source/spans).
R6. Entire-block plain fallback at 256 Ki UTF-16 units, 16 Ki per line, 65,536 spans or 100 ms cooperative budget. Check cancellation/time between lines and callbacks; individual backend calls cannot be interrupted.
R7. Pinned KDE/ECM 6.10 CI with verified hashes/provenance on Qt 6.8.0/6.12.0; shared/static OFF/ON, installed/extracted/Arch consumers and historical imports. Document licensing without vendoring definitions.
R8. Include playground, docs, tests and release preparation in one PR; finalize current summaries together, strict docs and final-head CI; preserve historical records. Public providers/C++ API, Tree-sitter, streaming, token emphasis/fonts and arbitrary themes are excluded.
