# QMarkdown 1.1.0 release notes — draft

Package 1.1.0 / QML module 1.1 adds opt-in native styling while retaining Legacy defaults and existing QML 1.0 consumers. This is an unreleased candidate; tagging and publication are separate approved work. See [iteration 026 verification](../specs/026-v1.1-integration/verification.md) for exact candidate evidence.

- Choose fixed `PolishedLight`, `PolishedDark`, `MinimalLight` or `MinimalDark` presets. Polished adds H1/H2 separators and code surfaces; Minimal keeps construction typography and spacing with decorations disabled. Presets do not subscribe to an application theme or paint a document background.
- Configure permanent `headings.h1`–`headings.h6`, `inlineCode` and `codeBlock` groups. Existing flat heading/font/color properties share the same state and notifications.
- Explicit assignments override preset values, including default values, false, zero and transparency. Individual grouped-property RESET clears one override; `resetOverrides()` restores the selected preset without detaching host QML bindings. Font edits retain whole-font overrides and resolve masks. Bind through one spelling of each aliased property.
- Native heading separators appear once per complete heading, including image rows. Inline-code backgrounds follow wrapped/bidirectional visual runs without changing text layout or link hit areas. Fenced/indented code remains literal native text with configurable surfaces, border, radius and padding.
- The playground includes preset/group controls and reset actions. Packaging verifies both 1.0 and 1.1 installed/relocated consumers on exact Qt 6.8.0 and 6.12.0; the minimum-built archive is checked on both runtimes.

Use `import QMarkdown 1.1` for the added API. Existing `import QMarkdown 1.0` and unversioned imports remain supported. CMake requests for 1.0 and 1.1 and EXACT 1.1.0 are accepted; EXACT 1.0.0 requires that historical package. Linux amd64, Qt >=6.8, C++17 and CMake >=3.21 remain the supported environment. Documented public QML behavior and CMake integration retain 1.x compatibility; private native classes have no public C++ ABI guarantee. Cross-runtime checks are bounded evidence, not a portable binary guarantee.

Hosts retain control of visual identity, backgrounds, scrolling, navigation and resource authorization. Parser/dependency/resource policy behavior is unchanged. HTML remains literal; PNG/JPEG images occupy separate rows; updates replace the full document; indivisible glyphs may overhang narrow widths. Active codecs cannot be interrupted, two busy workers may delay new work, and application shutdown may wait for codecs.

Selection/copy, GFM, streaming, highlighting, renderer extensions, inline-code padding/borders and arbitrary style inheritance remain deferred. See the [styling guide](styling.md) for palettes, metrics, aliases and reset examples.
