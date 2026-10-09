# Tasks

Approved on 2026-10-09 by explicit user approval. Scope covers requirements, design and tasks for the responsive decoder lifecycle and its verification.

- [x] T1 (R1–R3,R5,R6): Implement private shared jobs and application-owned two-worker FIFO scheduler, synchronized cancellation, dispatch and shutdown draining; integrate private sources with existing build targets.
- [x] T2 (R1,R3,R4): Replace controller pool ownership with job handles, nonblocking cancellation/destruction and guarded completion; preserve per-view policy/network/cache and sequential admission/deadlines.
- [x] T3 (R1–R5): Replace blocking-destruction evidence and add gated replacement/new-controller progress, two-worker cap, FIFO, queued cancellation/replacement, controller independence, queued expiry and application-drain checks. Retain existing regressions.
- [x] T4 (R1,R6): Add native view/engine teardown checks during held decoding with safe gate cleanup; verify no publication after destruction.
- [x] T5 (R6,R7): Run focused checks, strict CommonMark, shared/static CTest, task ci-6.8 and task ci-6.11. Record actual commands/counts/artifacts/limitations separately from plans.
- [x] T6 (R6,R7): Update README lifecycle text, specification index and current native coverage assessment; preserve historical results, inspect final diff and record completed scope.

Explicit approval of requirements.md, design.md and this task list was recorded before production edits. All authorized tasks are implemented and verified; actual results and remaining environment/lifecycle limits are recorded separately in verification.md.
