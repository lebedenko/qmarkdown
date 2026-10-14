# QMarkdown 1.8.0

<!-- current-release:start -->
QMarkdown 1.8.0 / QML 1.8 adds host QML custom fenced-block renderers and desktop
reading improvements. Double-click selects Unicode words, whitespace runs or
punctuation graphemes. Triple-click selects a prose paragraph or heading, a table
cell, or one logical code/literal-HTML line including its terminating newline.
Dragging extends by complete units. Semantic paragraphs include image alt text
and exclude synthetic list prefixes and sibling separators.

With selection enabled, desktop mouse links wait for the platform double-click
interval; repeated clicks, dragging and Shift-click suppress activation. Touch
and selection-disabled links retain their behavior. Actions remain excluded and
clipboard ownership remains with hosts.

`selectionViewport` and `selectionScrollRequested(real deltaY)` let hosts provide
vertical edge scrolling. A stationary window-space pointer follows newly exposed
content; hosts clamp their scroll positions. Null viewports disable requests.
The playground integrates this interface. See the [selection guide](selection.md).

Hosts provide trusted QML Components through `fencedBlockRenderers`. Dispatch uses
the first case-sensitive info-string token; invalid or unmatched components fall
back to source. Selection replaces custom visuals with selectable source. No
highlighting, Mermaid or math engine is bundled. See the [renderer guide](fenced-block-renderers.md).

The release documentation gate validates current summaries, versions, asset names,
download URLs and installation commands before distribution or publication.
Consult the actual checks in the [iteration record](../specs/044-v1.8-release/verification.md).

Historical QML 1.0–1.7 imports and compatible CMake 1.0–1.8 requests remain supported.
EXACT 1.8.0 selects this package; future 1.9/2.0 and exact historical versions are
rejected. Dependencies and private C++ boundaries remain unchanged. Supported
scope is Linux x86_64, Qt >=6.8, C++17 and CMake >=3.21.

Streaming, bundled renderer engines, touch handles, caret navigation, additional
copy formats, horizontal autoscrolling and broader platforms remain deferred.
Download source and Arch assets from the [1.8.0 release](https://github.com/lebedenko/qmarkdown/releases/tag/v1.8.0):
`qmarkdown-1.8.0.tar.gz` and `qmarkdown-1.8.0-1-x86_64.pkg.tar.zst`, each with a
`.sha256` companion. Use the [installation guide](release-install.md).
<!-- current-release:end -->
