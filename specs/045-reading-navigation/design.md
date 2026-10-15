# Design — 1.9 reading and navigation

Status: **approved on 2026-10-15** by the user response “approved”.
Approval covers requirements R1–R9, design and tasks T1–T6 implementation and
verification. Merge, tagging and publication require separate authorization.

## QML contract

MarkdownView exposes these additive interfaces. The [reader guide](../../docs/reading-navigation.md) documents their public behavior:

| Interface | Contract |
| --- | --- |
| `searchActive: bool` | Defaults false; controls highlights and searchable fenced source independently of selection |
| `searchQuery: string` | Defaults empty; retained when Find closes |
| `searchCaseSensitive: bool` | Defaults false; Qt Unicode case-insensitive comparison when false |
| `searchResultCount: readonly int` | Zero while inactive or query empty |
| `currentSearchResult: readonly int` | Zero-based index; -1 when no active match |
| `searchNext()` / `searchPrevious()` | Return bool; wrap and request current-match geometry on success |
| `headings: readonly var` | Snapshot array of `{level, title, fragmentId}`; index is document order |
| `navigateToHeading(index)` | Return bool; invalid indices fail without a signal |
| `navigateToFragment(fragment)` | Return bool; accepts a leading `#`, decodes its suffix once, exact case-sensitive ID lookup |
| `navigationRequested(rect target)` | Target bounding rectangle in view coordinates; host scrolls and clamps |
| `searchHighlightColor` / `currentSearchHighlightColor` | Host-settable view colors; defaults translucent yellow/orange |

Activating Find or changing query/case computes non-overlapping literal matches using QString search over the existing projection. Select the first result automatically (or -1); next/previous then advance with wrapping. Closing Find clears active results/highlights, retains query/case and cancels a pending search target. Source or effective parser options reset to the first result when active. Matches may span projection separators; highlight drawable portions and navigate to their bounding rectangle. Projection-only characters have no painted rectangle; use the nearest associated block geometry for navigation.

## Private state and projection

Extend private ViewState to own match offsets, outline snapshots, fragment lookup and the current logical navigation target. Reuse projectDocument offsets before projectImages splits presentation blocks. Collect headings recursively from this unsplit projection, retaining private offsets and lengths; expose only the documented entry fields. Rebuild logical state on semantic changes, not resource reloads. Do not route Find through selection state or mutate selection offsets.

For slug allocation, iterate Unicode code points, lowercase decoded titles with QString Unicode casing, remove punctuation categories, collapse whitespace to a single hyphen and trim leading/trailing whitespace-derived hyphens. Preserve symbols, letters, digits and combining marks. Use `section` if empty. Allocate each heading in document order: try the base, then base-1, base-2, etc. against the set of every previously allocated ID. Literal suffix-bearing headings use the same allocator. Do not reserve future headings' natural bases. For example `Foo`, `Foo`, `Foo 1` become `foo`, `foo-1`, `foo-1-1`.

Fragment resolution strips only the first leading `#`; percent-decodes the remainder once as UTF-8 using Qt's URL decoding utility. Plus remains plus; do not normalize case, whitespace or decode again. Empty and unmatched suffixes fail. The resolver never activates external navigation or changes the source.

## Drawing and geometry

Reuse FormattedText cursor/layout geometry to map independent search ranges to paint rectangles, including padded inline code and bidirectional/wrapped text. Paint ordinary matches then the current match, with selection painted above search; do not add format ranges that affect font metrics. Add range-rectangle support privately rather than duplicating text layout. Register searchable text/image/block geometry regardless of selectionEnabled; retain separate selection hit-testing eligibility. Use image item rectangles for matching image descriptions. Table cells, nested containers and fenced source participate through existing projection offsets.

Select searchable native text delegates whenever selectionEnabled or searchActive requires source geometry. Keep actual selection handling conditional on selectionEnabled. CodeSurface suppresses custom dispatch for either mode and restores it only when both are false. Keep code actions and payloads unchanged.

Map geometry into MarkdownView coordinates and union drawable target portions. Queue/coalesce geometry requests until delegates and text layout settle; a valid logical request may return true before its signal. Track only the most recently requested heading or match. On resize, style/font, image loading, renderer transitions or presentation rebuild, refresh that target once geometry is ready. Clear it on source/effective parser-option changes, invalid requests or Find closure for search targets. Never emit stale geometry from an earlier document. Zero-width/unlaid-out views defer emission until geometry is available. Hosts may choose whether refreshed requests scroll again.

## Example and delivery

Extend existing viewer controls and Samples.qml with nested/duplicate headings and fragment links. Route local links explicitly through navigateToFragment; retain existing external-link policy. Own Find focus/shortcuts in the viewer and handle navigationRequested by mapping the rectangle into the scroll content and clamping to content bounds.

Keep one meaningful change containing source, focused regression tests, installed-consumer API coverage, docs and metadata. Package/QML versions and current release documentation are updated together during implementation. Follow the [development workflow](../../docs/development-workflow.md); final hosted gates and separately authorized merge/publication are delivery steps.

## Release authorization — 2026-10-15

The user instruction “release it” additionally authorizes the release PR, protected
squash merge, tagging and publication of 1.9.0 after required hosted checks and
distribution checks pass. This supersedes the earlier delivery exclusions above.
Publication completion requires downloaded-asset hash verification. Final hosted
verification and release outcomes are retained in GitHub Actions and the release.
