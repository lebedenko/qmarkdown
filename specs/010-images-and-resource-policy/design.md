# 010: Design

**Approval:** On 2026-10-08 the user explicitly requested implementation of the supplied Feature 010 plan, approving these requirements, design and tasks for package 0.7.0 / module 0.7 before implementation.

Adapt the existing single cmark tree; append ImageSpan metadata without altering the fallback text/range/link projection. Keep source blocks immutable per generation. A private controller deduplicates resolved URLs and owns fetch/decode state. Authorize before opening resources and before every HTTPS redirect; reject invalid policy entries. Queue fetches and decode on a private single-worker pool, returning generation-tagged results to the GUI thread. Admit completed results in source order into a bounded image map.
Project ready resources into native image and clipped/rebased text blocks, grouped in a recursive sequence so inter-segment spacing follows enclosing paragraph/list semantics. Native painted image rows hold implicitly shared QImages and shrink without upscaling. Reuse DragThreshold handlers for enclosing links. Follow MarkdownStyle lifecycle for public policy ownership.
Use normal Qt TLS validation, no cookie load/save/auth reuse/disk cache, manual redirects and a total deadline timer. Check content signature, reader dimensions and oriented output dimensions; do not modify process-global allocation limits.
