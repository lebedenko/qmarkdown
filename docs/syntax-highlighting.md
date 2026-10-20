# Syntax highlighting (QML 1.11)

Built-in highlighting is library-owned and opt-in per view. Hosts control token colors
independently from Markdown typography and code-block container styling.

```qml
import QMarkdown 1.11

MarkdownView {
    markdown: source
    syntaxHighlightingEnabled: true
    syntaxTheme: SyntaxTheme {
        preset: SyntaxTheme.Dark
        keywordColor: "#c4b5fd"
    }
}
```

`syntaxHighlightingAvailable` reports compiled backend availability. Both APIs and
`SyntaxTheme` exist with the backend disabled. Fenced blocks with an empty or
unknown language and indented code remain plain without warnings. The existing
first info-string token is resolved case-insensitively, including C/C++, JS/TS,
Python, QML, Bash and JSON aliases, installed definition names and extensions.
Source content never determines language. Custom renderers still match
case-sensitively and take precedence. Selection and Find switch those visuals to
source, which can then be highlighted. Copy actions preserve exact parser source.

`syntaxTheme` is replaceable. Null assignment or destruction of an externally
owned theme restores the view-owned Light default. Preset changes preserve
explicit color overrides; assigning `undefined` resets a color. Live edits apply
to existing blocks without tokenizing again. Colors are `keywordColor`,
`typeColor`, `functionColor`, `variableColor`, `builtInColor`, `stringColor`,
`numberColor`, `commentColor`, `preprocessorColor`, `operatorColor`,
`constantColor` and `errorColor`.

| Roles | Light | Dark |
| --- | --- | --- |
| Keyword, preprocessor | `#7c3aed` | `#c4b5fd` |
| Type, built-in | `#0f766e` | `#5eead4` |
| Function | `#1d4ed8` | `#93c5fd` |
| Variable, constant | `#9a3412` | `#fdba74` |
| String | `#15803d` | `#86efac` |
| Number | `#b45309` | `#fcd34d` |
| Comment | `#64748b` | `#94a3b8` |
| Error | `#b91c1c` | `#fca5a5` |

Operator, normal and unmapped tokens retain `style.codeBlockColor` by default.
Code-block fonts, background, padding and borders remain in `MarkdownStyle`.
Token emphasis/fonts and arbitrary backend theme files are excluded.

Source appears immediately. One application worker processes complete blocks,
with a theme-independent LRU capped at 128 entries / 8 MiB including source and
spans. Replacement, disablement and delegate destruction invalidate subscriptions;
view destruction never waits for work. Whole-block plain fallback applies above
256 Ki UTF-16 units, 16 Ki per line, 65,536 spans, or a 100 ms cooperative processing
budget. Cancellation/time are checked between lines and in format callbacks; an
individual backend call cannot be forcibly interrupted. These are private limits.
The renderer omits one terminal newline for display only.

Source builds default `QMARKDOWN_WITH_KSYNTAXHIGHLIGHTING=OFF`. Enable it with
`-DQMARKDOWN_WITH_KSYNTAXHIGHLIGHTING=ON` and install KF6SyntaxHighlighting >=6.10.0;
explicit enablement fails configuration if the dependency is absent. Arch packages
enable it and depend on `syntax-highlighting>=6.10.0`. Enabled static consumers
also need that dependency's CMake package. Disabled builds never discover or link KDE.

KSyntaxHighlighting is linked externally; QMarkdown does not vendor syntax
resources. Its library code is predominantly MIT, with separately licensed
resources recorded in upstream SPDX metadata. Distribution of the dependency
must retain its applicable notices. CI builds the official KDE Frameworks 6.10.0
archives for KSyntaxHighlighting and Extra CMake Modules against each selected Qt
SDK, verifying pinned SHA-256 values and recording URLs/hashes/Qt provenance.
That release declares Qt 6.6.0 support, retaining QMarkdown's Qt 6.8 minimum.
See [upstream source and licensing](https://invent.kde.org/frameworks/syntax-highlighting/-/tree/v6.10.0)
and the [AbstractHighlighter contract](https://api.kde.org/ksyntaxhighlighting-abstracthighlighter.html).

Pinned CI archive SHA-256 values:

- `extra-cmake-modules-6.10.0.tar.xz`: `506989a0d400913403e669c1912238db053cd6b38dff74b17e2e6f879c79cca0`
- `syntax-highlighting-6.10.0.tar.xz`: `b5b5e343ff27bc5c95be0051d5606dfcb3295f835830e7fc6dac8d2863891699`

Qt 6.12 introduces an `_sv` literal that collides with the 6.10 build-time indexer's
local literal. CI renames that literal in the indexer only, guarded by its original
file hash, and records both file hashes in provenance. Library code and definitions
remain the pinned upstream versions.

In the playground, open **Style controls** for Syntax colors and Dark syntax palette,
then select the Syntax highlighting sample.


Run `task demo-highlighted` for a separate backend-enabled playground build. KF6SyntaxHighlighting must already be installed; CMake reports a missing dependency normally. Style controls display availability and disable both syntax controls when unavailable. Sample selection never enables highlighting or changes the palette.

## Host providers (QML 1.17)

Assign a host-owned `SyntaxHighlightingProvider` to `syntaxHighlightingProvider`
and enable `syntaxHighlightingEnabled`. The provider works with the optional
built-in backend disabled; `syntaxHighlightingAvailable` still reports only that
backend. A provider may be shared by several views and is never reparented.

```qml
import QMarkdown 1.17

MarkdownView {
    syntaxHighlightingEnabled: true
    syntaxHighlightingProvider: SyntaxHighlightingProvider {
        onHighlightRequested: request => {
            if (request.language !== "my-language") { request.decline(); return }
            // Hosts may retain request and complete it later on the QML thread.
            // Check request.cancelled before delivering asynchronous results.
            request.complete([{ start: 0, length: 3,
                                role: SyntaxHighlightingProvider.Keyword }])
        }
    }
}
```

The example assumes the language's source begins with a three-unit keyword.
Use offsets into the exact read-only `request.source`, measured in UTF-16 units
(JavaScript string indices), and the exact case-sensitive `request.language`.
Empty source, empty languages and indented code submit no requests. Eligible
fenced sources include languages unknown to the built-in backend and code inside
footnotes. Custom fenced visuals take precedence; their selection/Find source
fallback can request tokens. Source, copying, Find and layout typography remain
library-owned.

`complete(spans)` accepts an array of `{start, length, role}` objects. Fields must
be finite integer numbers; length must be positive, ranges in bounds and
nonoverlapping, and neither boundary may split a surrogate pair. Valid spans are
sorted. Any invalid span rejects the entire result. The role enums, in palette
order, are `Keyword`, `Type`, `Function`, `Variable`, `BuiltIn`, `String`, `Number`,
`Comment`, `Preprocessor`, `Operator`, `Constant` and `Error`. Providers supply
roles; `SyntaxTheme` supplies colors, including live changes without new requests.

`complete([])` deliberately leaves source plain and suppresses built-in fallback.
`decline()`, invalid results or an unanswered five-second deadline use the built-in
backend, or plain source when it is unavailable. The deadline also applies if host
JavaScript blocks timer delivery. Duplicate and stale completions are ignored.
The 256 Ki source-unit, 16 Ki line-unit and 65,536-span limits apply before work
or result acceptance. Host callbacks run on the QML thread; hosts arrange their
own asynchronous execution and return results on that thread.

Replacement, disablement and delegate/view destruction cancel subscriptions.
Provider destruction restores built-in/plain behavior for surviving views.
`request.cancelled` becomes true when no subscribers remain, on invalidation or
on timeout; a host-retained request remains readable for late asynchronous work.
Identical active requests on a shared provider can be coalesced, so cancelling one
view does not cancel the others. Hosts should release retained requests after
completion or cancellation.

Successful results, including empty success, use a provider-local 128-entry /
8 MiB cache keyed by exact source and language. Declined, invalid and timed-out
decisions are not cached. `provider.invalidate()` clears its cache, cancels pending
work and requests fresh tokens for active sources. Provider objects and invalidation
generations never share host results. The playground's Host highlighting sample
uses a deterministic asynchronous provider without requiring KDE.
