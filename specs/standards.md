# Standards and conformance

## Accepted direction and current status

Full CommonMark parsing conformance is a v1.0 release gate, initially pinned to [CommonMark 0.31.2](https://spec.commonmark.org/0.31.2/). This is project direction, not authorization to implement a parser. Feature 001 was approved on 2026-10-07 and implements a bounded paragraph/ATX slice with literal fallback; feature 002 provides the approved module scaffold. Feature 003 adds authored fixtures and privately bundled cmark 0.31.2 for bounded emphasis/strong/code and escapes/entities, with source-preserved resource syntax. Its [verification](003-inline-formatting/verification.md) remains separate from the future official-suite gate. Feature 004 adds bounded top-level fenced code recognition and native rendering; see [004 verification](004-fenced-code-blocks/verification.md). Full CommonMark conformance is **not verified**.

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

Current full-conformance results: **not verified**. Feature 012 records 652/652 official bundled-parser comparisons passing, 626 native model passes (583 HTML projections and 43 independent source-authored raw models) and 26 uncheckable native-model examples, with explicit comparison limits and information loss. Its strict mode fails as intended. Feature 001 retains its historical bounded-slice results. See the [roadmap](roadmap.md) and [feature 001 verification](001-static-text/verification.md) for the distinction between future coverage and current records.

Feature 006, explicitly approved on 2026-10-08, adds bounded top-level Setext headings, thematic breaks and indented code in package 0.4.0 / module 0.4. It preserves paragraph joining and unsupported-container fallback; nested lists/block quotes are deferred to Feature 007. Full conformance remains unverified. See [006 requirements](006-core-leaf-blocks/requirements.md), [design](006-core-leaf-blocks/design.md) and [verification](006-core-leaf-blocks/verification.md).
