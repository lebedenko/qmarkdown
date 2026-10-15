# Requirements — QML responsibilities and private contracts

Status: **approved on 2026-10-15** by the user response “approved”.
Approval covers requirements R1–R5, design and tasks T1–T7 implementation and
verification. Merge and publication require separate authorization.

## Scope

Address all four boundary findings in one behavior-preserving change. Keep
package 1.9.0 / QML 1.9, supported public APIs and dependencies unchanged.

- R1: C++ owns bounded search-range processing. Queries return only intersecting,
  clipped, item-local UTF-16 ranges with current-result flags. QML bindings refresh
  when matches or the current result change, including unchanged result counts.
  Preserve cross-block and image-description highlights and selection above search.
- R2: MarkdownView retains the public facade, ViewState, keyboard handling and
  document selection gestures. Private QML components own recursive block layout,
  text, image and list presentation with explicit inputs and relayed link/copy
  signals. Preserve fast native Text, selectable/searchable paths, lazy recursive
  loaders, style replacement, heading decoration and zero-width behavior.
- R3: Code components consume explicit inputs rather than the whole view. Preserve
  host creation contexts, dynamic bindings/destruction, renderer fallback, token
  invalidation, synchronous host replacement and stale-copy suppression. Derive
  fence language once in the C++ presentation projection with existing semantics.
- R4: Type component style inputs as MarkdownStyle and validate table-loaded
  sequence instances before property assignment. Replace dynamic projection
  property reads with typed private selection registrations, including text/image
  metadata and exclusion-only table/action items. Dependency replacement and
  QObject destruction must leave no stale registrations.
- R5: Deliver focused regressions, advisory search benchmarks, private tooling
  metadata, resources, installed payloads and source-archive inventories together.
  Verify configured Qt 6.8.0 and 6.12.0 shared/static packaging and installed
  consumers without source-tree imports; distinguish planned and actual evidence.

## Boundaries

Retain Layout.js, measurements, visual composition, gestures and viewer UI policy
in QML/JavaScript. Exclude viewer restructuring and broader ViewState decomposition.
No HoloNight dependencies, document HTML, QTextDocument or WebEngine rendering.
One focused PR; merge and publication require separate authorization.
