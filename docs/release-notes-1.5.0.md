# QMarkdown 1.5.0

Package 1.5.0 / QML module 1.5 ships three independent, default-off extensions together. Publication and downloaded-package acceptance are pending; historical 1.3/1.4 source candidates were not published separately.

- `taskListsEnabled` adds native read-only checked/unchecked indicators in ordered, unordered, nested and quoted lists. Hosts edit tasks by replacing Markdown; indicators do not toggle or modify source.
- `strikethroughEnabled` recognizes matching single/double raw tildes and uses native strikeout, composing with emphasis, code, links and image descriptions.
- `extendedAutolinksEnabled` recognizes selected HTTP(S), `www.`, email, `mailto:` and `xmpp:` forms, including XMPP resources. Explicit/reference links retain precedence; image descriptions stay inert. Hosts handle `linkActivated(destination)`; recognition grants no network permissions.

Each switch reparses original Markdown through the existing resource lifecycle. CommonMark 0.31.2 and Legacy styling remain defaults. The three switches support all eight combinations. This is selected GFM extension support, not full GFM conformance; tables and HTML filtering remain deferred.

Use `import QMarkdown 1.5` and `find_package(QMarkdown 1.5 CONFIG REQUIRED)` for the current contract. Historical QML 1.0–1.4 and unversioned imports remain supported. Compatible CMake 1.0–1.5 requests and EXACT 1.5.0 are accepted; exact historical versions require historical packages. Linux amd64, Qt >=6.8, C++17 and CMake >=3.21 remain supported. Private native classes have no public C++ ABI guarantee.

HTML stays literal, PNG/JPEG images occupy separate rows, updates replace the document and indivisible glyphs may overhang narrow widths. Image codecs cannot be interrupted; occupied workers may delay work and shutdown may wait. Streaming, document-wide selection, highlighting and generic renderer extensions remain deferred.

The release gates verify an Ubuntu 24.04 x86_64 archive built with Qt 6.8.0 on both Qt 6.8.0 and 6.12.0, plus an Arch package's installation, ownership, consumers, upgrade and removal. Qt is not bundled. These runtime results are bounded evidence, not a portable binary guarantee. See [iteration 037](https://github.com/lebedenko/qmarkdown/blob/v1.5.0/specs/037-gfm-extended-autolinks/verification.md), [task lists](https://github.com/lebedenko/qmarkdown/blob/v1.5.0/docs/task-lists.md), [strikethrough](https://github.com/lebedenko/qmarkdown/blob/v1.5.0/docs/strikethrough.md) and [extended autolinks](https://github.com/lebedenko/qmarkdown/blob/v1.5.0/docs/extended-autolinks.md).

After publication, download both files for your distribution and verify the checksum before installation. Follow the [Ubuntu guide](https://github.com/lebedenko/qmarkdown/blob/v1.5.0/docs/release-install.md) or [Arch guide](https://github.com/lebedenko/qmarkdown/blob/v1.5.0/docs/arch-install.md).

- Ubuntu: [archive](https://github.com/lebedenko/qmarkdown/releases/download/v1.5.0/qmarkdown-1.5.0-ubuntu24.04-x86_64-qt6.8.tar.gz), [SHA-256](https://github.com/lebedenko/qmarkdown/releases/download/v1.5.0/qmarkdown-1.5.0-ubuntu24.04-x86_64-qt6.8.tar.gz.sha256).
- Arch: [package](https://github.com/lebedenko/qmarkdown/releases/download/v1.5.0/qmarkdown-1.5.0-1-x86_64.pkg.tar.zst), [SHA-256](https://github.com/lebedenko/qmarkdown/releases/download/v1.5.0/qmarkdown-1.5.0-1-x86_64.pkg.tar.zst.sha256).
