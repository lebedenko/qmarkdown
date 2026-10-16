# QMarkdown 1.11.0

<!-- current-release:start -->
QMarkdown 1.11.0 / QML 1.11 adds optional fenced-code syntax highlighting.

Set `syntaxHighlightingEnabled: true` per view and inspect read-only
`syntaxHighlightingAvailable` for compiled backend availability. Replaceable
`SyntaxTheme` Light/Dark palettes expose resettable token colors, independent of
Markdown fonts, code surfaces and host visual identity. Explicit overrides survive
preset changes. Null or destroyed external themes restore the view's Light default.
See the [syntax guide](syntax-highlighting.md) for colors, languages and limits.

The private backend uses KSyntaxHighlighting >=6.10.0. Source builds default it
OFF; explicit ON requires the dependency. Arch enables it with
`syntax-highlighting>=6.10.0`. Enabled static consumers also discover the KDE
CMake package. QMarkdown links the dependency externally without vendoring its
syntax resources. CI pins KDE/ECM 6.10.0 to each selected Qt SDK with verified
archive hashes and recorded provenance.

Source displays immediately. One application worker supplies complete semantic
UTF-16 spans with stale-result guards, cancellation, duplicate-request sharing and
a 128-entry / 8 MiB theme-independent LRU. Whole-block limits are 256 Ki UTF-16
units, 16 Ki per line, 65,536 spans and a 100 ms cooperative processing budget.
An individual backend call cannot be forcibly interrupted.

Unknown/empty languages and indented code stay plain. Case-insensitive highlighting
language resolution preserves case-sensitive custom renderer precedence. Find and
selection can reveal highlighted source; code actions and document copy retain
the existing exact source payloads. Native wrapping, alignment and terminal display
newline behavior are preserved. Playground controls demonstrate both palettes.

Historical QML 1.0–1.10 and unversioned imports and compatible CMake 1.0–1.11
requests remain supported. EXACT 1.11.0 selects this package; exact older requests
require historical packages. Requests newer than 1.11.0 and 2.x are rejected.
Linux amd64 requires Qt >=6.8, C++17 and CMake >=3.21; configured verification
targets are Qt 6.8.0 and 6.12.0. Native implementation types remain private;
there is no public C++ ABI. Hosts own clipboard, scrolling and visual identity.
Public highlighting providers, Tree-sitter, streaming, arbitrary theme files,
token fonts/emphasis, Mermaid and math remain deferred.

Download [1.11.0 release assets](https://github.com/lebedenko/qmarkdown/releases/tag/v1.11.0):
`qmarkdown-1.11.0.tar.gz` and `qmarkdown-1.11.0-1-x86_64.pkg.tar.zst`, each with a
`.sha256` companion. See [installation commands](release-install.md) and
[Arch installation](arch-install.md). Historical [1.10.0 notes](release-notes-1.10.0.md)
retain that release's behavior. Actual checks and limitations are recorded in
[Feature 049 verification](../specs/049-syntax-highlighting/verification.md).
<!-- current-release:end -->
