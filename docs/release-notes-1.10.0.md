# QMarkdown 1.10.0

<!-- current-release:start -->
QMarkdown 1.10.0 / QML 1.10 adds keyboard reading and presentation polish.

Set `keyboardNavigationEnabled: true` to traverse native links with Tab/Shift+Tab
in document order. Traversal exits to the host focus chain at either boundary.
Enter/Return activates once per nonrepeated press through `linkActivated`.
`keyboardFocusColor` follows `style.linkColor` by default and paints a
two-logical-pixel outline around every active fragment without layout changes.
`navigationRequested` asks the host to reveal the focused geometry; hosts retain
scrolling, destination policy and fragment resolution.

Logical identities preserve one traversal stop and accessible Link across wrapped
text and loaded image rows. Distinct links with the same destination remain
separate. Accessible links expose decoded names, destination, geometry, focus
state and press action independently of keyboard opt-in. Source and effective
parser-option changes invalidate old activation callbacks. Selection, Find,
pointer activation and host controls coexist with keyboard links. See the
[reader guide](reading-navigation.md).

`taskIndicator` accepts host decorative components with live checked/style inputs
and preferred dimensions. The library supplies an inert rounded fallback;
the playground demonstrates the active Controls checkbox style and a fallback
toggle. See the [task presentation contract](task-lists.md#host-presentation-110).
The private QML responsibility refactor preserves public behavior and keeps
application styling, resource policy, controls and navigation host-owned.

Keyboard traversal defaults off; accessible links do not require it. Existing
hosts can keep their current keyboard behavior. Historical QML 1.0–1.9 imports,
unversioned imports and compatible CMake 1.0–1.10 requests remain supported.
EXACT 1.10.0 selects this package; exact older versions require historical
packages. Requests newer than 1.10.0 and 2.x are rejected. Linux amd64 requires
Qt >=6.8, C++17 and CMake >=3.21. Configured verification targets remain Qt 6.8.0
and 6.12.0. No new library dependency or public C++ ABI is introduced.

Streaming, highlighting, Mermaid/math engines, footnotes, caret navigation, touch
selection handles and additional platforms remain deferred. Static source
replacement and existing resource policy remain the update model.

Download source and Arch assets from the [1.10.0 release](https://github.com/lebedenko/qmarkdown/releases/tag/v1.10.0):
`qmarkdown-1.10.0.tar.gz` and `qmarkdown-1.10.0-1-x86_64.pkg.tar.zst`, each with a
`.sha256` companion. See [download, checksum and installation commands](release-install.md)
and [Arch installation](arch-install.md). Historical 1.9 behavior is documented in
[1.9.0 release notes](release-notes-1.9.0.md). Actual checks and their limits are
recorded separately in [feature 048 verification](../specs/048-keyboard-reading/verification.md),
[046 verification](../specs/046-qml-boundaries/verification.md) and
[047 verification](../specs/047-task-indicator-presentation/verification.md).
<!-- current-release:end -->
