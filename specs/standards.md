# Standards and conformance

## Current 1.0 candidate direction

[Iteration 021](021-v1-release-candidate/requirements.md) is approved for Linux amd64 candidate 1.0.0 / QML 1.0, Qt >=6.8, C++17 and CMake >=3.21. Pinned Qt 6.8.0/6.11.3, host Qt and native Wayland execution are separate evidence; see [actual verification](021-v1-release-candidate/verification.md) for gate status. Features 017/019/020 establish complete official-corpus semantics, representative native presentation and responsive decoder lifecycle. This candidate corrects long fences without upgrading cmark or changing the official corpus.

Documented public QML properties, signals, behavior and CMake integration remain compatible throughout 1.x. QMarkdown.Private/native classes stay private; no installed C++ headers, public C++ ABI or portable binary guarantee. Consumers use compatible Qt/toolchains and migrate 0.7 imports/package requests to 1.0; unversioned imports work and legacy aliases are absent.

Accepted limits: separate PNG/JPEG image rows, literal HTML, full replacement, narrow-glyph overhang and uninterruptible codecs. Two occupied decoder workers can delay fresh work; application shutdown may wait for codecs. Optional syntax, streaming, selection/copy, renderer extensions, performance optimization and broader platforms are deferred. Local candidate readiness requires every iteration 021 gate; Feature 022 adds release packaging and hosted publication gates; see [release packaging](022-release-packaging/requirements.md). Historical iteration descriptions below retain their original versions and evidence.


## Accepted direction and current status

Full CommonMark parsing conformance is a v1.0 release gate, initially pinned to [CommonMark 0.31.2](https://spec.commonmark.org/0.31.2/). This is project direction, not authorization to implement a parser. Feature 001 was approved on 2026-10-07 and implements a bounded paragraph/ATX slice with literal fallback; feature 002 provides the approved module scaffold. Feature 003 adds authored fixtures and privately bundled cmark 0.31.2 for bounded emphasis/strong/code and escapes/entities, with source-preserved resource syntax. Its [verification](003-inline-formatting/verification.md) remains separate from the future official-suite gate. Feature 004 adds bounded top-level fenced code recognition and native rendering; see [004 verification](004-fenced-code-blocks/verification.md). These are historical bounded slices; current pinned-corpus semantics are complete as recorded below. A v1.0 release is not declared.

This repository-owned summary preserves the initial brief's standards direction without requiring `/tmp/qt-markdown.md`: an independent Qt/QML library parses Markdown into a private document/AST model, renders native Qt Quick blocks, and leaves visual identity and resource permissions to hosts. Streaming and richer renderers are subsequent capabilities. The brief's profile, policy, document, and extension APIs are illustrative and introduce no public interface commitments.

## Three separate contracts

- **Parsing semantics:** recognize all pinned CommonMark syntax and precedence, including raw HTML blocks and inline raw HTML. Recognition does not require executing or rendering HTML.
- **Native presentation:** map document semantics to Qt Quick components and verify layout and interaction separately. Official HTML examples encode semantic expectations; they do not require production HTML, QTextDocument, or WebEngine rendering.
- **Host resource policies:** specify resource access, navigation, and raw HTML presentation behavior before resource-bearing rendering is implemented. Semantic recognition alone grants no resource permission. Feature 001 continues to load no resources and activate no links.

## Selected extensions and richer capabilities

[GitHub Flavored Markdown 0.29-gfm](https://github.github.com/gfm/) defines extensions on an older CommonMark baseline (0.29), including tables, task lists, strikethrough, extended autolinks, and raw HTML filtering. Plan the first four as selected GFM syntax extensions, separately from CommonMark 0.31.2 core work. Resolve interactions with the newer core baseline in later approved specifications. Supporting those selected extensions must not be described as full GFM conformance; GFM raw HTML filtering also needs a deliberate policy decision.

Syntax highlighting, Mermaid, math, footnotes, and other richer capabilities are separate rendering or syntax features, not CommonMark core or the selected GFM set. Their dependencies, policies, and APIs require later design and approval. Streaming and optional extensions are not v1.0 release gates.

[RFC 7763](https://www.rfc-editor.org/info/rfc7763/) registers the `text/markdown` media type; it does not define Markdown parsing syntax.

## Future conformance verification

Later approved specifications must trace core requirements to implementation tasks and semantic checks. Use all official examples for the pinned CommonMark version, preserving version, example identifiers, upstream source attribution, and a reproducible fixture origin/checksum. Review fixture redistribution and test-tool licensing before importing them; the CommonMark specification identifies its CC BY-SA 4.0 license. Do not assume the repository's MIT license covers upstream fixtures.

A test-only serialization adapter may compare the private document model with official HTML expectations without introducing production HTML rendering. Document normalization and adapter behavior so it cannot hide semantic failures; cover model distinctions and raw HTML recognition independently where needed. Keep selected GFM checks separate so extensions do not alter core conformance expectations.

The future verification record must include exact acquisition/build/run commands, pinned inputs and tool versions, total/pass/fail/skipped counts, failures by example identifier, and limitations. Feature 011 now provides an offline runner, `python3 scripts/verify-commonmark.py`, with separate bundled-parser and native-model results; see its [verification record](011-commonmark-baseline/verification.md). The v1.0 gate requires every pinned official example to pass semantic checks with no failures or skips, native renderer verification, and documented resource/HTML behavior. Native checks must cover supported semantic blocks/inlines, wrapping, layout, and host policy behavior; parser results alone do not verify presentation.

Current pinned-corpus semantic evidence: **complete**. [Feature 017](017-complete-semantic-evidence/verification.md) records 652/652 parser and model passes, zero mismatches/uncheckable examples and zero unresolved aggregate limits/losses; strict mode exits 0. Complete source-reviewed expectations cover 459 examples, with 193 HTML-only examples. All 390 formerly limited cases retain required HTML checks alongside complete source checks. Individual HTML limitations remain visible in report schema 4, resolved only by passing complete source evidence. CodeBlock recognition/content/full info are retained; fenced versus indented syntax origin is outside that contract. Raw production models remain identical to iteration 016; package/module remain 0.7.0/0.7.

This result completes semantic evidence for the pinned corpus, not the separate native presentation and release requirements. Local v1.0 candidate gates are recorded in iteration 021; a published release is not declared. Existing native renderer/resource regressions remain separate evidence; GFM, streaming, selection/copy, highlighting and performance work remain deferred. See the [conformance inventory](../tests/commonmark/README.md) and [roadmap](roadmap.md).

Feature 006, explicitly approved on 2026-10-08, adds bounded top-level Setext headings, thematic breaks and indented code in package 0.4.0 / module 0.4. It preserves paragraph joining and unsupported-container fallback; nested lists/block quotes are deferred to Feature 007. Full conformance remains unverified. See [006 requirements](006-core-leaf-blocks/requirements.md), [design](006-core-leaf-blocks/design.md) and [verification](006-core-leaf-blocks/verification.md).

Feature 019, approved on 2026-10-09 and implemented/verified on host shared/static and both pinned Qt CI versions, completes the representative native presentation assessment separately from complete corpus semantics. See the [assertion-level coverage matrix](019-native-presentation-evidence/coverage.md) and [execution record](019-native-presentation-evidence/verification.md). This tests/documentation scope preserves 0.7.0/0.7 behavior. Iteration 021 approves Linux amd64 support, accepts documented presentation/resource/lifecycle limits and records local candidate/versioning gates separately; GPU, accessibility and other-platform evidence are not established.
