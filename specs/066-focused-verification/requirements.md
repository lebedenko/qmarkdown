# Feature 066: Focused CI and local verification

Approved by the user implementation request on 2026-10-23. Scope: the supplied Feature 066 plan, including three CI profiles, exact Qt targets, grouped current-version consumers, compiler caching, local hooks, release protection and an open optimization PR. Public APIs, package discovery semantics, versions and historical records remain unchanged.

Light validates documentation, changed evidence syntax and Python tools. Core verifies shared runtime, lint, privacy and installed consumers on both exact targets, adding syntax and math coverage when affected. Full retains all linkage/backend combinations, math-disabled builds, one library-only build and development benchmark smoke. Unknown inputs and ci:full escalate to Full. Only schema-identified Full evidence permits publication.

Local hooks validate staged/committed inputs without modifying the working tree; pre-push incrementally verifies shipping inputs on exact development Qt. Missing prerequisites fail without downloads. The 8–12 minute Core target is a measurement goal, not an acceptance claim.

## Acceptance requirements

- R1: Classify the complete PR diff into Light/Core/Full, combine mixed requirements, expand renames/deletions, escalate unknown paths and honor `ci:full` without reducing coverage.
- R2: Run Qt-independent checks once; retain parser/CommonMark with runtime tests. Keep required packaging names and exact independent minimum/development targets, `fail-fast: false`, and per-PR cancellation.
- R3: Core runs shared runtime/lint/privacy/current installed consumers, with additive syntax/provider/code-block and math-disabled coverage. Full retains four principal configurations, two math-disabled variants, one default library-only configuration and development benchmark smoke.
- R4: Group consumer configurations while preserving separate processes, shared-linked/shared-plugin/static-plugin loading, current feature assertions and exact metadata-derived package/QML versions. Remove historical requests; retain SameMajorVersion discovery policy.
- R5: Use Ninja, two compiler workers and serial GUI tests. Persist build-only ccache by recipe/Qt/architecture; correctness is independent of cache availability. Report durations/statistics without claiming unmeasured speedups.
- R6: Separate image/Core/Full deadlines at 20/20/45 minutes; hosted outer budgets are 50/75 minutes and supervisor collection has extra time. Collect available evidence unconditionally with existence checks and bounded retention, acknowledging forced termination limits.
- R7: Only successful schema-identified Full runs produce publication evidence, retaining final-head/tree/Qt/version checks and all source archive, Arch package and downloaded-hash publication safeguards.
- R8: Provide check/check-push/install-hooks tasks and framework-free staged pre-commit and committed pre-push hooks. Handle non-HEAD tips, new/deleted branches and multiple refs without modifying the checkout. Lock stable exports, preserve timestamps/build outputs and require exact configured host Qt without downloading tools.
- R9: Keep experiments outside shipping CI, syntax-check changed evidence and require their own local exact-Qt plans. Document bypasses and distinguish local success from hosted release evidence.
- R10: Deliver this optimization as one PR from current main, keep it open with auto-merge disabled, exercise final-head Full checks and report cold/warm conditions separately. Do not create evidence-only commits or change PR 49 before this policy merges.
