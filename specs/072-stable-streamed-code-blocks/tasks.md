# Stable streamed code blocks

Approved on 2026-11-04 through the user-supplied implementation plan and explicit request to implement it. Approval covers requirements, design, mapped tasks, implementation and verification. Merge, publication and system-package replacement require separate authorization.

## Mapped tasks and checks

- T1 (R1, R3): publication context, old geometry capture, generation-guarded restore and bottom-following. Check code-block and streaming cases for both ownership paths, overflow, pause/resume, closure, flush, disabling and replacements.
- T2 (R2): stable text component and private prefix-span retention. Check syntax requests and views with delayed results, empty/error/cancellation and configuration changes.
- T3 (R3, R4): update guides, capture a rendered sequence; report native acceptance separately.
- T4 (all): focused tests, then Core with syntax on exact Qt 6.8.0 and 6.12.0 (runtime, lint, symbol privacy, installed consumers), documentation checks and one reviewable PR. No merge/publication/system-package replacement.

Actual execution belongs in [verification](verification.md).
