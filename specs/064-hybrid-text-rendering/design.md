# Design

FormattedText owns the prepared layout and logical interaction coordinates. A lazily selected raster or scene-graph child owns drawing. Raster world-pixel alignment and common logical/physical extent correction are explicitly ported from V9 via V10, then moved to the child. Private geometry signatures remain; paintOffset becomes zero in logical owner coordinates.

A render-thread root owns persistent ordered code, Find, selection, glyph and focus subtrees, with separate revision snapshots. Each setter invalidates its affected subtree; layout preparation invalidates all. No GUI-thread scene-node pointer is retained. Normal Qt item deletion replaces the inactive surface and scene invalidation recreates all nodes.

Instrumentation is confined to separate diagnostic sources/tests, never shipping counters or renderer switches. Existing V9/V10 evidence remains immutable.

## Regression map

| Contract | Coverage |
| --- | --- |
| Native shaping, bidi, combining text, fractional motion | StylingTest::sceneGraphLifecycleAndStock against Qt Text |
| Raster alignment and common pixel grid | rasterPlacementTracksTranslations, presentedStrokesSurviveFractionalTranslations, strokesSurviveActualFlickable, rasterWindowLifecycle |
| Independent overlays and foreground restoration | overlayUpdatesRetainLayout; separate diagnostic node-identity assertions and bounded event analysis |
| Single surface across math source/result/disable/width/failure | MathTest::hybridBackendTransitions, existing inlineLayout and backendFailures suites |
| Formatting, padding, decorations, syntax/link precedence | styling, syntax-view, codeblocks suites; link precedence in native lifecycle test |
| Selection/copy, Find, navigation, accessibility, links | selection, reading and keyboard suites |
| Tables, images, footnote backlinks | view, selection, footnotes suites |
| Delayed custom fenced visuals | RendererTest::asynchronousDiagram; replacement, failure source, sizing/style, teardown and selection/Find source contracts |
| Window and scene invalidation; repeated thread lifetimes | native stock/lifecycle tests and separate 24-cycle memory/resource harness |

Tests inspect only upstream-owned items and public Qt APIs. The node-identity assertion and lifetime recorder are separate diagnostic patches, not shipping observer APIs. The logical owner still supplies a painter helper for the raster child and raster-only unit tests; only the child allocates a texture, aligns it and applies the drawing offset.

## Approved continuation: highlighting repository ownership

The scheduler prepares the built-in backend once, lazily in application-thread dispatch before
queuing its first job. Its private factory accepts the destination thread. Construction asserts
application-thread affinity; after constructing KDE's repository, remove its application event
filter before moving the repository to the worker. QMarkdown emits token roles, so repository
translation-change notifications are unnecessary. All subsequent repository access and backend
teardown occur on the worker; existing scheduler shutdown drains worker work before quitting.
The test backend factory continues to execute on the worker. Direct backend unit tests may use
the current application thread by omitting the destination. No public interface changes.

A real-backend scheduler regression treats every warning as failure through initialization,
asynchronous delivery, cancellation and scheduler teardown/recreation. Existing deterministic fake
backend tests retain cancellation, cache and delivery coverage. Fresh playground reproduction
precedes the fix. Exact container matrices and native DPR assertions remain qualification gates.
