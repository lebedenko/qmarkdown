# Requirements

Approved on 2026-10-09 through the explicit user approval of Iteration 019. Approval covers requirements, design and tasks; scope is tests/documentation only. Production changes require revised approval.

## Purpose

Feature 017 completes independent semantic evidence for all 652 pinned CommonMark examples. Native presentation is a separate v1.0 gate. Existing view/resource tests cover substantial behavior, but there is no consolidated requirement-to-check assessment. Complete that assessment and close bounded test gaps before deciding release readiness.

- R1: Maintain an explicit native-presentation coverage matrix for paragraphs, H1–H6, thematic breaks, fenced/indented code, tight/loose and nested lists, quotes, mixed containers, emphasis/strong/code, breaks, escapes/entities, links/autolinks, image fallback/rows and literal raw HTML. Each row identifies the presentation contract, named executable checks, environment and actual result; unresolved gaps remain visible.
- R2: Verify settled native layout at ordinary, narrow, zero and restored positive widths, with wrapping, contentHeight/implicitHeight, spacing/indentation, empty containers, live style changes and Markdown replacement. Preserve documented indivisible-glyph overhang and literal code behavior; do not invent new layout requirements.
- R3: Verify the presentation boundary for host-controlled navigation, deny-by-default resources, formatted image descriptions, authorized image arrival, policy revocation and stale completion. Reuse existing resource-policy/lifecycle checks; native checks must prove the displayed result where controller checks alone cannot.
- R4: Keep complete CommonMark semantic evidence separate from native coverage. Strict verification must retain 652 parser/model passes with zero mismatches, uncheckable examples or unresolved limits/losses.
- R5: Run focused checks, shared/static CTest and both pinned local Qt CI tasks. Record actual results and limitations separately from plans. Correct stale current-status documentation without erasing historical verification.
- R6: Preserve production behavior, dependencies, public interfaces and package/module 0.7.0/0.7. Report discovered defects explicitly; production fixes require a revised approved scope. Do not declare or publish v1.0 in this iteration.

Excluded: exhaustive screenshots of all official examples, pixel-identical cross-platform rendering, new syntax, selection/copy, streaming, rendering optimizations, decoder scheduling changes, hosted workflow execution, version bumps and release publication.
