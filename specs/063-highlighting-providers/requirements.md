# 063 — Host highlighting providers requirements

Status: explicitly approved by the user on 2026-10-20.
Target package 1.17.0 / QML 1.17. Merge, tagging and publication are excluded.

- **R1 — Public QML contract.** Add host-owned `SyntaxHighlightingProvider` and
  nullable `MarkdownView.syntaxHighlightingProvider`, default null. A provider
  emits `highlightRequested(request)`. Requests expose read-only `source`,
  `language`, `cancelled`, and `complete(spans)` / `decline()`. Publish the twelve
  existing SyntaxTheme roles as provider enum values: Keyword, Type, Function,
  Variable, BuiltIn, String, Number, Comment, Preprocessor, Operator, Constant, Error.
  Hosts own async execution; callbacks and request methods run on the QML thread.
- **R2 — Dispatch and fallback.** `syntaxHighlightingEnabled` remains the master
  switch. Offer eligible nonempty-language fenced source to the provider first,
  including languages unknown to the compiled backend. Source and language are exact
  parser values; spans use UTF-16 offsets into request.source. `complete([])` is
  successful plain source and suppresses fallback. `decline()`, invalid results and
  a five-second unanswered request fall back to the built-in backend; without that
  backend, source remains plain. Empty source submits no work; indented code remains plain.
- **R3 — Visual precedence.** Custom fenced visuals remain authoritative. Request
  highlighting when their source fallback is displayed, including existing selection/
  Find fallback. Retain native math/custom renderer behavior and plain source/copy
  projections, including fenced code in footnotes. `syntaxHighlightingAvailable`
  continues reporting only compiled built-in availability; providers work with it OFF.
- **R4 — Strict spans.** Accept arrays of `{start, length, role}` with finite integer
  numeric fields, positive lengths, in-bounds UTF-16 boundaries, known roles and no
  overlaps. Sort valid spans; reject an invalid result as a whole. Reject offsets
  splitting a surrogate pair. Adjacent spans are valid; duplicates overlap and are
  invalid. Apply existing limits before host dispatch and result acceptance:
  256 Ki UTF-16 source units, 16 Ki units per line and 65536 spans.
- **R5 — Lifecycle.** Cancel subscriptions on source/language replacement,
  disablement, provider replacement/destruction and delegate/view destruction.
  Discard duplicate, stale and cancelled completions. Shared providers may coalesce
  identical active work; cancelling one subscriber must not cancel another's work.
  The request becomes cancelled when all active subscriptions disappear, it is
  invalidated or its unanswered deadline expires. Host-retained request wrappers
  must permit checking cancelled before late async completion.
- **R6 — Cache and themes.** Isolate provider results by provider object identity,
  invalidation generation, exact source and language. Cache successful results,
  including empty success; do not cache failed/declined/timed-out host decisions.
  `provider.invalidate()` discards its cache, cancels old-generation work and
  refreshes active subscriptions. Theme changes recolor existing roles without new
  token requests. Provider destruction removes its cache and restores built-in/plain
  fallback for surviving views.
- **R7 — Delivery.** Include implementation, tests, playground provider example,
  documentation and verification in one PR. Cover completion/empty/decline/timeout,
  invalid spans, cancellation/stale/duplicate/reentrant completion, shared providers,
  invalidation/cache isolation, live themes, custom-renderer precedence, footnote
  bodies and backend ON/OFF. Verify older imports, installed shared/static consumers,
  source limits, lint/docs and exact Qt 6.8.0/6.12.0 packaging gates. Identify any
  unavailable native-input acceptance separately.

Qt >=6.8, C++17, private C++ implementation, native Qt Quick rendering and existing
host ownership remain. No public C++ provider ABI, arbitrary provider typography,
Mermaid, popup notes, streaming, merge or release publication is added.

Approval record: the user selected “Approve Feature 063” for requirements, design
and T1–T7 on 2026-10-20. Implementation and verification are authorized after
Feature 062's required CI checks pass. Merge, tagging and publication remain
excluded. No provider runtime implementation had started at approval time.
