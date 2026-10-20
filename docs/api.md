# Public API reference

QMarkdown 1.14.0 / QML 1.14 is a Qt Quick rendering library. Its supported surface is
`MarkdownView`, `MarkdownStyle`, `MarkdownResourcePolicy` and the CMake integration
below. Documented API remains compatible throughout 1.x. Native implementation
classes, source headers, `QMarkdown.Private`, child item trees and helper functions
are private; no C++ headers are installed or C++ ABI compatibility promised.

Use `import QMarkdown 1.14` for the complete current API. Earlier 1.x imports and
unversioned imports remain supported. “Since” below identifies the feature's
introduction, not a promise that every later member is hidden from older imports.
Qt Quick inherited Item properties and methods remain available on MarkdownView.

## MarkdownView

MarkdownView is an Item. Set a positive width; implicit width is zero and implicit
height follows contentHeight. Hosts own scrolling, clipping and backgrounds.

| Property | Type / default | Since | Contract |
| --- | --- | --- | --- |
| `syntaxHighlightingEnabled` | bool / false | 1.11 | Opt-in fenced-code highlighting; see the [syntax guide](syntax-highlighting.md). |
| `syntaxHighlightingAvailable` | readonly bool | 1.11 | Compiled backend availability. |
| `syntaxTheme` | SyntaxTheme / view-owned Light | 1.11 | Replaceable palette; null or external destruction restores default. |
| `keyboardNavigationEnabled` | bool / false | 1.10 | Tab/Shift+Tab traverse drawable logical links without wrapping; hosts retain boundary focus traversal. |
| `keyboardFocusColor` | color / bound to style.linkColor | 1.10 | Two-logical-pixel outline around active link fragments, without layout changes. |
| `taskIndicatorsReplaceBullets` | bool / false | 1.14 | Places unordered task indicators in the marker lane; ordered tasks retain numbers and adjacent indicators. |
| `taskIndicator` | Component / null | 1.10 | Decorative Item with live `checked` and `markdownStyle` inputs; preferred dimensions and fallback follow the [task indicator contract](task-lists.md#host-presentation-110). |
| `searchActive` | bool / false | 1.9 | Enables Find and searchable fenced source. |
| `searchQuery` | string / empty | 1.9 | Non-overlapping literal query over decoded document text. |
| `searchCaseSensitive` | bool / false | 1.9 | Unicode case comparison mode. |
| `searchResultCount` | readonly int / 0 | 1.9 | Zero while inactive or query empty. |
| `currentSearchResult` | readonly int / -1 | 1.9 | Zero-based current result, -1 when absent. |
| `searchHighlightColor`, `currentSearchHighlightColor` | color / #70ffff00, #b0ff9900 | 1.9 | Independent background highlights beneath selection. |
| `headings` | readonly var / empty array | 1.9 | Document-order snapshots of level, title and fragmentId. |
| `markdown` | string / empty | 1.0 | Full source replacement; equal assignments do nothing. |
| `style` | MarkdownStyle / view-owned Legacy style | 1.0 | Shared host styles are borrowed; see lifecycle below. |
| `baseUrl` | url / empty | 1.0 | Explicit base for relative image URLs; no inferred working/component directory. |
| `resourcePolicy` | MarkdownResourcePolicy / view-owned deny-all policy | 1.0 | Shared host policies are borrowed. |
| `contentHeight` | readonly real / 0 | 1.0 | Settled document height; zero at nonpositive width. Layout settles through Qt polish. |
| `fencedBlockRenderers` | var / empty map | 1.8 | Language-to-Component map; replace whole map to update; contract below. |
| `codeBlockActions` | Component / null | 1.2 | Per-code-block host action Item; contract below. |
| `taskListsEnabled` | bool / false | 1.3 | Enables task markers. |
| `strikethroughEnabled` | bool / false | 1.4 | Enables strikethrough. |
| `extendedAutolinksEnabled` | bool / false | 1.5 | Enables extended autolinks. |
| `tablesEnabled` | bool / false | 1.6 | Enables native tables. |
| `tagFilterEnabled` | bool / false | 1.6 | Retains filtered semantic HTML privately; displayed HTML remains literal. |
| `gfmEnabled` | bool / false | 1.6 | Effective extension flags are `gfmEnabled || individualEnabled`; individual values are retained. |
| `selectionViewport` | Item / null | 1.8 | Host visible viewport for desktop selection edge scrolling; null disables requests. |
| `selectionEnabled` | bool / false | 1.7 | Enables desktop document selection; disabling clears it. |
| `selectedText` | readonly string / empty | 1.7 | Selected substring of the plain-text projection. |
| `selectionStart`, `selectionEnd` | readonly int / 0 | 1.7 | Normalized UTF-16 projection offsets, start ≤ end; not Markdown source offsets. |

Changing extension switches reparses the stored source without changing markdown
or its notification. Effective parser-option changes clear selection. Masked flag
changes, resource updates, layout changes and style changes retain selection.

| Method | Since | Behavior |
| --- | --- | --- |
| `searchNext()`, `searchPrevious()` | 1.9 | Return bool; advance with wrapping and request geometry. |
| `navigateToHeading(index)` | 1.9 | Return bool; request outline entry geometry by zero-based index. |
| `navigateToFragment(fragment)` | 1.9 | Return bool; decode a local #fragment once and resolve its exact generated ID. |
| `selectAll()` | 1.7 | Selects the complete projection when enabled. |
| `clearSelection()` | 1.7 | Clears selection and resets both endpoints to zero. |
| `requestCopy()` | 1.7 | Emits copyRequested only when enabled and selection is nonempty. |

| Signal | Since | Host responsibility |
| --- | --- | --- |
| `navigationRequested(rect target)` | 1.9 | Host scrolls/clamps to the target in view coordinates, including refreshed geometry. |
| `linkActivated(string destination)` | 1.0 | Validate, resolve and navigate; the parser-decoded destination is unchanged, including empty strings and custom schemes. |
| `codeBlockCopyRequested(string text)` | 1.2 | Copy exact parser-produced code, including trailing newlines; fences/info strings are excluded. |
| `selectionScrollRequested(real deltaY)` | 1.8 | Host scroll request in logical pixels, positive downward; host clamps. |
| `copyRequested(string text)` | 1.7 | Copy the selected plain text. |

The view never opens links or writes the clipboard. A codeBlockActions Component
must create an Item declaring `required property string codeText` and
`required property var requestCopy`. Calling that callback emits
codeBlockCopyRequested. The library owns each created Item, assigns width, and
reserves finite nonnegative implicitHeight. Component/block replacement or view
removal invalidates old callbacks. Hosts own controls, accessibility, focus and
feedback; the Component creation context must remain valid. See
[code actions](code-block-actions.md) for an example.

`fencedBlockRenderers` selects host Items for fenced code using the first token of
the decoded info string, case-sensitively. Indented/empty/unmatched blocks remain
ordinary code. Hosts declare required string `codeText`, `infoString`, `language`
and required `markdownStyle`; the library supplies them at construction, assigns
width and reserves finite nonnegative implicitHeight. Existing surface/actions
remain. Selection or Find enabled always displays searchable source; copy payloads retain
source. Null/invalid entries and creation failures fall back with diagnostics.
Instances follow document/component/selection lifetime; width/style changes update
them. Components are trusted host code. See [renderer contract and example](fenced-block-renderers.md).

Assigning null/resetting style or resourcePolicy, or destroying a supplied object,
restores the view-owned default. The view does not take ownership of supplied
objects. Default-style reset restores Legacy construction defaults; default-policy
reset restores deny-all. Updates to shared objects affect attached views.

[Selection](selection.md) defines exact copy separators, list markers, table tabs,
image alt text, gesture routing and desktop interaction limits. [GFM](gfm.md)
defines syntax/layout scope. HTML remains inert literal text.

## MarkdownStyle

MarkdownStyle is a shareable QObject. Typography snapshots application defaults at
construction: body uses the application font with Normal weight; code blocks use
that font with the system fixed-font family; inline code specifies only that family
and inherits other font attributes. Heading fonts use body size multipliers
2, 1.75, 1.5, 1.25, 1.135, 1 with Bold weight. Fonts support point or pixel sizes;
all geometric metrics use logical pixels.

| Property | Legacy default | Since |
| --- | --- | --- |
| `bodyFont`, `h1Font` … `h6Font`, `codeBlockFont`, `inlineCodeFont` | Construction fonts above | 1.0 |
| `bodyColor`, `h1Color` … `h6Color`, `codeBlockColor` | `#202020` | 1.0 |
| `linkColor`, `linkUnderline` | `#0066cc`, true | 1.0 |
| `thematicBreakColor`, `thematicBreakThickness` | `#202020`, 1 | 1.0 |
| `blockSpacing`, `listIndent`, `quoteIndent` | 8, 24, 16 | 1.0 |
| `quoteRuleColor`, `quoteRuleThickness` | `#808080`, 2 | 1.0 |
| `preset` | `MarkdownStyle.Legacy` | 1.1 |
| `selectionBackgroundColor` | `#603b82f6` | 1.7 |

Preset enum values are Legacy, PolishedLight, PolishedDark, MinimalLight and
MinimalDark. Dark palettes use `#e6e6e6` text, `#8db9f2` links, `#454b54` rules,
`#a0a0a0` quote rules and `#7078a9ef` selection. Non-Legacy light rules use
`#d0d7de`. Minimal retains construction metrics and disables decorations. Polished
uses block spacing 12, H1/H2 separators (thickness 1, spacing 8), inline-code radius
4, and code-block padding 12, border 1 and radius 6. Polished code backgrounds are
`#eff1f3` light / `#30343b` dark. Presets do not provide a document background.

Groups are readonly permanent QObject properties owned by their MarkdownStyle;
the following members are writable and resettable:

| Group | Members and Legacy defaults | Since |
| --- | --- | --- |
| `headings.h1` … `headings.h6` | `font`, `color` as above; `separatorVisible`: false; `separatorColor`: transparent; `separatorThickness`, `separatorSpacing`: 0 | 1.1 |
| `inlineCode` | `font` as above; `foreground`: invalid/inherited; `backgroundColor`: transparent; `cornerRadius`: 0 | 1.1 |
| `inlineCode` | `horizontalPadding`: 4; `verticalPadding`: 2, in every preset | 1.6 |
| `codeBlock` | `font`, `color` as above; `backgroundColor`, `borderColor`: transparent; `borderWidth`, `cornerRadius`, `horizontalPadding`, `verticalPadding`: 0 | 1.1 |
| `table` | `headerForeground`: invalid/inherits bodyColor; `headerBackground`, `bodyBackground`, `borderColor`: transparent; `borderWidth`: 0; `horizontalPadding`: 4; `verticalPadding`: 2, in every preset; `borderMode`: TableBorderGrid; `alternateRowBackground`: inherits bodyBackground | 1.6; new controls 1.14 |

Flat heading/code font and color properties alias their grouped equivalents. Both
spellings notify when the value changes; bind only one spelling for each value.
Ordinary style properties provide corresponding `propertyNameChanged` signals.
Group identities are constant. Assignments establish whole-property overrides even
when equal to defaults. Font overrides retain resolve masks. Changing preset
preserves overrides. `resetOverrides()` (1.1) clears all overrides and applies the
selected preset. Assigning undefined to a grouped property or
selectionBackgroundColor resets just that property. Legacy flat properties have
no individual RESET; use their grouped aliases where available or resetOverrides.
Resets preserve host bindings, which can later reapply values.

Inline foreground inherits containing text when invalid; link color takes
precedence for linked code. Invalid table header foreground inherits bodyColor. See [table presentation](styling.md#table-presentation-qml-114) for border modes, alternating backgrounds, reset behavior and Qt’s grouped-property revision limitation.
Rendering sanitizes metrics; modern decoration/padding values clamp negative or
nonfinite values to zero. Legacy spacing/indent/rule metrics retain their documented
fallback defaults for nonfinite values. Stored style values remain the assigned
values. See [styling](styling.md) for examples and geometry details.

## MarkdownResourcePolicy

All members were available in 1.0. Changes emit the shared `changed()` signal.

| Property | Type / default | Authorization |
| --- | --- | --- |
| `allowedFileRoots` | list of url / empty | Existing absolute directory URLs with empty/localhost authority; canonical paths constrain traversal and symlinks. |
| `allowedHttpsOrigins` | list of url / empty | HTTPS origins only; no credentials, queries, fragments or non-root paths. Every redirect is checked. |
| `allowQrc` | bool / false | Allows application qrc image resources. |

There are no public QML methods or per-property RESET operations. Assign empty
lists/false to revoke permissions, or reset the view's resourcePolicy. Base URL or
policy changes cancel resource work and discard cached images. Missing, denied or
failed images keep their alt text. Images are separate native rows, PNG/JPEG only;
HTML never triggers resource requests. Limits are four fetches per view, two shared
decoder workers, 15 seconds per image including queue waiting, five redirects,
8 MiB encoded bytes, 16 megapixels per image and 64 MiB retained decoded images.
Running codecs cannot be interrupted; shutdown waits for active workers. The
network manager is private and cannot be replaced through the public API.

## CMake integration and deployment

Supported environment: Linux amd64, Qt ≥6.8, C++17, CMake ≥3.21.

```cmake
find_package(QMarkdown 1.14 CONFIG REQUIRED)
target_link_libraries(myapp PRIVATE QMarkdown::QMarkdown)
```

The package finds Qt Core, Gui, Qml, Quick and Network. Set the application's QML
import path to exported `QMarkdown_QML_IMPORT_PATH` as needed; deploy the backing
library, plugin, qmldir, qmltypes and QML/JS implementation files together. For static
QMarkdown additionally link `QMarkdown::QMarkdownPlugin` and use
`Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)` in one C++ translation unit after including
`<QtQml/QQmlExtensionPlugin>`. Static registration embeds module resources.

SameMajorVersion accepts compatible requests no newer than the installed 1.x
version; EXACT requires the actual installed version. Source build options are
BUILD_SHARED_LIBS (ON), BUILD_TESTING (OFF), QMARKDOWN_BUILD_EXAMPLES (OFF),
QMARKDOWN_BUILD_BENCHMARKS (OFF, requires tests), and QMARKDOWN_QML_INSTALL_DIR
(default `${CMAKE_INSTALL_LIBDIR}/qt6/qml`). See [installation](release-install.md)
and [source build](source-install.md). Hosts own Qt runtime deployment.

[Reading and navigation](reading-navigation.md) defines Find lifecycle, heading IDs,
fragment decoding and deferred host navigation requests.

`SyntaxTheme` (since 1.11) provides Light/Dark presets and resettable keyword,
type, function, variable, builtIn, string, number, comment, preprocessor, operator,
constant and error color properties. Explicit overrides survive preset changes;
live edits recolor existing source without tokenization. See the
[palette and fallback contract](syntax-highlighting.md).

## Code-block source layout

See [code-block usability](code-blocks.md) for the view-wide wrapping, optional
source-line numbers, bounded source height, internal scrollbars and extended
language-aware `codeBlockActions` contract. Defaults preserve wrapping and natural
height; line numbers and height limiting default off. Headers and clipboard writes
remain host-owned.

## Native block math (1.13)

`MarkdownView.mathEnabled` defaults to false. Read-only `mathAvailable` reports
compiled backend availability. Exact first info token `math` selects native fenced
rendering after host renderers. Standalone `$$` delimiter lines are parsed only
when enabled. Active Find shows literal TeX body text. Since 1.14,
selection keeps eligible native formulas rendered as atomic objects.
`MarkdownStyle.math.color` inherits `bodyColor`; reset restores inheritance.
`math.scale` defaults to 1.0 relative to resolved body text size; nonpositive or
nonfinite values reset to 1. Bundled mathematical fonts are independent of the
host body family. See the [dialect and limits](math.md).

## Atomic native math selection (1.14)

With selection enabled, eligible native formulas remain rendered and select as complete objects. Find shows TeX source and preserves exact selection endpoints when closed. Copy uses the existing body projection. Registered host `math` renderers retain source fallback during selection and Find. No public API changes.

## Development inline math (1.15)

`MarkdownView.inlineMathEnabled: false` independently opts into single-line dollar
and backslash-parenthesis formulas. `mathEnabled` continues controlling block math.
`mathAvailable` and `style.math` serve both. Inline rendering uses surrounding font
size, baseline atoms and wrappable body fallback; Find shows body source and copy
omits delimiters. See [inline math](math.md#inline-math-development-qml-115).
Package 1.15.0 accepts compatible 1.0–1.15 imports; published 1.14 remains unchanged.

## Development footnotes (1.16)

`MarkdownView.footnotesEnabled: false` independently enables Markdown footnote
references and definitions. Forward/repeated references share native numbered
endnotes; superscript references and decorative backlinks use existing host link
activation, fragment navigation, keyboard traversal and accessibility. Copy/Find
use `[n]` and numbered bodies. See the [footnote guide](footnotes.md).
Package 1.16.0 accepts compatible 1.0–1.16 imports; published 1.14 remains unchanged.
