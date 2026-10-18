# Requirements — Native fenced math

Approval: requirements, design and tasks approved by the user response “approved” on 2026-10-16. Dependency audit and verification remain implementation tasks; no success is implied. One cohesive 1.13.0 / QML 1.13 release PR is planned; merge, tagging and publication require separate authorization.

Release delivery approval — 2026-10-17: the user authorized the release plan for PR #36, including documentation completion, fresh final-head Qt 6.8.0/6.12.0 CI and retained full gate, squash merge through normal protections, `release.yml` dispatch with `release_pr=36`, tagging `v1.13.0`, stable/latest publication and downloaded hash verification of exactly four assets. Retain package 1.13.0 / QML 1.13 and existing APIs; no runtime changes. Stop on failed gates, changed inputs or differing published bytes. Historical approvals and results below remain unchanged; this approval supersedes earlier statements that delivery still requires authorization.

R055.1: Privately bundle a pinned MicroTeX snapshot, Qt painter backend, required fonts/resources and XML support, licenses and provenance. Use a repository-owned CMake wrapper, C++17 and existing Qt minimums; exclude upstream demos. QMARKDOWN_WITH_MATH defaults ON. OFF retains the public interface and source fallback. Installed, relocated and extracted-source builds require no downloads or source-tree paths.

R055.2: QML 1.13 adds MarkdownView.mathEnabled (false) and read-only mathAvailable (compiled availability). Exact first info token `math` selects native rendering only when enabled. Host fenced renderers retain precedence and their existing contract, including source fallback on host renderer failure.

R055.3: Test fractions, radicals, scripts, Greek, sums, integrals, scalable delimiters, matrices and aligned equations. Document the tested dialect, without claiming full LaTeX compatibility. Unsupported commands, malformed input, backend/resource failures and exceeded limits fall back for the whole formula.

R055.4: Preserve fenced code surfaces and source-copy headers. Center fitting formulas; scroll wide formulas horizontally at natural size using the host scrollbar contract. Anonymous grouped MarkdownStyle.math exposes color (inherited body foreground) and scale (1.0 times resolved body text size); glyphs use bundled fonts.

R055.5: Serialize engine work on one application-owned worker, register fonts on the GUI thread, return immutable painter recordings and discard stale completions. Bound source, parser/layout work and cached results. Exclude custom command definitions, dynamic evaluation and external resources/fonts; isolate formula state across blocks/views. Document uninterruptible backend operations.

R055.6: Selection enabled or active Find shows source. Search/plain-text copy use the TeX body without fences. Restore formulas when source mode ends. Missing backend and rendering failure use that same source representation.

R055.7: Deliver examples/toggles, API/dialect guidance, release documentation, license payloads and verification in one 1.13 release PR with feature 056. Mermaid, inline math, public highlighting providers and footnotes remain deferred.

## Development supersession — 2026-10-18

Feature [057](../057-atomic-math-selection/requirements.md) supersedes only the requirement to show source when selection is enabled: eligible native formulas remain rendered and select atomically. Find still shows source. Historical approval and verification records above are unchanged.
