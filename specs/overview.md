# Project overview

## Accepted project direction

`qt-markdown` is a standalone reusable Qt/QML Markdown rendering library, independent of any other project, with chat and file preview as initial use cases. The product is the library; an editable example viewer demonstrates integration. Hosts own visual identity, resource permissions, and navigation decisions. The library owns parsing semantics and rendering behavior.

Markdown source flows through a private parser into a document/AST model and then native Qt Quick block components. Keep parser-specific types out of the public QML interface. Library code must remain independent of host applications and their theme or component libraries. Document-level HTML, QTextDocument, and WebEngine rendering are outside the architecture.

The model should allow block-oriented rendering and later evolution toward streaming without committing to an incremental algorithm or stable block identity in feature 001. Full replacement and full parsing are sufficient for the initial slice.

Full CommonMark parsing conformance, initially pinned to CommonMark 0.31.2, is a v1.0 release gate. See [standards and conformance](standards.md) for parsing, native presentation, resource-policy boundaries, and selected GFM extensions. Feature 001 delivers the separately approved bounded static-text slice. Full conformance is not verified.

## Planned capabilities

- Resource policies: explicit control of local/remote images, relative resolution, file/external links, data URLs, SVG, and raw HTML. Define defaults and host responsibilities before adding resource-bearing rendering. Feature 001 loads no resources and activates no links.
- Streaming: shared semantics for static and appended input, with incremental parsing/rendering considered in its own feature.
- Fenced-block extensions: Feature 004 preserves decoded info privately and renders native code; future rendering can select ordinary code or an extension such as diagrams or math. Extension APIs and engines require later design and approval.

The brief's directory tree, C++ renderer interface, `MarkdownDocument.append`, `baseUrl`, `MarkdownPolicy`, style fields, and extension names are illustrative, not accepted public contracts. The approved public interface is described in [001 design](001-static-text/design.md) and its [003 inline-formatting extension](003-inline-formatting/design.md) and [004 fenced-code extension](004-fenced-code-blocks/design.md).

## Open implementation decisions

| Decision | Required resolution |
| --- | --- |
| Parser and dialect | Feature 001 implements a focused private parser without new dependencies, CommonMark-aligned paragraph/ATX structure, and explicit literal fallback. Feature 003, approved on 2026-10-07, now uses privately bundled cmark 0.31.2 for bounded inline parsing while retaining that block splitter. Neither slice claims full conformance. |
| Minimum Qt version | Feature 002 selects Qt ≥6.8; local scaffold verification uses Qt 6.11.2. Qt 6.8 execution remains unverified. |
| Packaging | Feature 002 provides `QMarkdown`, `QMarkdown::QMarkdown`, shared/static packages, and an import-only example. Feature 001 adds MarkdownView and MarkdownStyle with native rendering. |
| Licensing | MIT, attributed to qt-markdown contributors; see the repository LICENSE. Bundled cmark retains its complete applicable notices, installed with the package; see [provenance](../third_party/cmark/PROVENANCE.md). Check future dependencies separately. |

Feature 002 resolves scaffold packaging, licensing, and minimum Qt requirements. Qt Core, Gui, Quick, and Qml are scaffold dependencies; Qt Test is required only for tests. Feature 001 parser and rendering decisions are resolved and approved on 2026-10-07; its bounded slice is implemented. Feature 003 extends it with emphasis/strong/code, escapes/entities, an inline-code font overlay, and a private cached QTextLayout delegate; actual results are in [003 verification](003-inline-formatting/verification.md).

Feature 004, approved on 2026-10-07, adds bounded top-level native fenced code blocks and independent font/color styling in version 0.3.0. See [004 verification](004-fenced-code-blocks/verification.md) for actual checks and limitations.

Feature 006, explicitly approved on 2026-10-08, adds bounded top-level Setext headings, thematic breaks and indented code in package 0.4.0 / module 0.4. It preserves paragraph joining and unsupported-container fallback; nested lists/block quotes are deferred to Feature 007. Full conformance remains unverified. See [006 requirements](006-core-leaf-blocks/requirements.md), [design](006-core-leaf-blocks/design.md) and [verification](006-core-leaf-blocks/verification.md).
