# Actual verification

For fresh transferred-checkout results, see [qualification continuation](continuation-verification.md).
The original status and results below are preserved as historical evidence.

Status: implementation and available qualification complete. Not ready for upstream integration/release: required Qt 6.8 checks are unavailable and syntax-enabled full packaging gates fail.

Base: clean sibling bbb0d0c1985567c4f1a7b71f82445c8fafd6ccf7, independent clone under md-viewer/build/diagnostics/qmarkdown-hybrid-064. V9/V10 archive hashes match their existing SDD records; sibling remains clean. No acquisition, package/preference change, API/version change, commit, PR or adoption.

## Completed checks

- Qt 6.12.0 clean provider: 25/25 offscreen/software suites. Both diagnostic providers also pass 25/25. Final overlay-only corner-radius refinement has focused styling coverage.
- Viewer integration against isolated candidate prefix: 68/68. Harness: 2/2. md-viewer task check: exit 0.
- Final native OpenGL/Vulkan stock-text and lifecycle checks pass at observed DPR 1/1.25/1.5/2, with explicit per-run assertions. The inherited harness initially used scale factors, producing actual DPR 1.5/1.875/2.25/3 on this display; both successful multiplier matrices are retained but do not qualify the requested DPRs. V10 native/opengl-1.log also reports DPR 1.5, correcting the earlier interpretation of that historical matrix without changing its evidence. Native selection, keyboard, reading/Find, syntax, math, footnotes and asynchronous renderer extension suites pass.
- Overlay diagnostic: 75 surface updates, zero layout rebuilds, glyph builds or raster paints in the bounded interval. Render-thread assertions preserve glyph-node identity. Software and native image restoration tests pass.
- Source distribution builds/installs offline; its installed consumer runs successfully. The consumer has no CTest registration: a no-tests CTest invocation is retained and is not evidence of consumer execution.
- Math-disabled shared/static focused suites pass. Qt 6.12 syntax-disabled shared/static full suites pass 34/34 each, with relocated consumers and library-only builds passing. Syntax-enabled shared/static full suites each pass 33/34; independent relocated consumers and library-only builds pass, while both full gates remain failed. Final focused shared styling/view checks pass after the corner-radius refinement.

## Wheel negative-control correction

Original V10 failure evidence is preserved. The first strengthened control asserted delivery and found zero handler invocations natively, for both accepting and passive cases. An explicitly constructed synthetic Mouse device fixes delivery; accepting-blocker and passive-observer rows pass on software and native Wayland. Qt WheelHandler defaults to accepting mouse devices; the test now verifies invocation as well as scrolling. A separate eight-case Qt-only probe reports the native primary device as TouchPad/Finger and confirms zero delivery for that device. Explicit Mouse delivery reaches the handler once in every case; blocking prevents outer scrolling whether the event is accepted or rejected, while passive observation permits scrolling. This isolates device filtering as the original failure cause. No production input behavior changed. See [Qt WheelHandler](https://doc.qt.io/qt-6/qml-qtquick-wheelhandler.html).

## Additional qualification failure

The syntax-enabled playgroundReadingControls test fails on QObject::installEventFilter cross-thread warnings. Plugin isolation does not resolve it. An offline gdb backtrace places the warning in construction of the existing highlighting Backend on its worker thread. syntaxbackend.cpp is byte-identical to V10, whose preserved native syntax-view, footnote and code-block logs contain the same warning. This failure is retained, not ignored or called passed. A diagnostic runner continues independent packaging checks but returns failure when any full CTest gate fails. No unrelated highlighting change is included.

## Memory and resources

Separate native 24-cycle processes repeatedly replace documents, switch plain/padded-code content, recreate windows and invalidate/recreate scene graphs. This public-API memory harness uses default renderer options plus selection and Inter 9pt; it is separate from the replay fixture configuration. Clean candidate RSS across the last 12 released cycles grows 16 KiB reduced / 40 KiB original, versus V10 3948 / 16 KiB. Candidate first/last settled RSS: 381704/403268 KiB reduced and 392812/404840 KiB original; V10: 404612/426572 and 401816/413264 KiB. These are bounded process-memory observations, not a proof against all leaks.

Diagnostic counts balance to zero live surfaces and roots for both providers and fixtures after teardown. Candidate peaks: 3/65 surfaces and 1/51 roots; V10: 6/130 surfaces and 2/63 roots (reduced/original). All four processes record 49 thread lifetimes without overflow. Slot leases reuse recorder storage after thread exit. Instrumented RSS is recorded separately and not used as clean memory evidence. Graphics/driver allocation and cache residency are not directly measured; Qt-owned child nodes are covered by root ownership/teardown, not individual GPU allocation counters.

## Unavailable and pending

Docker 29.9.1 is available, but its cached inventory has no qmarkdown-ci images. No local Qt 6.8.0 SDK was located; no missing toolchain/image was acquired. Required Qt 6.8.0 shared/static/optional/consumer checks remain unverified. Perceived shimmer, live monitor migration, exotic scales/transforms, screen-reader sessions and owner visual acceptance remain unassessed. The bounded source-package native Wayland/OpenGL smoke passes at runtime-reported DPR 1.5. Replay results are below; archive integrity is recorded in SDD 0018 and the adjacent archive checksum.

## Fixed replay qualification

All 28 trials completed, with 18 measured trials qualifying and 10 disabled controls remaining unmeasured. No retry, altered threshold, changed trace or discarded timing attempt. All measured runs applied the same 563 trial-5 offsets in source order and within bounds. Runtime checks confirm native Wayland/OpenGL, effective DPR 1.5, Inter 9pt, 600-pixel column, viewport (0,27,960,667). p95 must be at most one measured refresh interval; maximum at most two.

| Provider / fixture | p95 lateness, ms (1 / 2) | Maximum, ms (1 / 2) | Qualification |
| --- | --- | --- | --- |
| V10 / reduced | 1.777 / 1.788 | 2.119 / 3.318 | Pass / Pass |
| Candidate / reduced | 1.772 / 1.798 | 3.216 / 4.378 | Pass / Pass |
| Stock Qt / reduced | 1.846 / 1.768 | 3.278 / 4.061 | Pass / Pass |
| V10 / original | 3.272 / 4.375 | 9.422 / 11.417 | Pass / Pass |
| Candidate / original | 5.784 / 4.105 | 9.813 / 6.307 | Pass / Pass |

Both providers retain V10's scroll-performance qualification. These samples do not establish a speedup: candidate original repetition 1 has higher p95 lateness than V10. All eight separately instrumented runs also qualify, but their timings are not substituted for clean runs.

Each reduced replay has one eligible paragraph, 562 changed-offset callbacks, zero paragraph raster paints, zero layout rebuilds and zero glyph builds. Original replays have 111 scene-graph and 12 padded-code raster instances among 123 participating text owners. All eligible nodes were built before replay; eligible raster paints and all layout/glyph rebuild counts remain zero. Counts describe participating instances, not viewport area or full-renderer coverage.

V10: 6648 / 6516 raster paints; 3.070 / 3.185 seconds in paragraph paint helpers.

Candidate: 6588 / 6636 raster paints; 2.951 / 2.985 seconds in paragraph paint helpers.

Residual raster cost is retained explicitly. Paint scopes cover the paragraph painter helper, not GPU execution or compositor presentation. Coalesced offsets are not counts of dropped displayed frames. Full synchronization, timer, render, overlap and fallback statistics are in measurements.json and backend-coverage.json.

## Final test refinement and source identities

After the fixed timing matrix, the wheel test alone gained an explicit rejecting-blocker row, preserving the original negative-control condition alongside the accepting-blocker and passive-observer rows. This follows the completed Qt-only contract probe. Final code-block software/native results are recorded in the delivery logs. Renderer and replay-host binaries remain byte-identical to those measured (measured-binary-hashes.json); there was no renderer change or timing rerun. The new math transition test passes in shared/static and math-disabled configurations. Diagnostic instrumentation remains a separate patch.

The final source distribution is built from working candidate bytes, not the unchanged Git HEAD. Its archive keeps upstream's 1.17.0 package version solely for this isolated candidate; it must not replace the published 1.17.0 artifact. Implementation and diagnostic patches are independently checked by reconstruction and byte comparison during packaging. No commit or publication was created.

Final-source memory refresh: memory-final/ repeats the candidate clean and instrumented cases after the corner-radius invalidation refinement; unchanged V10 controls are reused. The final figures above use those refreshed runs. Original memory/ samples and memory-analysis-initial.json remain retained. Counts again balance to zero across 49 thread lifetimes, without overflow. Final full code-block suites pass 39/39 on software and native Wayland.

## Workspace transfer (2026-10-21)

The results above are historical evidence executed from the md-viewer diagnostic workspaces, not
builds of the transferred checkout. The statement that the sibling remains clean describes that
historical snapshot. QMarkdown now contains the uncommitted candidate; see [handoff](handoff.md)
for transfer-only verification. No runtime result has been upgraded or rerun by the transfer.
