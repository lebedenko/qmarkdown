# Design — library maintainability

Status: approved on 2026-10-14. Scope: [requirements](requirements.md).

## Private contracts (R1)

Give BlockModel roles an enum with explicit existing Qt::UserRole offsets. Define
private RenderKind and SelectionItemKind enums; export their names only through
QMarkdown.Private and update private tooling metadata. Keep semantic BlockKind
independent. Replace corresponding numbers in production QML/C++ and fixtures.
Do not change QVariant role payloads or public module type registration.

## Presentation composition (R2, R6)

Extract the shared code surface into a private QML component taking style, text,
selection state/range and action context explicitly. Keep native Text for disabled
selection and FormattedText for enabled selection. Share surface padding, borders,
action header and height calculations around those text paths.

Extract table presentation into a private component with an explicit block-sequence
Component dependency for recursive cell contents. Pass view/selection/action context
as explicit properties rather than depending on the public root's lexical scope.
Keep the sequence coordinator responsible for choosing block delegates. Move metric
sanitization and heading-style lookup into internal helper scope. Private components
must not acquire entries in the supported public QML API.

Preserve action creation contexts, callback invalidation, selection registration,
touch scrolling, table alignment and recursive style propagation. Add private QML
files to CMake resources, installed payload and source archive allowlist together.

## Semantic and presentation data (R3)

Keep Block as the parser-owned semantic tree: text, inline semantics, container
metadata, code info and filtered HTML. Remove decoded QImage/link presentation
fields, synthetic Image/Segments kinds and selection ranges from that tree.

Add a private PresentationBlock tree containing the render-facing text/ranges,
container metadata, decoded image/link data, children and selection range. A private
projection builder consumes const semantic blocks and produces the immutable copy
text plus presentation blocks with source ranges. Image projection consumes this
presentation tree and splits text/image leaves without mutating semantic blocks.
BlockModel owns presentation blocks. Resource discovery continues to consume the
semantic tree. ViewState coordinates parsing, projection and resource updates.

Copy only the fields presentation needs; do not retain a second complete semantic
subtree inside every presentation node. Adapt private parser/projection tests and
benchmarks to the explicit boundary. Preserve current full-model replacement.

The extra projection type introduces conversion code, but makes resource and
selection state independent of parser semantics without a new framework or variant
hierarchy. Public behavior and private semantic conformance output stay stable.

## Typed style implementation (R4)

Use typed value structs for body/general, heading, inline-code, code-block and table
styles, with corresponding default/current values. Track explicit overrides using
named private property identifiers (including heading level), not strings. Preset
application computes typed defaults and applies only non-overridden properties.

Keep existing public QObject groups and their parent ownership. Accessors delegate
to typed setters; setters explicitly emit the relevant grouped and legacy signals.
Keep equality handling for QFont resolve masks and NaN metrics. Resetting a property
clears its override and restores the selected preset value. An explicit assignment
equal to a default still establishes an override. Table foreground inheritance must
continue notifying when body foreground changes. Do not generate signal names or
use reflection to locate heading groups.

## Ownership and headers (R7)

Use std::unique_ptr with a cmark_node_free deleter for the parser tree. Child cmark
node pointers remain borrowed. Make ResourceController::restart take a const policy
reference and update production/test callers. Keep setNetworkManager private to
the implementation, explicitly documenting transfer of a non-null heap object;
guard null and same-manager assignments before cancelling or deleting anything.

Forward-declare network manager/reply, decoder job/scheduler and watcher types where
only pointer declarations require them. Move operations requiring complete types
out of headers. Add direct memory/utility/container includes where used. Preserve
complete definitions for base classes, value members and Qt property metadata.
Do not replace QObject parent ownership with competing smart-pointer ownership.

## API and delivery (R5, R8)

Add docs/api.md as the authoritative supported surface and link it from README and
feature guides. Document QML version introductions, legacy aliases, ownership,
reset/destruction fallback, effective GFM flags, copy/link callbacks, selection
offset units, resource rules and CMake shared/static targets. State that native
implementation headers and QMarkdown.Private are unsupported APIs.

Keep old release/specification records historical. Correct current stale statements
without claiming new verification. Update build/install/archive inventories for
new files; exercise installed consumers without source-tree imports. Record checks
in verification.md. Prepare one focused change; pushing, merging and publication
are outside this proposal's local implementation scope.
