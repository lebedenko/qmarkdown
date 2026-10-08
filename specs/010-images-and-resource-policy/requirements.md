# 010: Requirements

**Approval:** On 2026-10-08 the user explicitly requested implementation of the supplied Feature 010 plan, approving these requirements, design and tasks for package 0.7.0 / module 0.7 before implementation.

R1: Deny all image access by default. Expose baseUrl and host-owned/shared MarkdownResourcePolicy with allowedFileRoots, allowedHttpsOrigins and allowQrc. Reset/destruction restores deny-all.
R2: Resolve only against explicit baseUrl. Canonical local directory boundaries, no remote file authorities, credentials or symlink escapes; HTTPS origin-only allowlists and independent redirect authorization. HTML stays literal, links report original destinations.
R3: Preserve ordered cmark image metadata, titles, UTF-16 description offsets, formatting and enclosing links; nested description images/links stay inert.
R4: Ready PNG/JPEG images occupy native rows within paragraphs/headings, with rebased text spans, recursive containers, block spacing (zero in tight lists), aspect ratio, no upscaling and oriented logical dimensions. Image taps cancel on drag.
R5: Per-view asynchronous bounded resources: 8 MiB encoded, 16 MP decoded, four fetches, one decoder, 64 MiB retained in document order; isolated network manager, manual redirects (five), normal TLS, total 15-second deadline. No retry. Generations cancel and release stale work.
R6: Version 0.7.0/0.7, Network package dependency, private tooling, opt-in bundled qrc playground, host documentation and shared/static relocated packaging.
Deferred: inline composition, SVG, data URLs, animation, HTML rendering, configurable limits and public status API. Acceptance does not imply full CommonMark conformance.
