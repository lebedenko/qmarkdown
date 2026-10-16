# Tasks — syntax highlighting

Approval: approved on 2026-10-16 through the user instruction to implement the supplied “QMarkdown 1.11: syntax highlighting” plan. Approval covers requirements, design, mapped tasks, verification and one package 1.11.0 / QML 1.11 release PR. Merge, tagging and publication require separate authorization.

| Task | Requirements | Work and checks |
| --- | --- | --- |
| T1 | R1, R2 | Palette, view plumbing, optional dependency; test default/override/reset/replacement/destruction and OFF/ON builds. |
| T2 | R3–R6 | Adapter, language resolution, scheduler/cache/limits; test languages, multiline/Unicode, cancellation/stale results, dedup/eviction and limits. |
| T3 | R3, R4 | Native color integration; test pixels, wrapping/alignment/empty code, custom precedence, selection/Find and exact copy. |
| T4 | R7 | CI dependency provisioning, source inventory, static discovery, Arch and installed consumers; verify matrix and packaging. |
| T5 | R8 | Playground/docs/release 1.11; strict docs, advisory performance, actual evidence and one final-head verified PR. |
