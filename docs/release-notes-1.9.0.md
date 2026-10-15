# QMarkdown 1.9.0

<!-- current-release:start -->
QMarkdown 1.9.0 / QML 1.9 adds literal document Find, a recursive heading outline
and explicit local fragment navigation. Find includes case modes, independent
ordinary/current highlights and wrapping previous/next actions. Search uses the
decoded plain-text projection, including tables, code and image descriptions.
Active Find replaces custom fenced visuals with searchable source; closing it
restores eligible visuals subject to selection mode.

Outline entries contain H1–H6 level, decoded title and unique generated fragment
ID, including nested headings. IDs use lowercase text, punctuation removal,
whitespace hyphens and collision-safe numeric suffixes; GitHub slug compatibility
is not claimed. Explicit local resolvers decode once and report missing targets.
`linkActivated(destination)` remains unchanged.

`navigationRequested(rect target)` exposes refreshed geometry in view coordinates.
Hosts retain scrolling, controls, clipboard and navigation policy. The playground
adds Find controls and shortcuts, a collapsible outline and local fragment links.
See the [reader guide](reading-navigation.md) and
[actual verification record](../specs/045-reading-navigation/verification.md).

Historical QML 1.0–1.8 imports and compatible CMake 1.0–1.9 requests remain supported.
EXACT 1.9.0 selects this package; future 1.10/2.0 and exact historical versions are
rejected. Dependencies and private C++ boundaries remain unchanged. Linux x86_64
requires Qt >=6.8, C++17 and CMake >=3.21.

Regex, replacement, keyboard link traversal, caret navigation, streaming, bundled
renderer engines and broader platform support remain deferred.
Download source and Arch assets from the [1.9.0 release](https://github.com/lebedenko/qmarkdown/releases/tag/v1.9.0):
`qmarkdown-1.9.0.tar.gz` and `qmarkdown-1.9.0-1-x86_64.pkg.tar.zst`, each with a
`.sha256` companion. See the [installation guide](release-install.md).
<!-- current-release:end -->
