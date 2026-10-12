# Changelog

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
