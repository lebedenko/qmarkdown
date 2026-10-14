# Styling in QMarkdown

Use `import QtQuick` and `import QMarkdown 1.1` for the new API. The published 1.2.0 package accepts compatible 1.0/1.1/1.2 CMake requests and EXACT 1.2.0; exact earlier versions require historical packages. Existing QML 1.0 consumers continue to work with Legacy defaults.

`MarkdownStyle.preset` defaults to `MarkdownStyle.Legacy`. Additional presets are `PolishedLight`, `PolishedDark`, `MinimalLight`, and `MinimalDark`. These are fixed palettes, with no application-theme subscription or document background. The host owns backgrounds, scrolling, navigation, and resource authorization.

| Palette | Light | Dark |
| --- | --- | --- |
| Body, heading, fenced/indented code text | `#202020` | `#e6e6e6` |
| Links | `#0066cc` | `#8db9f2` |
| Separators, code borders, thematic breaks | `#d0d7de` | `#454b54` |
| Quote rules | `#808080` | `#a0a0a0` |
| Polished code backgrounds | `#eff1f3` | `#30343b` |

Legacy retains construction typography. Decorations default to disabled/transparent with zero metrics, except inline-code padding: 1.6 intentionally adds 4/2 logical pixels in every preset. Minimal keeps construction typography, block spacing, list and quote metrics, and disables new decorations. Polished keeps construction typography/list/quote metrics, sets block spacing to 12, H1/H2 separators to thickness 1 and spacing 8, inline code radius to 4, code padding to 12 in both directions, border width to 1 and radius to 6. H3–H6 separators remain disabled.

Grouped objects are permanent and owned by their MarkdownStyle. They cannot be replaced.

| Group | Writable properties |
| --- | --- |
| `headings.h1` through `headings.h6` | `font`, `color`, `separatorVisible`, `separatorColor`, `separatorThickness`, `separatorSpacing` |
| `inlineCode` | `font`, `foreground`, `backgroundColor`, `cornerRadius` |
| `table` (QML 1.6) | `headerForeground`, `headerBackground`, `bodyBackground`, `borderColor`, `borderWidth`, `horizontalPadding`, `verticalPadding` |
| `codeBlock` | `font`, `color`, `backgroundColor`, `borderColor`, `borderWidth`, `cornerRadius`, `horizontalPadding`, `verticalPadding` |

Existing `h1Font`/`h1Color` through `h6Font`/`h6Color`, `inlineCodeFont`, `codeBlockFont`, and `codeBlockColor` are aliases for the corresponding grouped values. Both paths notify when the effective value changes. Bind through only one spelling of each aliased property; competing bindings on both spellings can overwrite one another.

Assignments create explicit whole-property overrides, even when assigning the current/default value, zero, false, or transparency. Preset changes update untouched properties and retain overrides, independent of QML declaration order. Font edits preserve existing QFont resolve-mask behavior: an inline-code family-only font inherits the containing text's size, weight and other unset attributes. Editing a font subproperty makes the resulting whole font an override; setting a preset never rescales host fonts.

`inlineCode.foreground` inherits containing text by default. An explicit color replaces it; linked code still uses the link color. Reset foreground to restore inheritance. Inline backgrounds follow visual character runs across wrapped and bidirectional lines; they reserve horizontal padding around each wrapped visual fragment and increase line height by twice the vertical padding on code-bearing lines. Defaults are 4/2 logical pixels in every preset, including Legacy and Minimal. Semantic text and UTF-16 ranges stay unchanged; linked padding activates the enclosing link. Transparent backgrounds still reserve space. Set `style.inlineCode.horizontalPadding` and `verticalPadding` to zero to restore previous spacing.

Set a grouped property to `undefined` to invoke its RESET and clear only that override:

```qml
style.codeBlock.horizontalPadding = undefined
style.headings.h1.color = undefined
```

`style.resetOverrides()` clears all overrides and restores the selected preset's construction-based values. It updates native state without removing host QML bindings; those bindings can update the property again when their inputs change. The library's private default-style lifecycle resets to Legacy construction snapshots. Reading properties and switching the selector creates no overrides.

Heading separators appear once after the complete heading, including headings split into text and authorized image rows. Fenced and indented code remain literal native Text inside native surfaces, including empty blocks. New decoration metrics render nonnegative; NaN and infinities render as zero. Horizontal code padding clamps to leave at least one text pixel at positive widths; indivisible glyphs can still overhang very narrow views. Borders do not add implicit padding. [Host-controlled code-block copy actions](code-block-actions.md) are available in 1.2. Document-wide selection, inline borders, highlighting and arbitrary style inheritance remain deferred.

The playground's Neutral/Reset controls restore Legacy and host theme bindings; Alternate starts from that baseline and adds host overrides. The host theme controls use explicit bindings. Selecting a library preset preserves those host overrides; use “Reset overrides to selected preset” to preview the preset palette. The host preview surface remains independent of the preset.

QML 1.6 adds [table styling](gfm.md): inherited header foreground, bold headers, transparent surfaces/borders, zero border width, and 4/2 logical-pixel cell padding in every preset. Inline-code padding uses Qt paragraph shaping with cached geometry for painting and hit testing. This intentionally changes default wrapping and line height in 1.6. Negative/nonfinite padding renders as zero; grouped style values follow existing override/reset conventions. At widths smaller than an indivisible shaped glyph plus its padding, that fragment may overhang.

QML 1.7 adds `selectionBackgroundColor`, default `#603b82f6` (light/Legacy) or `#7078a9ef` (dark presets). It uses the same explicit override and `resetOverrides()` conventions and supports property reset. Highlights preserve text foregrounds and layout. See [document selection](selection.md).
