# Changelog

## Unreleased — 1.0.0 candidate

- Prepare Linux amd64 support with Qt >=6.8, C++17 and CMake >=3.21; QML module version 1.0 and shared-library SOVERSION 1.
- Correct private bundled cmark 0.31.2 fence storage: openers longer than 255 characters require a matching closer at least as long as the complete opener. Upstream corpus, version, checksum and notices are retained.
- Guarantee documented public QML properties, signals, behavior and CMake integration throughout 1.x. Implementation classes and QMarkdown.Private remain private; no installed C++ headers, public C++ ABI or portable prebuilt binary guarantee.
- Retain native CommonMark blocks/inlines, host-controlled links, PNG/JPEG image rows and iteration 020's responsive decoder lifecycle. HTML stays literal; document changes replace the full model; narrow widths may overhang glyphs. Codecs cannot be interrupted, two occupied workers can delay fresh work, and application shutdown may wait for codecs.

### Migration

Replace `import QMarkdown 0.7` with `import QMarkdown 1.0`, or use an unversioned import. Replace CMake `find_package(QMarkdown 0.7 CONFIG REQUIRED)` with `find_package(QMarkdown 1.0 CONFIG REQUIRED)`. Legacy 0.x imports/package requests are unsupported. SameMajorVersion package compatibility accepts requests within major 1 up to the installed version; this candidate accepts 1.0 and EXACT 1.0.0, and rejects 0.7, 1.1 and 2.0. Consumers build against a compatible Qt/toolchain.

### Candidate evidence

See [iteration 021 verification](specs/021-v1-release-candidate/verification.md) for actual local gate results, environments and artifacts. This entry describes an unreleased candidate; it does not record a tag or publication. Hosted GitHub execution remains a pre-publication follow-up. Optional syntax, streaming, selection/copy, renderer extensions, performance optimization and broader platforms remain deferred.
