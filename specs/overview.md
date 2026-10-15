# Project overview

## Current release

<!-- current-release:start -->
Package **1.10.0 / QML 1.10**. Release assets: [1.10.0 downloads](https://github.com/lebedenko/qmarkdown/releases/tag/v1.10.0).
It includes all five independent opt-in GFM extensions on CommonMark 0.31.2,
native tables, inline-code padding, opt-in document-wide desktop selection and
host-owned plain-text copy requests, custom fenced-block renderers, repeated-click
selection and host-owned selection edge autoscrolling. Exact GFM 0.29-gfm conformance is not claimed.
See the [release notes](../docs/release-notes-1.10.0.md) and
[installation instructions](../docs/release-install.md).

Linux amd64 requires Qt >=6.8, C++17 and CMake >=3.21. Configured verification
targets are Qt 6.8.0 and 6.12.0. Public QML behavior and CMake integration remain
compatible throughout 1.x. Package 1.10.0 accepts compatible 1.0–1.10 requests and
EXACT 1.10.0; exact earlier versions require historical packages. Native classes
and QMarkdown.Private remain private; no public C++ ABI is promised.
Hosts own visual identity, scrolling, navigation, clipboard and resource policy.

Reading tools include literal Find with independent highlights, a recursive heading
outline and explicit fragment navigation. Hosts own controls and scrolling. See
the [reader guide](../docs/reading-navigation.md).

QML 1.10 adds opt-in keyboard link traversal, visible focus and one accessible
Link per logical link, including text/image splits. Tab exits through the host
focus chain; Enter activates through host-owned link handling. Replaceable task
indicators include a rounded fallback; the playground demonstrates the active
Controls style. Keyboard traversal defaults off; accessible links remain available
without keyboard opt-in. Task indicators remain read-only. Private QML
responsibilities are refactored without a new public C++ surface.
<!-- current-release:end -->

## Historical 1.6.0 status

Package **1.6.0 / QML 1.6** is [released](https://github.com/lebedenko/qmarkdown/releases/tag/v1.6.0) on **2026-10-13 UTC**. Optional code-block actions retain host-owned clipboard handling. Styling includes opt-in Polished/Minimal light/dark presets; Legacy remains the default. Inline-code padding defaults to 4/2 logical pixels in every preset; zero metrics restore previous spacing. Hosts own visual identity, scrolling, navigation and resource authorization.

Linux amd64 support requires Qt >=6.8, C++17 and CMake >=3.21. Configured verification targets are minimum Qt 6.8.0 and development Qt 6.12.0, established by [iteration 024](024-development-host-qt/verification.md). Earlier Qt 6.11.2/6.11.3 and native Wayland runs remain historical evidence, not the current configured matrix.

The evidence contracts remain separate: [iteration 017](017-complete-semantic-evidence/verification.md) establishes all 652 pinned CommonMark parser/model semantic passes; [iteration 019](019-native-presentation-evidence/coverage.md) maps representative native presentation assertions, extended by [iteration 025](025-styling-presets/verification.md); iteration 033 records exact-release packaging, publication and downloaded-asset lifecycle checks. The Ubuntu 24.04 x86_64 archive built with Qt 6.8.0 passed on exact Qt 6.8.0 and 6.12.0 runtimes; the published Arch package also passed independent ownership, consumers, upgrade and removal checks. This is bounded binary evidence, not a portable binary guarantee.

Documented public QML behavior and CMake integration remain compatible throughout 1.x. QMarkdown.Private/native classes remain private; no C++ headers are installed and no public C++ ABI is promised. Package 1.6.0 accepts compatible 1.0–1.6 requests and EXACT 1.6.0; exact earlier versions require historical packages.

Accepted limits remain separate PNG/JPEG image rows, literal HTML, full document replacement, narrow-glyph overhang and uninterruptible codecs. Two occupied decoder workers can delay fresh work; application shutdown may wait for codecs. Touch selection handles, edge autoscrolling, rich-text copying, streaming, highlighting, bundled renderer engines, inline-code borders, arbitrary style inheritance, further performance optimization and broader platforms remain deferred. Historical descriptions below retain their original versions and verification scope.


## Accepted project direction

`qt-markdown` is a standalone reusable Qt/QML Markdown rendering library, independent of any other project, with chat and file preview as initial use cases. The product is the library; an editable example viewer demonstrates integration. Hosts own visual identity, resource permissions, and navigation decisions. The library owns parsing semantics and rendering behavior.

Markdown source flows through a private parser into a document/AST model and then native Qt Quick block components. Keep parser-specific types out of the public QML interface. Library code must remain independent of host applications and their theme or component libraries. Document-level HTML, QTextDocument, and WebEngine rendering are outside the architecture.

The model should allow block-oriented rendering and later evolution toward streaming without committing to an incremental algorithm or stable block identity in feature 001. Full replacement and full parsing are sufficient for the initial slice.

Full CommonMark parsing conformance, initially pinned to CommonMark 0.31.2, is a v1.0 release gate. See [standards and conformance](standards.md) for parsing, native presentation, resource-policy boundaries, and selected GFM extensions. Feature 001 delivers the separately approved bounded static-text slice. Official-corpus semantics are verified by Feature 017; historical candidate evidence is tracked by iteration 021, with completed publication in iteration 027.

## Remaining planned capabilities

- Resource policies: Feature 010 implements opt-in PNG/JPEG access and Feature 009 reports link destinations to hosts. SVG, additional formats and richer raw HTML behavior require later policy/design approval. Feature 001’s no-resource/no-link slice remains historical.
- Streaming: shared semantics for static and appended input, with incremental parsing/rendering considered in its own feature.
- Bundled renderer engines: highlighting, Mermaid and math remain deferred.

[Feature 042](042-custom-fenced-block-renderers/requirements.md), approved on 2026-10-14,
implements host-provided fenced-block QML Components delivered in 1.8 and
retained in 1.10. Historical 1.7.0 downloads exclude these renderers.
Selection enabled displays source; copying retains source. See the
[renderer guide](../docs/fenced-block-renderers.md) and separate
[verification/delivery record](042-custom-fenced-block-renderers/verification.md).

The brief's directory tree, C++ renderer interface, `MarkdownDocument.append`, `baseUrl`, `MarkdownPolicy`, style fields, and extension names are illustrative, not accepted public contracts. The approved public interface is described in [001 design](001-static-text/design.md) and its [003 inline-formatting extension](003-inline-formatting/design.md) and [004 fenced-code extension](004-fenced-code-blocks/design.md).

## Resolved foundations and historical decisions

| Decision | Required resolution |
| --- | --- |
| Parser and dialect | Feature 001 implements a focused private parser without new dependencies, CommonMark-aligned paragraph/ATX structure, and explicit literal fallback. Feature 003 introduced privately bundled cmark 0.31.2 for bounded inlines; Feature 007 now uses it for the entire document and recursive native containers. Official-corpus semantic evidence is complete; iteration 021 supplies the long-fence correction and release checks. |
| Minimum Qt version | Feature 002 selects Qt ≥6.8; historical scaffold verification used Qt 6.11.2. Earlier local CI verified Qt 6.8.0/6.11.3; the current configured checks use Qt 6.8.0/6.12.0 (iteration 024). |
| Packaging | Feature 002 provides `QMarkdown`, `QMarkdown::QMarkdown`, shared/static packages, and an import-only example. Feature 001 adds MarkdownView and MarkdownStyle with native rendering. |
| Licensing | MIT, attributed to qt-markdown contributors; see the repository LICENSE. Bundled cmark retains its complete applicable notices, installed with the package; see [provenance](../third_party/cmark/PROVENANCE.md). Check future dependencies separately. |

Feature 002 resolves scaffold packaging, licensing, and minimum Qt requirements. Qt Core, Gui, Quick, and Qml are scaffold dependencies; Qt Test is required only for tests. Feature 001 parser and rendering decisions are resolved and approved on 2026-10-07; its bounded slice is implemented. Feature 003 extends it with emphasis/strong/code, escapes/entities, an inline-code font overlay, and a private cached QTextLayout delegate; actual results are in [003 verification](003-inline-formatting/verification.md).

Feature 004, approved on 2026-10-07, adds bounded top-level native fenced code blocks and independent font/color styling in version 0.3.0. See [004 verification](004-fenced-code-blocks/verification.md) for actual checks and limitations.

Feature 006, explicitly approved on 2026-10-08, adds bounded top-level Setext headings, thematic breaks and indented code in package 0.4.0 / module 0.4. It preserves paragraph joining and unsupported-container fallback; nested lists/block quotes are deferred to Feature 007. Full conformance remains unverified. See [006 requirements](006-core-leaf-blocks/requirements.md), [design](006-core-leaf-blocks/design.md) and [verification](006-core-leaf-blocks/verification.md).

Feature 007, approved for implementation on 2026-10-08, supersedes unsupported-container/resource-source fallback with native recursive lists/quotes and inert formatted labels in package 0.5.0 / module 0.5. Production parsing now adapts one cmark tree. Hard breaks and reference definitions follow cmark; HTML stays literal. Full conformance and resource rendering remain deferred. See [requirements](007-container-blocks/requirements.md), [design](007-container-blocks/design.md), [tasks](007-container-blocks/tasks.md) and [verification](007-container-blocks/verification.md).
