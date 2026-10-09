# Design

Approved on 2026-10-09 through the explicit user approval of Iteration 019. Approval covers requirements, design and tasks; scope is tests/documentation only. Production changes require revised approval.

## Coverage and evidence

Add `coverage.md` in this directory during implementation. Audit `tests/tst_view.cpp` and `tests/tst_resources.cpp` assertion by assertion, mapping R1–R3 contracts to named tests and data rows. A test name alone is insufficient evidence: describe the observable assertion and any limits. Mark rows covered, partially covered or missing before adding tests, then record executed results after verification. Parser-only assertions cannot close native presentation rows.

Reuse existing Qt Test executables, test helpers and CTest/CI integration. Add only missing behavior checks; do not create another renderer, corpus oracle, public API or test framework. Curated source fixtures exercise native mapping and representative combinations; the complete official corpus continues to verify semantics independently. Native coverage does not claim that every corpus example has been visually checked.

## Layout and interaction checks

Use the public MarkdownView/MarkdownStyle/resourcePolicy contract to drive changes. Inspect native item text, effective QTextLayout formats, geometry, line counts and activation signals through the established private test helpers. Wait for Qt Quick polish with existing bounded retry patterns. Assert relationships and finite geometry rather than hard-coded font-specific heights. Use explicit fonts where required by a comparison; retain the existing point/pixel typography checks.

Prioritize gaps involving mixed containers, raw HTML literal display, preserved semantic structures versus their flattened presentation, and width/style/document transitions. Reuse existing heading/code/container checks if they already demonstrate the contract. A zero-width view suppresses layout; restoring width must reproduce the current document and style. Link and image fixtures use controlled local resources or the existing injected network manager, never live Internet requests.

Existing tests such as `containerLayoutAndStyle`, `leafLayoutAndStyle`, `combinedFormatsAndLiveRanges`, `nativeLayoutAndReplacement`, `linkInteraction`, `imageRowsAndPolicyLifecycle`, `staleDecoderCompletion` and `activeDecodeDestruction` are audit inputs, not assumed complete coverage. Preserve the documented blocking active-decode destruction limitation; this plan assesses evidence without changing its scheduling contract.

## Verification and documentation

Run newly added or affected view/resource cases first, then strict corpus verification and shared/static suites. Run `task ci-6.8` and `task ci-6.11` using Feature 018 cached toolchains and fresh builds. Keep artifact paths, Qt/tool versions, exit codes, counts and warnings in verification.md. Update README's stale 626/26 baseline and strict-failure description to Feature 017's current result, linking historical records rather than rewriting them. Update standards/roadmap with the actual native assessment and remaining release decisions.

Offscreen geometry, native layout and synthetic input provide deterministic evidence, not a claim of GPU, assistive-technology or every-platform verification. If an assertion exposes a production defect, retain a precise reproduction and mark the affected matrix row unresolved; request approval for a concrete revised fix plan before production edits. Release readiness remains a separate decision even when all planned checks pass.
