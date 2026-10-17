# Design — Standalone display math

Approval: requirements, design and tasks approved by the user response “approved” on 2026-10-16. Dependency audit and verification remain implementation tasks; no success is implied. One cohesive 1.13.0 / QML 1.13 release PR is planned; merge, tagging and publication require separate authorization.

Release delivery approval — 2026-10-17: the user authorized the release plan for PR #36, including documentation completion, fresh final-head Qt 6.8.0/6.12.0 CI and retained full gate, squash merge through normal protections, `release.yml` dispatch with `release_pr=36`, tagging `v1.13.0`, stable/latest publication and downloaded hash verification of exactly four assets. Retain package 1.13.0 / QML 1.13 and existing APIs; no runtime changes. Stop on failed gates, changed inputs or differing published bytes. Historical approvals and results below remain unchanged; this approval supersedes earlier statements that delivery still requires authorization.

Extend the vendored private cmark block parser, its prefixed symbols/options and document adapter. Add a private display-math node/block kind without renumbering existing semantic fixture identifiers, and a ParseOptions flag driven by mathEnabled. Do not replace source with synthetic fences or scan a completed inline AST.

At a block-start position with no open paragraph, inspect a delimiter using the
parser's consumed container prefixes and indentation. Literal code/HTML handling
takes precedence. A first block parse tentatively treats delimiters as a literal
fenced candidate; only explicit closure in the same container marks a math node.
Collect matched opening line numbers in a linear-size bitmap, then perform a
second ordinary block parse that permits only those opening lines. This avoids
source rewriting and suffix lookahead, and restores unmatched input to ordinary
Markdown paragraph/list/code rules. Container continuation and closing use the
existing block parser. Blank body lines and Markdown punctuation remain literal.
Each pass is linear in source/container parsing work; no per-opener suffix scan.
The default-disabled path remains the existing single parse.

Adapt the node to a new private block kind with literal body text and no inline ranges. Extend view-state presentation and selection/search projections so only body text participates, with ordinary inter-block separators. BlockSequence presents a decoration-free formula viewport from feature 055, or the ordinary selectable literal source surface in source mode. Standalone blocks do not enter the fenced renderer registry because its public contract is specifically for fences. Both forms share rendering, style, limits and fallback, while preserving their distinct decorations.

Backend OFF plus mathEnabled true still recognizes standalone syntax and projects the body as source. mathEnabled false retains the original Markdown parse. Reparse invalidates pending formula generations and rebuilds projections using existing document replacement behavior. Selection-enabled and active-Find presentation changes do not need to change parsing.
