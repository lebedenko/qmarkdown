# QMarkdown 1.18.0

<!-- current-release:start -->
QMarkdown 1.18.0 / QML 1.18, released 2026-10-27, includes streaming Markdown,
retained padded-paragraph rendering and CI improvements since 1.17.1.

`MarkdownView.streamingEnabled` defaults to false. `appendMarkdown(chunk: string)`
and strict suffix-growing assignments notify accepted source immediately. A
non-restarting 16 ms timer batches publication; `flushMarkdown()` synchronously
publishes buffered source. Disabling streaming flushes; replacements publish
immediately, and settings changes consume buffered source under the new settings.
Each published snapshot uses a complete static parse, including incomplete syntax
and late definitions. See the [streaming contract](streaming.md).

Unchanged recursive content, delegates and resources survive publication. Selection
survives unchanged semantic ranges; surviving focused links and Find occurrences
retain reading state. Resource settings and replacement retain cancellation
semantics. Hosts own scrolling and source bindings: append to the host source when
using a binding, or let appendMarkdown own the view source.

Padded paragraphs retain raster content during scrolling. Measured reductions in
repeated delegate creation and resource work are bounded observations, not claims
of incremental parsing, guaranteed throughput, frame rate or visual acceptance.
CI selects Light/Core/Full profiles from PR inputs, retains the Full release gate,
and adds committed-snapshot local hooks and compiler caching.

Earlier 1.x imports remain supported. Package 1.18.0 accepts compatible 1.0–1.18
CMake requests and EXACT 1.18.0; exact earlier packages and future versions are
rejected. SOVERSION 1 and dependencies remain unchanged. Linux amd64 requires Qt
>=6.8, C++17 and CMake >=3.21; configured targets are Qt 6.8.0 and 6.12.0.

Assets: [1.18.0 downloads](https://github.com/lebedenko/qmarkdown/releases/tag/v1.18.0),
`qmarkdown-1.18.0.tar.gz` and `qmarkdown-1.18.0-1-x86_64.pkg.tar.zst`, each with a
`.sha256` companion. See [installation](release-install.md),
[Arch installation](arch-install.md) and [source instructions](source-install.md).
<!-- current-release:end -->

[Feature 069 verification](../specs/069-streaming-markdown/verification.md) records
local correctness and bounded measurements, including method and limitations.
Final hosted verification and publication evidence belong in the release PR and
linked Actions runs. Owner visual acceptance remains unassessed.
