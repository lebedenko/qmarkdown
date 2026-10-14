# QMarkdown 1.7.0

QMarkdown 1.7.0 / QML 1.7 adds opt-in document-wide desktop selection across native
Markdown blocks. Mouse dragging, Shift-click, standard Select All/Copy shortcuts
and Escape operate on a deterministic plain-text projection. Links retain ordinary
click activation and selection gestures suppress it.

Read-only selection text and normalized UTF-16 offsets expose selection state.
`copyRequested(text)` leaves clipboard ownership with hosts, and host controls can
copy after focus transfer. Translucent selection styling preserves text colors;
selection follows grapheme, bidi, wrapping and padded inline-code geometry.

Copy text includes list/task markers, image alt text and tab-separated tables while
preserving code/HTML whitespace. Resources, visual wrapping and action headers do
not alter it. Selection survives resource/layout changes and clears on source,
effective parser-option changes or disabling. The playground enables selection and
supplies Select All/Copy controls and feedback. See the [selection guide](selection.md).

Historical QML 1.0–1.6 imports and compatible CMake 1.0–1.7 requests remain supported.
EXACT 1.7.0 selects this package; future 1.8/2.0 and exact historical versions are
rejected. Dependencies and private C++ boundaries are unchanged. Supported scope
remains Linux x86_64, Qt >=6.8, C++17 and CMake >=3.21.

Touch handles, automatic edge scrolling, double/triple-click selection, full caret
navigation, rich-text/Markdown copying and editing are outside this scope. Source
and Arch distribution instructions are in the [installation guide](release-install.md).
Publication requires separate authorization.
