# QMarkdown 1.13.0

<!-- current-release:start -->
QMarkdown 1.13.0 / QML 1.13 adds opt-in native block math with a bundled private
MicroTeX renderer. `mathEnabled` defaults off per view; `mathAvailable` reports
compiled availability. Exact `math` fences and standalone `$$` delimiter lines
share natural-size mathematical rendering and horizontal scrolling. Host fenced
renderers retain precedence. Selection and Find expose searchable/copyable TeX
body source; failures and disabled builds use source fallback.

`MarkdownStyle.math` adds inherited foreground color and body-relative scale.
Mathematical fonts and XML support are bundled with their license/provenance
payloads. `QMARKDOWN_WITH_MATH` defaults ON and can be disabled without removing
the QML properties. The [math guide](math.md) describes syntax, tested dialect,
resource restrictions, work budgets and remaining uninterruptible operations.

Existing 1.x imports and public behavior remain compatible. Linux amd64 minimums
remain Qt 6.8, C++17 and CMake 3.21. Qt 6.8.0 and 6.12.0 are the configured
verification targets. Inline math, Mermaid, highlighting providers and footnotes
remain deferred. Actual results belong to the feature
[055](../specs/055-native-fenced-math/verification.md) and
[056](../specs/056-standalone-display-math/verification.md) verification records.

Assets: [1.13.0 downloads](https://github.com/lebedenko/qmarkdown/releases/tag/v1.13.0),
`qmarkdown-1.13.0.tar.gz` and `qmarkdown-1.13.0-1-x86_64.pkg.tar.zst`, each with a
`.sha256` companion. See [installation](release-install.md), [Arch installation](arch-install.md)
and [source build instructions](source-install.md). <!-- current-release:end -->

Merge, tagging and publication
remain separately authorized delivery actions; these notes do not record them.
