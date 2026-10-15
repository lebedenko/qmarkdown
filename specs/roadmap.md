# Staged roadmap

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
Controls style. Private QML responsibilities are refactored without a new public
C++ surface.
<!-- current-release:end -->

## Historical 1.6.0 status

Package **1.6.0 / QML 1.6** is [released](https://github.com/lebedenko/qmarkdown/releases/tag/v1.6.0) on **2026-10-13 UTC**. Optional code-block actions retain host-owned clipboard handling. Styling includes opt-in Polished/Minimal light/dark presets; Legacy remains the default. Inline-code padding defaults to 4/2 logical pixels in every preset; zero metrics restore previous spacing. Hosts own visual identity, scrolling, navigation and resource authorization.

Linux amd64 support requires Qt >=6.8, C++17 and CMake >=3.21. Configured verification targets are minimum Qt 6.8.0 and development Qt 6.12.0, established by [iteration 024](024-development-host-qt/verification.md). Earlier Qt 6.11.2/6.11.3 and native Wayland runs remain historical evidence, not the current configured matrix.

The evidence contracts remain separate: [iteration 017](017-complete-semantic-evidence/verification.md) establishes all 652 pinned CommonMark parser/model semantic passes; [iteration 019](019-native-presentation-evidence/coverage.md) maps representative native presentation assertions, extended by [iteration 025](025-styling-presets/verification.md); iteration 033 records exact-release packaging, publication and downloaded-asset lifecycle checks. The Ubuntu 24.04 x86_64 archive built with Qt 6.8.0 passed on exact Qt 6.8.0 and 6.12.0 runtimes; the published Arch package also passed independent ownership, consumers, upgrade and removal checks. This is bounded binary evidence, not a portable binary guarantee.

Documented public QML behavior and CMake integration remain compatible throughout 1.x. QMarkdown.Private/native classes remain private; no C++ headers are installed and no public C++ ABI is promised. Package 1.6.0 accepts compatible 1.0–1.6 requests and EXACT 1.6.0; exact earlier versions require historical packages.

Accepted limits remain separate PNG/JPEG image rows, literal HTML, full document replacement, narrow-glyph overhang and uninterruptible codecs. Two occupied decoder workers can delay fresh work; application shutdown may wait for codecs. Touch selection handles, edge autoscrolling, rich-text copying, streaming, highlighting, bundled renderer engines, inline-code borders, arbitrary style inheritance, further performance optimization and broader platforms remain deferred. Historical descriptions below retain their original versions and verification scope.


These stages preserve the brief's direction. The table separates completed foundations from deferred capabilities; it does not schedule future releases. Each feature needs its own requirements, design, tasks, and approval. The [standards reference](standards.md) defines the pinned target and future conformance evidence.

| Stage | Intended scope |
| --- | --- |
| Buildable scaffold | [002 project scaffold](002-project-scaffold/requirements.md): import-only Qt/QML module, shared/static packaging, and smoke checks; scaffold foundation completed. |
| First vertical slice | [001 static text](001-static-text/requirements.md): paragraphs, H1–H6, native block layout, styling, replacement updates. Approved on 2026-10-07 and implemented; see its verification record. |
| CommonMark core coverage (completed) | Complete CommonMark 0.31.2 semantics: ATX and Setext headings, paragraphs, emphasis/strong emphasis, inline code, fenced and indented code, inline and reference links, images, ordered/unordered and nested lists, block quotes, thematic breaks, escapes/entities, autolinks, hard/soft breaks, and raw HTML recognition. Include whitespace, line handling, delimiter rules, and precedence. Features 009/010 specify host navigation and resource policies; Feature 017 completes semantic evidence. |
| Selected GFM syntax (implemented) | Task lists, strikethrough and extended autolinks have independent opt-in slices and separate extension checks on the newer CommonMark core; tables and tag filtering are released in 1.6.0; no full GFM conformance claim. |
| Interaction improvements | Optional host-controlled code-block actions released in 1.2.0; [Feature 040](040-document-selection/requirements.md) implements opt-in document-wide desktop selection and host-owned plain-text copy requests in 1.7.0 / QML 1.7. Selection is included in the 1.7.0 downloads. Iteration 044 adds double/triple-click selection and host-owned vertical edge scrolling in 1.8. Touch handles, horizontal autoscrolling, caret navigation and rich-text copying remain deferred. |
| Streaming | Static/appended semantic consistency, incremental document updates and rendering; specify stability and performance checks. |
| Renderer extensions | [Feature 042](042-custom-fenced-block-renderers/requirements.md), approved on 2026-10-14, implements host QML custom fenced-block renderers after the 1.7.0 tag; these are absent from 1.7.0 downloads. Selection shows source; existing actions copy source. Verification/delivery are recorded separately. Bundled syntax highlighting, Mermaid and math remain deferred. Footnotes and other richer syntax require separate specifications. |
| v1.0 conformance gate (completed) | All pinned CommonMark 0.31.2 official examples pass semantic checks, with native renderer verification and documented resource/HTML behavior. Record reproducible commands and actual counts. Streaming and optional extensions are not gates. |

The capability stages separate work streams rather than impose delivery dates or require optional work before v1.0. The static-text slice is implemented; Feature 017 completes pinned-corpus semantic evidence. Iteration 021 records historical local candidate readiness; iterations 023/027 record completed 1.0.0/1.1.0 publication separately.

## Development work for 1.10

Released **1.9.0 / QML 1.9** remains the published baseline; see its
[release notes](../docs/release-notes-1.9.0.md). The current summary above describes
1.10 content; publication remains separately authorized.
[046 QML boundaries](046-qml-boundaries/verification.md) and
[047 task-indicator presentation](047-task-indicator-presentation/verification.md)
are implemented and locally verified development work. The development package is
1.10.0 / QML 1.10; these records do not claim 1.10 publication.

Feature 048 adds opt-in keyboard link traversal, visible link focus and per-link
accessibility through [048 requirements](048-keyboard-reading/requirements.md),
[design](048-keyboard-reading/design.md), [tasks](048-keyboard-reading/tasks.md) and
[verification](048-keyboard-reading/verification.md). Feature 048 requirements,
design and tasks were approved on 2026-10-15; implementation and local verification
on both configured Qt shared/static targets are complete. Release documentation and delivery
belong to that meaningful change; merge, tagging and publication need separate
authorization. Streaming is excluded. Highlighting, Mermaid/math engines,
footnotes, caret navigation, touch selection handles and further platforms remain
deferred.

## Historical implementation stages

First-slice fallback rendering was temporary behavior for unsupported syntax. The approved feature 001 design aligns supported paragraph/ATX structure with CommonMark 0.31.2 and specifies literal inline and protected-fence fallback, concrete styling, sizing, and updates. Feature 003 supersedes the paragraph/heading literal-inline behavior with bounded emphasis/strong/code and escapes/entities while retaining the block splitter and protected fences. Feature 004 supersedes protected fences with bounded native code rendering and independent font/color. Full block precedence, remaining inline semantics, and fenced-block extensions require later approved revisions. The standards target does not authorize implementation. Feature 002 approval and scaffold scope remain unchanged.

Feature 006, explicitly approved on 2026-10-08, adds bounded top-level Setext headings, thematic breaks and indented code in package 0.4.0 / module 0.4. It preserves paragraph joining and unsupported-container fallback; nested lists/block quotes are deferred to Feature 007. Full conformance remains unverified. See [006 requirements](006-core-leaf-blocks/requirements.md), [design](006-core-leaf-blocks/design.md) and [verification](006-core-leaf-blocks/verification.md).

Feature 007, approved for implementation on 2026-10-08, supersedes unsupported-container/resource-source fallback with native recursive lists/quotes and inert formatted labels in package 0.5.0 / module 0.5. Production parsing now adapts one cmark tree. Hard breaks and reference definitions follow cmark; HTML stays literal. Full conformance and resource rendering remain deferred. See [requirements](007-container-blocks/requirements.md), [design](007-container-blocks/design.md), [tasks](007-container-blocks/tasks.md) and [verification](007-container-blocks/verification.md).

Feature 009: [host-controlled links](009-host-controlled-links/requirements.md), approved on 2026-10-08 for 0.6.0/0.6. See [design](009-host-controlled-links/design.md), [tasks](009-host-controlled-links/tasks.md), and [verification](009-host-controlled-links/verification.md). Hosts own navigation; resource rendering remains deferred.

Feature 010: [images and host resource policy](010-images-and-resource-policy/requirements.md), approved on 2026-10-08 for 0.7.0/0.7. See design, tasks and verification in that directory.

## CommonMark verification baseline

[Feature 011: CommonMark conformance baseline](011-commonmark-baseline/requirements.md) was approved on 2026-10-08 and implemented after Feature 010. The pinned official corpus establishes separate bundled-parser and production-model evidence, with explicit limits and regression checks. Package/module 0.7.0/0.7 and production behavior remain unchanged. Feature 012, approved on 2026-10-08, now supplies that independent coverage for the 43 HTML-block examples, plus Linux Qt CI, deterministic lifecycle verification and opt-in advisory benchmarks. Feature 015 now covers the remaining 26 uncheckable cases with independent complete raw-model expectations. Feature 016 implements the subsequently approved inline semantic retention scope. The baseline does not satisfy the v1.0 gate. See [design](011-commonmark-baseline/design.md), [tasks](011-commonmark-baseline/tasks.md) and [verification](011-commonmark-baseline/verification.md).

See [012 requirements](012-verification-hardening/requirements.md), [design](012-verification-hardening/design.md), [tasks](012-verification-hardening/tasks.md) and [verification](012-verification-hardening/verification.md). Performance thresholds and optimization remain outside this iteration.

Feature 013: [efficient inline format preparation](013-inline-format-performance/requirements.md), approved on 2026-10-08 through the explicit implementation request. See [design](013-inline-format-performance/design.md), [tasks](013-inline-format-performance/tasks.md) and [verification](013-inline-format-performance/verification.md). Scope is preparation only; package/module remain 0.7.0/0.7.

Feature 014: [local CI tasks](014-local-ci/verification.md), completed local Ubuntu 24.04 amd64 verification on Qt 6.8.0 and 6.11.3. Both passed seven shared and seven static CTest checks, packaging, symbol privacy, QML lint and benchmark smoke checks. Hosted GitHub workflow execution remains pending.

Feature 015: [close remaining oracle gaps](015-oracle-gaps/requirements.md), approved on 2026-10-09 through the explicit implementation request. See [design](015-oracle-gaps/design.md), [tasks](015-oracle-gaps/tasks.md) and [verification](015-oracle-gaps/verification.md). Tests/documentation only; package/module remain 0.7.0/0.7. The corpus has 652 parser and 652 model passes, zero mismatches/uncheckable; strict mode still fails for retained limits/losses. Feature 016 subsequently implements inline semantic retention; GFM, streaming, selection/copy and further performance work remain deferred.

Feature 016: [preserve inline semantics](016-inline-semantic-retention/requirements.md), approved on 2026-10-09; implemented and locally verified on host/shared/static, Qt 6.8.0 and Qt 6.11.3. Owned trees retain soft breaks, titles, repeated emphasis nesting, empty links and inline HTML identity. Native display/navigation/resource policies and versions remain unchanged. The corpus has 652 parser/model passes, no mismatches/uncheckable examples and no reported semantic losses. Independent evidence still has 390 limited examples, including two newly explicit HTML source ambiguities; strict mode fails. Closing remaining evidence limits requires later approved work. See [verification](016-inline-semantic-retention/verification.md).

Feature 017: [complete independent semantic evidence](017-complete-semantic-evidence/requirements.md), approved on 2026-10-09. Complete source-reviewed models resolve the 390 iteration 016 limited examples while preserving every required HTML check. Strict corpus verification passes with 652 parser/model passes and zero unresolved limits/losses. Production models, public interfaces and 0.7.0/0.7 versions are unchanged. This supersedes Feature 016’s remaining evidence work; native presentation and release requirements still need their own assessment before v1.0 readiness. See [actual verification](017-complete-semantic-evidence/verification.md).

Feature 019: [native presentation evidence](019-native-presentation-evidence/requirements.md), approved on 2026-10-09. Implemented and verified on host shared/static and both pinned Qt CI versions. The [coverage matrix](019-native-presentation-evidence/coverage.md) maps existing/new assertions to native contracts; [verification](019-native-presentation-evidence/verification.md) records actual execution separately. Production behavior and 0.7.0/0.7 remain unchanged. This prepares the native assessment without declaring or publishing v1.0; release decisions and documented verification limits remain separate.

Iteration 031: [host-controlled code-block copy actions](031-code-block-copy-actions/requirements.md), approved on 2026-10-12 under the original local iteration-030 plan; renumbered because main owns 030 for Arch integration. Implementation and historical local verification are retained in [design](031-code-block-copy-actions/design.md), [tasks](031-code-block-copy-actions/tasks.md) and [verification](031-code-block-copy-actions/verification.md). Package 1.2.0/QML 1.2 is integrated; publication completed in iteration 033.

Iteration 032: [1.2 candidate integration](032-v1.2-integration/requirements.md), approved on 2026-10-12 through the supplied plan. See [design](032-v1.2-integration/design.md), [mapped tasks](032-v1.2-integration/tasks.md) and [planned/actual verification](032-v1.2-integration/verification.md). Integration is complete; iteration 033 records successful stable/latest v1.2.0 publication and independent downloaded-asset acceptance.

Iteration 033: [stable v1.2.0 publication](033-stable-v1.2-release/requirements.md), approved on 2026-10-12 through the supplied plan. See [design](033-stable-v1.2-release/design.md), [mapped tasks](033-stable-v1.2-release/tasks.md) and [planned/actual verification](033-stable-v1.2-release/verification.md). Includes protected release preparation, fresh exact-main gates, annotated tagging, Ubuntu/Arch stable/latest publication and independent downloaded-asset acceptance. Publication and independent downloaded-asset acceptance are complete.
