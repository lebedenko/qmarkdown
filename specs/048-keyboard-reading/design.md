# Design — keyboard reading and link accessibility

Status: **approved on 2026-10-15** by the user response “approved”, covering
requirements, design, tasks, implementation and verification. Merge and publication
remain separately authorized.

## Logical identity and lifetime

Extend private presentation metadata with a document-order logical-link ID and a
catalog containing destination and decoded accessible label. Assign IDs during
source-to-presentation projection, before `projectImages`; traverse recursive
blocks and table cells in source order. Keep IDs on clipped LinkSpan fragments and
linked ImageSpan/PresentationBlock metadata. Do not use destination as identity.
Build labels from semantic inline text, including image descriptions; descendants
inside image descriptions cannot create separate links. Empty labels fall back to
destination. The source AST remains private and parser semantics stay unchanged.

A private coordinator owned by ViewState holds the catalog and a monotonically
advancing document generation. Rebuild it only for changed source or effective
parse options. Resource projection and style/layout changes update geometry for
the same IDs. Every activation validates generation and ID against the current
catalog, including accessibility calls and queued callbacks. Invalidate old
objects synchronously before publishing replacement models. Use QPointer and
context-bound queued work for destruction safety.

## Geometry and focus composition

Extend the declarative registration path used by SelectableText and ImageBlock
with link metadata. FormattedText remains responsible for shaped text fragment
geometry, including wrapping and bidirectional runs; image items contribute their
painted bounds. Aggregate drawable fragments by logical ID in view coordinates.
Ignore empty geometry and nonvisible presentation items. Refresh after layout,
width/style changes and image decode; coalesce layout refreshes without rebuilding
the logical catalog. Reuse existing navigation geometry conventions.

Create one private QQuickItem focus target for each drawable logical link, owned
at the first contributing presentation position. Maintain their traversal order
in the visual item tree alongside existing action hosts, rather than intercepting
Tab globally or imposing a focus list over host controls. Later fragments refer to
the same target. Set activeFocusOnTab only when opt-in is enabled. Do not add a
MarkdownView Tab stop. Default Qt traversal handles entry, reverse entry, no-link
documents and boundary exit. Custom renderer focus descendants remain untouched.
Test Qt's actual focus-chain behavior on both supported SDKs.

Targets handle unmodified Tab/Shift+Tab through Qt traversal and unmodified
Enter/Return through a single press handler; reject auto-repeat activation and
leave modified keys unconsumed. Existing selection shortcuts bubble to the view.
Focus acquisition requests the union of current fragment rectangles through
navigationRequested; later geometry changes refresh a still-focused target's
visibility request. Focus loss hides outlines. Navigation disablement removes
Tab participation and clears target focus without selecting a replacement link.
When temporary delegate replacement occurs during image projection, transfer
active focus only to a surviving target with the same generation/ID; never steal
focus back after the host has moved it elsewhere.

Paint focus outlines from the existing shaped fragment geometry and image bounds,
using a two-logical-pixel pen and no layout padding. Clip painting to available
item bounds and keep it above selection/Find highlights. One view color input,
`keyboardFocusColor: style.linkColor`, follows style replacement through binding.
No Controls dependency is added to the library.

## Accessibility

Use a private native target type and public Qt accessibility interfaces, following
the existing TaskIndicatorHostItem factory pattern. Each target supplies role
Link, decoded Name, destination Value, screen-coordinate union bounds, focus state
and a press action. Report focusability consistent with keyboard opt-in while
retaining press action when opt-in is false. Expose only one Link node per ID;
fragment paint items must not create duplicate accessible links. Preserve existing
non-link text and host-control accessibility. Emit focus, name/location and
object lifecycle notifications as applicable. Activation routes through the same
generation-checked coordinator dispatch as keyboard activation.

This native target adds private implementation code but avoids fake transparent
QML controls and supports one accessible object across multiple painted fragments.
Only the two MarkdownView properties are public API additions.

## Delivery

Extend focused Qt tests near tst_reading/tst_selection and add native accessibility
assertions. Update private registration, QML type metadata, source distribution
allowlist and installed inventories for any added helper. Add playground examples
with duplicate destinations, wrapped text, linked images and nested/table links,
plus controls before/inside/after the view for focus checks.

Keep current published summaries at 1.9 during implementation. In the final
release-preparation step, update all current-release markers, 1.10 notes/changelog,
compatibility/download/install guidance together and run the strict release gate.
Do not prewrite successful verification or publication evidence. Keep release
preparation in the same meaningful change; no merge/tag/publish action is granted.

## Release delivery authorization

The supplied “Update documentation and publish QMarkdown 1.10.0” plan explicitly
authorizes documentation reconciliation, one release PR, squash merge, workflow
dispatch, tagging and stable/latest publication with four source/Arch assets and
downloaded-asset SHA-256 verification. This extends the historical implementation
approval above without changing runtime scope, package 1.10.0 or QML 1.10.
Follow the [development workflow](../../docs/development-workflow.md); stop on a
failed stage and preserve existing tags and published bytes.
