# Requirements — 1.9 reading and navigation

Status: **approved on 2026-10-15** by the user response “approved”.
Approval covers requirements R1–R9, design and tasks T1–T6 implementation and
verification. Merge, tagging and publication require separate authorization.

## Scope

- R1: Deliver package 1.9.0 / QML 1.9 with additive 1.x-compatible QML interfaces. Keep native Qt Quick rendering, private parser/model classes and no public C++ API or new dependencies. Hosts own controls, scrolling, navigation, clipboard and visual identity.
- R2: Provide opt-in literal Find, case-insensitive by default, with query and case state, result count, current result, highlights and wrapping next/previous actions. Search the existing decoded plain-text projection, including formatted text, tables, code and image descriptions. Empty queries have no results. Matches are non-overlapping in document order.
- R3: Distinguish the current match without changing layout, selected text or clipboard behavior. While Find is active, show searchable source instead of custom fenced visuals; restore eligible visuals when Find closes, subject to existing selection mode.
- R4: Expose read-only H1–H6 entries in document order, including headings nested in lists/quotes. Entries contain level, decoded title and unique generated fragment ID. Allow explicit heading-location requests.
- R5: Generate IDs by lowercasing decoded heading text, removing Unicode punctuation, replacing whitespace runs with hyphens and using `section` for an empty result. Allocate numeric suffixes starting at 1 and skip all IDs already allocated. No GitHub slug compatibility is claimed.
- R6: Resolve explicit local fragments beginning with `#`, percent-decoding once and matching exact generated IDs. Missing/invalid targets return failure without navigation. Preserve `linkActivated(destination)` unchanged; hosts decide whether to resolve fragments or handle external links.
- R7: Expose successful target rectangles in MarkdownView coordinates through one shared navigation request signal. Never scroll internally. Source/effective parser-option changes rebuild headings/results and reset the current match; layout/resource changes preserve logical targets and refresh geometry.
- R8: Add a host-owned playground Find bar (query, case toggle, count, previous/next), collapsible outline and local links. Demonstrate Ctrl+F, Enter/Shift+Enter and Escape, including host clamping of scroll positions.
- R9: Deliver implementation, tests, public documentation, 1.9 release notes, changelog and compatible package/QML metadata together. Verify configured shared/static Qt targets, installed consumers, conformance regressions and the release documentation gate; record actual evidence separately.

## Exclusions

Regex, replacement, keyboard link traversal, caret navigation, streaming, bundled renderer engines and additional platforms are deferred. Merge, tagging and publication require separate authorization.

## Release authorization — 2026-10-15

The user instruction “release it” additionally authorizes the release PR, protected
squash merge, tagging and publication of 1.9.0 after required hosted checks and
distribution checks pass. This supersedes the earlier delivery exclusions above.
Publication completion requires downloaded-asset hash verification. Final hosted
verification and release outcomes are retained in GitHub Actions and the release.
