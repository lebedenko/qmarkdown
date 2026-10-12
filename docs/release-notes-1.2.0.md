# QMarkdown 1.2.0

Package 1.2.0 / QML module 1.2 adds optional host-controlled code-block actions. Release date: 2026-10-12 UTC. Integration results are recorded in [iteration 032](https://github.com/lebedenko/qmarkdown/blob/v1.2.0/specs/032-v1.2-integration/verification.md); publication and downloaded-asset results are recorded separately in [iteration 033](https://github.com/lebedenko/qmarkdown/blob/docs/v1.2.0-publication-evidence/specs/033-stable-v1.2-release/verification.md).

- Assign `MarkdownView.codeBlockActions` to an Item Component declaring `required property string codeText` and `required property var requestCopy`. Calling `requestCopy()` emits `codeBlockCopyRequested(string text)` on its owning view.
- Fenced, indented, nested and empty blocks receive exact parser text, including whitespace and trailing newlines, without fences/info strings or further normalization. Headers sit above code inside surface padding. Null remains the default and preserves existing rendering and dimensions.
- Component/block replacement, removal, zero width and view destruction invalidate old callbacks. Delegate creation failures diagnose while preserving code rendering. Hosts own spacing, styling, accessibility, clipboard and feedback; no Qt Quick Controls dependency is added to the library.
- The playground supplies a host clipboard helper and Copy/Copied control with keyboard, mouse and touch activation. Installed/relocated and distribution verification includes QML 1.0/1.1/1.2 consumers, linked and plugin-only where applicable.

Use `import QMarkdown 1.2` for the added contract. Existing 1.0/1.1 and unversioned consumers remain supported. Compatible CMake requests for 1.0/1.1/1.2 and EXACT 1.2.0 are accepted; EXACT 1.0.0/1.1.0 requires those historical packages. Linux amd64, Qt >=6.8, C++17 and CMake >=3.21 remain supported. Public QML/CMake behavior retains 1.x compatibility; private native classes have no public C++ ABI guarantee.

Legacy styling defaults and host navigation/resource ownership remain unchanged. HTML stays literal, PNG/JPEG images occupy separate rows, updates replace the document and indivisible glyphs may overhang narrow widths. Codecs cannot be interrupted; two busy workers may delay new work and shutdown may wait for codecs. Document-wide selection, inline-code actions, language labels, GFM, streaming, highlighting and generic renderer extensions remain deferred.

The release distributes an Ubuntu 24.04 x86_64 archive built with Qt 6.8.0 and an Arch package. Verification checks the same minimum-built Ubuntu archive on Qt 6.8.0 and 6.12.0 and disposable Arch installation/ownership/consumers/upgrade/removal. Qt is not bundled; cross-runtime results are bounded evidence, not a portable binary guarantee. See [code-block actions](https://github.com/lebedenko/qmarkdown/blob/v1.2.0/docs/code-block-actions.md), [Ubuntu installation](https://github.com/lebedenko/qmarkdown/blob/v1.2.0/docs/release-install.md) and [Arch packaging](https://github.com/lebedenko/qmarkdown/blob/v1.2.0/docs/arch-install.md).

Download both files for your distribution and verify its checksum before installation:

- Ubuntu: [archive](https://github.com/lebedenko/qmarkdown/releases/download/v1.2.0/qmarkdown-1.2.0-ubuntu24.04-x86_64-qt6.8.tar.gz), [SHA-256](https://github.com/lebedenko/qmarkdown/releases/download/v1.2.0/qmarkdown-1.2.0-ubuntu24.04-x86_64-qt6.8.tar.gz.sha256).
- Arch: [package](https://github.com/lebedenko/qmarkdown/releases/download/v1.2.0/qmarkdown-1.2.0-1-x86_64.pkg.tar.zst), [SHA-256](https://github.com/lebedenko/qmarkdown/releases/download/v1.2.0/qmarkdown-1.2.0-1-x86_64.pkg.tar.zst.sha256).
