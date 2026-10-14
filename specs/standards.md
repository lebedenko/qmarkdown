# Standards and conformance

Current local package: **1.7.0 / QML 1.7**. All five independent opt-in GFM extensions
(task lists, strikethrough, extended autolinks, tables and tag filtering) are implemented
on CommonMark 0.31.2; exact GFM 0.29-gfm conformance is not claimed.
[Feature 040](040-document-selection/verification.md) implements opt-in document-wide
desktop selection and host-owned plain-text copy requests.
[Feature 041](041-library-maintainability/verification.md) is implemented and locally
verified. Protected delivery of those features is recorded by merged
[PR 19](https://github.com/lebedenko/qmarkdown/pull/19) and
[PR 20](https://github.com/lebedenko/qmarkdown/pull/20); their verification documents
retain the original local-stage records. Publication requires separate authorization. Published package **1.6.0 / QML 1.6** and its historical evidence remain
in [Feature 038](038-gfm-completion-inline-code-padding/verification.md).

## Current published status

Package **1.6.0 / QML 1.6** is [released](https://github.com/lebedenko/qmarkdown/releases/tag/v1.6.0) on **2026-10-13 UTC**. Optional code-block actions retain host-owned clipboard handling. Styling includes opt-in Polished/Minimal light/dark presets; Legacy remains the default. Inline-code padding defaults to 4/2 logical pixels in every preset; zero metrics restore previous spacing. Hosts own visual identity, scrolling, navigation and resource authorization.

Linux amd64 support requires Qt >=6.8, C++17 and CMake >=3.21. Configured verification targets are minimum Qt 6.8.0 and development Qt 6.12.0, established by [iteration 024](024-development-host-qt/verification.md). Earlier Qt 6.11.2/6.11.3 and native Wayland runs remain historical evidence, not the current configured matrix.

The evidence contracts remain separate: [iteration 017](017-complete-semantic-evidence/verification.md) establishes all 652 pinned CommonMark parser/model semantic passes; [iteration 019](019-native-presentation-evidence/coverage.md) maps representative native presentation assertions, extended by [iteration 025](025-styling-presets/verification.md); iteration 033 records exact-release packaging, publication and downloaded-asset lifecycle checks. The Ubuntu 24.04 x86_64 archive built with Qt 6.8.0 passed on exact Qt 6.8.0 and 6.12.0 runtimes; the published Arch package also passed independent ownership, consumers, upgrade and removal checks. This is bounded binary evidence, not a portable binary guarantee.

Documented public QML behavior and CMake integration remain compatible throughout 1.x. QMarkdown.Private/native classes remain private; no C++ headers are installed and no public C++ ABI is promised. Package 1.6.0 accepts compatible 1.0–1.6 requests and EXACT 1.6.0; exact earlier versions require historical packages.

Accepted limits remain separate PNG/JPEG image rows, literal HTML, full document replacement, narrow-glyph overhang and uninterruptible codecs. Two occupied decoder workers can delay fresh work; application shutdown may wait for codecs. Touch selection handles, edge autoscrolling, rich-text copying, streaming, highlighting, bundled renderer engines, inline-code borders, arbitrary style inheritance, further performance optimization and broader platforms remain deferred. Historical descriptions below retain their original versions and verification scope.


## Selected extensions in 1.6.0

Released **1.6.0 / QML 1.6** supports all five independent default-off GFM extensions and an aggregate switch on CommonMark 0.31.2 core. Native tables, filtered raw-HTML semantics and layout-aware inline-code padding are implemented. [Iteration 038](038-gfm-completion-inline-code-padding/verification.md) preserves historical verification and approvals. Exact GFM 0.29-gfm conformance is not claimed.

## Historical direction and bounded slices

Full CommonMark parsing conformance is a v1.0 release gate, initially pinned to [CommonMark 0.31.2](https://spec.commonmark.org/0.31.2/). This is project direction, not authorization to implement a parser. Feature 001 was approved on 2026-10-07 and implements a bounded paragraph/ATX slice with literal fallback; feature 002 provides the approved module scaffold. Feature 003 adds authored fixtures and privately bundled cmark 0.31.2 for bounded emphasis/strong/code and escapes/entities, with source-preserved resource syntax. Its [verification](003-inline-formatting/verification.md) remains separate from the future official-suite gate. Feature 004 adds bounded top-level fenced code recognition and native rendering; see [004 verification](004-fenced-code-blocks/verification.md). These are historical bounded slices; current pinned-corpus semantics are complete as recorded below. Version 1.0.0 was subsequently published in iteration 023; 1.2.0 publication is recorded in iteration 033; current published 1.6.0 evidence is retained in iteration 038.

This repository-owned summary preserves the initial brief's standards direction without requiring `/tmp/qt-markdown.md`: an independent Qt/QML library parses Markdown into a private document/AST model, renders native Qt Quick blocks, and leaves visual identity and resource permissions to hosts. Streaming and richer renderers are subsequent capabilities. The brief's profile, policy, document, and extension APIs are illustrative and introduce no public interface commitments.

## Three separate contracts

- **Parsing semantics:** recognize all pinned CommonMark syntax and precedence, including raw HTML blocks and inline raw HTML. Recognition does not require executing or rendering HTML.
- **Native presentation:** map document semantics to Qt Quick components and verify layout and interaction separately. Official HTML examples encode semantic expectations; they do not require production HTML, QTextDocument, or WebEngine rendering.
- **Host resource policies:** specify resource access, navigation, and raw HTML presentation behavior before resource-bearing rendering is implemented. Semantic recognition alone grants no resource permission. Feature 001 loaded no resources and activated no links; current host-controlled links and opt-in images are specified in Features 009/010.

## Selected extensions and richer capabilities

[GitHub Flavored Markdown 0.29-gfm](https://github.github.com/gfm/) defines extensions on an older CommonMark baseline (0.29), including tables, task lists, strikethrough, extended autolinks, and raw HTML filtering. All five extensions have independent default-off switches and an aggregate switch, implemented on CommonMark 0.31.2. [Feature 038](038-gfm-completion-inline-code-padding/verification.md) records tables and tag filtering alongside prior slices. Tag filtering retains private filtered semantics while native HTML remains literal. Separate extension checks cover interactions with the newer core; full GFM conformance is not claimed.

Syntax highlighting, Mermaid, math, footnotes, and other richer capabilities are separate rendering or syntax features, not CommonMark core or the selected GFM set. [Feature 042](042-custom-fenced-block-renderers/requirements.md), approved on 2026-10-14, implements custom fenced-block QML Components without bundled engines; [verification and delivery](042-custom-fenced-block-renderers/verification.md) remain separately recorded. Other dependencies, policies, and APIs require later design and approval. Streaming and optional extensions are not v1.0 release gates.

[RFC 7763](https://www.rfc-editor.org/info/rfc7763/) registers the `text/markdown` media type; it does not define Markdown parsing syntax.

## Conformance verification and future extensions

Later approved specifications must trace core requirements to implementation tasks and semantic checks. Use all official examples for the pinned CommonMark version, preserving version, example identifiers, upstream source attribution, and a reproducible fixture origin/checksum. Review fixture redistribution and test-tool licensing before importing them; the CommonMark specification identifies its CC BY-SA 4.0 license. Do not assume the repository's MIT license covers upstream fixtures.

A test-only serialization adapter may compare the private document model with official HTML expectations without introducing production HTML rendering. Document normalization and adapter behavior so it cannot hide semantic failures; cover model distinctions and raw HTML recognition independently where needed. Keep selected GFM checks separate so extensions do not alter core conformance expectations.

The future verification record must include exact acquisition/build/run commands, pinned inputs and tool versions, total/pass/fail/skipped counts, failures by example identifier, and limitations. Feature 011 now provides an offline runner, `python3 scripts/verify-commonmark.py`, with separate bundled-parser and native-model results; see its [verification record](011-commonmark-baseline/verification.md). The v1.0 gate requires every pinned official example to pass semantic checks with no failures or skips, native renderer verification, and documented resource/HTML behavior. Native checks must cover supported semantic blocks/inlines, wrapping, layout, and host policy behavior; parser results alone do not verify presentation.

Current pinned-corpus semantic evidence: **complete**. [Feature 017](017-complete-semantic-evidence/verification.md) records 652/652 parser and model passes, zero mismatches/uncheckable examples and zero unresolved aggregate limits/losses; strict mode exits 0. Complete source-reviewed expectations cover 459 examples, with 193 HTML-only examples. All 390 formerly limited cases retain required HTML checks alongside complete source checks. Individual HTML limitations remain visible in report schema 4, resolved only by passing complete source evidence. CodeBlock recognition/content/full info are retained; fenced versus indented syntax origin is outside that contract. At iteration 017, raw production models remained identical to iteration 016 and package/module remained 0.7.0/0.7; current published versions are 1.6.0/1.6 and local versions are 1.7.0/1.7.

This result completes semantic evidence for the pinned corpus, not the separate native presentation and release requirements. Historical local v1.0 candidate gates are recorded in iteration 021; completed release/publication gates are recorded separately in iterations 023, 027 and 033. Existing native renderer/resource regressions remain separate evidence. Selected GFM syntax and desktop selection are implemented. Streaming, highlighting, bundled renderer engines and further performance work remain deferred. See the [conformance inventory](../tests/commonmark/README.md) and [roadmap](roadmap.md).

Feature 006, explicitly approved on 2026-10-08, adds bounded top-level Setext headings, thematic breaks and indented code in package 0.4.0 / module 0.4. It preserves paragraph joining and unsupported-container fallback; nested lists/block quotes are deferred to Feature 007. Full conformance remains unverified. See [006 requirements](006-core-leaf-blocks/requirements.md), [design](006-core-leaf-blocks/design.md) and [verification](006-core-leaf-blocks/verification.md).

Feature 019, approved on 2026-10-09 and implemented/verified on host shared/static and both pinned Qt CI versions, completes the representative native presentation assessment separately from complete corpus semantics. See the [assertion-level coverage matrix](019-native-presentation-evidence/coverage.md) and [execution record](019-native-presentation-evidence/verification.md). This tests/documentation scope preserves 0.7.0/0.7 behavior. Iteration 021 approves Linux amd64 support, accepts documented presentation/resource/lifecycle limits and records local candidate/versioning gates separately; GPU, accessibility and other-platform evidence are not established.
