# QMarkdown 1.17.0

<!-- current-release:start -->
QMarkdown 1.17.0 / QML 1.17, released 2026-10-20, includes the changes since 1.14.0.

Host-owned asynchronous syntax highlighting providers supply validated UTF-16
semantic role spans while the library retains palette, layout, copy and Find
ownership. Requests support cancellation, five-second deadlines, shared-provider
coalescing, bounded caches and invalidation. Declined, invalid or timed-out results
use the optional built-in backend or plain source. Providers also work without KDE.
See the [provider guide](syntax-highlighting.md#host-providers-qml-117).

Independent opt-in inline math renders native baseline atoms with complete-formula
wrapping and literal TeX copy/Find projections. Block and inline math share bounded
requests and preserve source fallback. See the [math guide](math.md).

Independent opt-in Markdown footnotes support forward/repeated references, rich
native endnotes, superscript numbers and per-occurrence backlinks. Hosts own
fragment navigation; numbered copy/Find projections and keyboard/accessibility
behavior remain supported. See the [footnote guide](footnotes.md).

The playground demonstrates inline math, footnotes and asynchronous highlighting.
Existing 1.x consumers remain compatible. Package 1.17.0 accepts compatible
1.0–1.17 requests and EXACT 1.17.0; exact earlier versions require historical
packages. Import QMarkdown 1.17 for the provider type, 1.16 for footnotes and 1.15
for inline math. New features remain opt-in. Linux amd64 requires Qt >=6.8, C++17
and CMake >=3.21; configured verification targets are Qt 6.8.0 and 6.12.0.

Assets: [1.17.0 downloads](https://github.com/lebedenko/qmarkdown/releases/tag/v1.17.0),
`qmarkdown-1.17.0.tar.gz` and `qmarkdown-1.17.0-1-x86_64.pkg.tar.zst`, each with a
`.sha256` companion. See [installation](release-install.md),
[Arch installation](arch-install.md) and [source instructions](source-install.md).
<!-- current-release:end -->

Actual implementation checks are recorded in features
[061](../specs/061-inline-math/verification.md),
[062](../specs/062-footnotes/verification.md) and
[063](../specs/063-highlighting-providers/verification.md).
Final release verification and publication results are recorded in the release PR
and its linked Actions runs.
