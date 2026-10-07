# Staged roadmap

These stages preserve the brief's direction. Version labels describe intended scope, not shipped releases or schedules. Each feature needs its own requirements, design, tasks, and approval. The [standards reference](standards.md) defines the pinned target and future conformance evidence.

| Stage | Intended scope |
| --- | --- |
| Buildable scaffold | [002 project scaffold](002-project-scaffold/requirements.md): import-only Qt/QML module, shared/static packaging, and smoke checks; scaffold foundation completed. |
| First vertical slice | [001 static text](001-static-text/requirements.md): paragraphs, H1–H6, native block layout, styling, replacement updates. Approved on 2026-10-07 and implemented; see its verification record. |
| CommonMark core coverage | Complete CommonMark 0.31.2 semantics: ATX and Setext headings, paragraphs, emphasis/strong emphasis, inline code, fenced and indented code, inline and reference links, images, ordered/unordered and nested lists, block quotes, thematic breaks, escapes/entities, autolinks, hard/soft breaks, and raw HTML recognition. Include whitespace, line handling, delimiter rules, and precedence. Specify resource and activation policies before links/images. |
| Selected GFM syntax | Tables, task lists, strikethrough, and extended autolinks, with separate extension checks and newer-core compatibility decisions; no full GFM conformance claim. |
| Interaction improvements | Selection/copy improvements and code-block actions, with host navigation and resource behavior specified. |
| Streaming | Static/appended semantic consistency, incremental document updates and rendering; specify stability and performance checks. |
| Renderer extensions | Syntax highlighting, extension renderer API, Mermaid, math, and custom fenced-block renderers, subject to dependency and policy decisions. Footnotes and other richer syntax require separate specifications. |
| v1.0 conformance gate | All pinned CommonMark 0.31.2 official examples pass semantic checks, with native renderer verification and documented resource/HTML behavior. Record reproducible commands and actual counts. Streaming and optional extensions are not gates. |

The capability stages separate work streams rather than impose delivery dates or require optional work before v1.0. The static-text slice is implemented; full conformance remains **not verified**.

First-slice fallback rendering is temporary behavior for unsupported syntax. The approved feature 001 design aligns supported paragraph/ATX structure with CommonMark 0.31.2 and specifies literal inline and protected-fence fallback, concrete styling, sizing, and updates. Feature 003 supersedes the paragraph/heading literal-inline behavior with bounded emphasis/strong/code and escapes/entities while retaining the block splitter and protected fences. Feature 004 supersedes protected fences with bounded native code rendering and independent font/color. Full block precedence, remaining inline semantics, and fenced-block extensions require later approved revisions. The standards target does not authorize implementation. Feature 002 approval and scaffold scope remain unchanged.
