# Changelog

## 1.6.0 — release preparation

- Add independent default-off tables and tag-filter controls plus aggregate GFM enablement that preserves individual values.
- Render equal-width native tables with wrapping, alignment, bold headers, formatted links/code/strikethrough and resource-controlled images inside lists and quotes.
- Add grouped table surfaces, borders and padding with inherited foreground, transparent defaults and 4/2 logical-pixel padding in every preset.
- Retain private filtered HTML semantics while preserving visible raw spelling; keep CommonMark 0.31.2 core and bounded extension claims.
- Reserve layout-aware inline-code padding with 4/2 defaults in every preset, fragment backgrounds and source-mapped linked padding; zero metrics restore previous spacing.
- Extend package/module compatibility and installed consumers to 1.6.0/1.6. Release delivery and publication are authorized; exact-commit gates remain pending.

## 1.5.0 — 2026-10-13

- Ship task lists, strikethrough and extended autolinks together; historical 1.3/1.4 candidates were not published. All three switches are independent and default off.
- Add independent, default-off `MarkdownView.extendedAutolinksEnabled` for HTTP(S), `www.`, email, `mailto:` and `xmpp:` including resources.
- Preserve explicit-link precedence, inert image descriptions, CommonMark defaults and host-owned navigation; reuse native link styling and activation.
- Advance package/QML versions to 1.5.0/1.5 with historical imports, installed consumers and a playground toggle/sample.
- Add all 14 pinned GFM section 6.9 examples with separate offline semantic/span expectations; see [extension contract](docs/extended-autolinks.md) and [verification](specs/037-gfm-extended-autolinks/verification.md).

## 1.4.0 source candidate (unreleased)

- Add independent, default-off `MarkdownView.strikethroughEnabled` with native inline strikeout, nested formatting, links and image descriptions.
- Retain CommonMark 0.31.2 and historical imports; advance candidate package/QML versions to 1.4.0/1.4.
- Add selected GFM strikethrough fixtures, playground controls and installed consumers.

## 1.3.0 — unreleased

- Add default-off `MarkdownView.taskListsEnabled` with native read-only checked/unchecked indicators in ordered, unordered, nested and quoted lists. Preserve source text, list numbering, styling and host resource policies.
- Retain CommonMark 0.31.2 as the default parser contract; recognize raw task prefixes through a bounded private cmark patch and test the extension separately. Other GFM extensions remain deferred.
- Add playground controls and a QML 1.3 installed consumer while retaining earlier 1.x imports. CMake accepts compatible earlier 1.x requests and EXACT 1.3.0; exact historical requests require historical packages.
- See [the task-list contract](docs/task-lists.md) and [iteration 035 verification](specs/035-gfm-task-lists/verification.md). This candidate has not been published.

## 1.2.0 — 2026-10-12

- Add optional host-controlled action headers to fenced/indented code blocks, including nested and empty blocks. `codeBlockActions` delegates receive exact `codeText` and an invalidated-on-removal `requestCopy` callback; requests emit `codeBlockCopyRequested`.
- Preserve default code geometry and library dependencies. Hosts own controls, clipboard, accessibility and feedback; the playground demonstrates plain-text Copy with brief feedback.
- Prepare QML 1.2 and installed action consumers, retaining QML 1.0/1.1 compatibility. CMake accepts compatible earlier 1.x requests and EXACT 1.2.0; exact historical versions require historical packages.
- See [the action contract](docs/code-block-actions.md) and [iteration 031 local verification](specs/031-code-block-copy-actions/verification.md). [Release notes](docs/release-notes-1.2.0.md) and [iteration 032](specs/032-v1.2-integration/verification.md) record completed integration; [iteration 033](specs/033-stable-v1.2-release/verification.md) records completed stable/latest publication and downloaded-asset acceptance.

## 1.1.0 — 2026-10-11

- Add opt-in fixed light/dark Polished and Minimal presets, grouped heading/inline-code/code-block styling, explicit override tracking and binding-preserving resets. Legacy remains the default; existing flat properties and 1.0 imports remain supported.
- Add native heading separators, inline code backgrounds and code block surfaces without parser or resource-policy changes.
- Centralize reproducible minimum/development Qt verification targets (6.8.0/6.12.0), with stable hosted packaging check names.
- Add playground controls, presentation fixtures and installed 1.1 consumers alongside existing 1.0 consumers. See [release notes](docs/release-notes-1.1.0.md) for installation and compatibility.

## 1.0.0 — 2026-10-10

- Prepare Linux amd64 support with Qt >=6.8, C++17 and CMake >=3.21; QML module version 1.0 and shared-library SOVERSION 1.
- Correct private bundled cmark 0.31.2 fence storage: openers longer than 255 characters require a matching closer at least as long as the complete opener. Upstream corpus, version, checksum and notices are retained.
- Guarantee documented public QML properties, signals, behavior and CMake integration throughout 1.x. Implementation classes and QMarkdown.Private remain private; no installed C++ headers, public C++ ABI or portable prebuilt binary guarantee.
- Retain native CommonMark blocks/inlines, host-controlled links, PNG/JPEG image rows and iteration 020's responsive decoder lifecycle. HTML stays literal; document changes replace the full model; narrow widths may overhang glyphs. Codecs cannot be interrupted, two occupied workers can delay fresh work, and application shutdown may wait for codecs.

### Migration

Replace `import QMarkdown 0.7` with `import QMarkdown 1.0`, or use an unversioned import. Replace CMake `find_package(QMarkdown 0.7 CONFIG REQUIRED)` with `find_package(QMarkdown 1.0 CONFIG REQUIRED)`. Legacy 0.x imports/package requests are unsupported. SameMajorVersion package compatibility accepts requests within major 1 up to the installed version; 1.0.0 accepts 1.0 and EXACT 1.0.0, and rejects 0.7, 1.1 and 2.0. Consumers build against a compatible Qt/toolchain.

### Candidate evidence

See [iteration 021 verification](specs/021-v1-release-candidate/verification.md) for actual local gate results, environments and artifacts. These historical candidate results predate publication. Both hosted packaging checks subsequently passed in [run 37999664817](https://github.com/lebedenko/qmarkdown/actions/runs/37999664817). Stable publication evidence is recorded separately in [Feature 023](specs/023-stable-release/verification.md). Optional syntax, streaming, selection/copy, renderer extensions, performance optimization and broader platforms remain deferred.
