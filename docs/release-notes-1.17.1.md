# QMarkdown 1.17.1

<!-- current-release:start -->
QMarkdown 1.17.1 / QML 1.17, released 2026-10-22, includes the hybrid renderer and
built-in highlighting initialization fix since 1.17.0.

Eligible text automatically uses native Qt scene-graph text nodes; padded code,
rendered formula/footnote atoms and rounded code backgrounds retain corrected raster
rendering. Independent Find, selection and focus overlays preserve layout and glyph
nodes. Built-in highlighting initializes its repository on the application thread
before transferring processing and destruction to its worker, fixing cross-thread
event-filter warnings. No public API or QML version change is required.

Logical interaction geometry remains independent of the single selected drawing
surface. Shaping, links, selection, Find, focus and extension contracts are retained.
The highlighting fix avoids warning suppression and worker-to-GUI blocking calls.

Package 1.17.1 accepts compatible 1.0–1.17 requests, including 1.17.0, and EXACT
1.17.1. EXACT 1.17.0 and future versions are rejected. QML 1.17, SOVERSION 1,
public APIs and dependencies remain unchanged. Linux amd64 requires Qt >=6.8,
C++17 and CMake >=3.21; configured verification targets are Qt 6.8.0 and 6.12.0.

Assets: [1.17.1 downloads](https://github.com/lebedenko/qmarkdown/releases/tag/v1.17.1),
`qmarkdown-1.17.1.tar.gz` and `qmarkdown-1.17.1-1-x86_64.pkg.tar.zst`, each with a
`.sha256` companion. See [installation](release-install.md),
[Arch installation](arch-install.md) and [source instructions](source-install.md).
<!-- current-release:end -->

Automated correctness and historical replay/lifetime measurements are recorded in
[Feature 064 qualification](../specs/064-hybrid-text-rendering/continuation-verification.md).
The historical replay qualifies both providers but establishes no speedup. Residual
raster cost remains. Perceived shimmer and owner visual acceptance are unassessed;
this release does not adopt the library in an external viewer.
Final release verification and publication results are recorded in the release PR
and its linked Actions runs.
