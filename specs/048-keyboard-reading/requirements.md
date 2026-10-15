# Requirements — keyboard reading and link accessibility

Status: **approved on 2026-10-15** by the user response “approved”.
Approval covers this specification, design, tasks, implementation and verification. Target: package 1.10.0 / QML 1.10.

## Scope

- R1: Add `MarkdownView.keyboardNavigationEnabled`, default false, and enable it
  in the playground. Plain Tab and Shift+Tab visit drawable native document links
  in document order, entering from either direction. Traversal never wraps; at
  boundaries and in documents without links, normal host Qt focus traversal
  continues. Other modifiers retain existing host/selection behavior.
- R2: Enter/Return on a focused link emits `linkActivated(destination)` once per
  key press, ignoring auto-repeat. Traversal never activates links or changes
  selection. Pointer handling, selection shortcuts and Find remain available.
  Hosts own destination handling, including fragment resolution.
- R3: Add `keyboardFocusColor`, bound by default to `style.linkColor`. Show a
  two-logical-pixel outline around every visible fragment of the active link,
  without changing layout. Losing active focus hides the outline. Request
  visibility through existing `navigationRequested(rect target)` in view-local
  coordinates; hosts retain scrolling ownership.
- R4: One logical link is one traversal stop, including links split across text
  and loaded images. Distinct links with identical destinations remain distinct.
  Cover headings, recursive lists/quotes, table cells, autolinks and linked images.
  Links inside image descriptions are inert. Links without drawable text or image
  geometry have no focus stop. Host action controls retain normal focus order;
  custom renderer internals retain host-owned behavior.
- R5: Expose one accessible Link node per drawable logical link regardless of
  keyboard opt-in. Provide decoded text/image-description name, destination,
  global geometry, focused state and activation action. Use destination as name
  when the decoded label is empty. Accessible activation emits the same host link
  signal. Disabling keyboard navigation removes Tab stops, not accessible links.
- R6: Preserve identity across resizing, style changes and image loading. Source
  and effective parser-option changes invalidate old targets and callbacks.
  Disabling navigation clears keyboard link focus. Replacement or destruction
  while focused must not activate stale destinations or leave dangling objects.
- R7: Preserve historical QML imports and compatible 1.x CMake requests. Deliver
  implementation, tests, playground examples, API/reader documentation and actual
  verification in one focused PR. Prepare complete 1.10 release notes,
  compatibility guidance and current summaries through the documentation gate.
  Preserve historical evidence and distinguish released 1.9 from development work.

Streaming, highlighting, Mermaid/math engines, footnotes, caret navigation, touch
selection handles and further platforms are excluded. Features 046 and 047 are
already implemented development work, not new implementation tasks here. Merge,
tagging and publication require separate authorization.

## Release delivery authorization

The supplied “Update documentation and publish QMarkdown 1.10.0” plan explicitly
authorizes documentation reconciliation, one release PR, squash merge, workflow
dispatch, tagging and stable/latest publication with four source/Arch assets and
downloaded-asset SHA-256 verification. This extends the historical implementation
approval above without changing runtime scope, package 1.10.0 or QML 1.10.
Follow the [development workflow](../../docs/development-workflow.md); stop on a
failed stage and preserve existing tags and published bytes.

## Approved publication recovery and tooling repair

The supplied “Recover QMarkdown 1.10.0 and harden publication” plan authorizes
recovery first, followed by one tooling fix PR and squash merge after final-head
configured CI passes. Preserve v1.10.0, commit efb2d35 and checked distribution
bytes. No runtime, public API, package/QML version, dependency or workflow-entrypoint
changes are included.

- P1: Retry only the failed publication job after checking unchanged main/tag,
  final PR gate, retained artifacts, existing draft and the UTC changelog date.
  Stop release writes if recovery fails or the date differs.
- P2: Create drafts through REST and retain the numeric ID. Discover existing
  releases by paginated exact-tag match, rejecting duplicates. Use that ID and
  its upload URL throughout upload, publication and verification.
- P3: Validate ID, tag and stable state on every refresh. Retry delayed visibility
  and transient read failures six times with 1, 2, 4, 8 and 16 second delays and
  cache revalidation. Identity changes, unexpected assets and differing bytes
  fail immediately. Keep eligibility checks before writes and publication.
- P4: After uncertain uploads, refresh and compare existing bytes before retrying;
  never overwrite assets or blindly retry draft creation. Completion requires
  refreshed published state and all four downloaded SHA-256 comparisons.
