# Design — QML responsibilities and private contracts

Status: **approved on 2026-10-15** by the user response “approved”.
Approval covers requirements R1–R5, design and tasks T1–T7 implementation and
verification. Merge and publication require separate authorization.

## Search projection

Replace ViewState's private searchMatches property/accessor with invokable
`searchRangesFor(int offset, int length)`. Treat intervals as half-open UTF-16
projection ranges; empty/nonpositive lengths yield no ranges. Use widened endpoint
arithmetic to avoid overflow. Binary-search ordered non-overlapping matches for
the first match whose end exceeds offset, then iterate while start precedes the
query end. Clip intersections and subtract offset; return `{start, length,
current}` maps compatible with FormattedText painting.

Expose a private integer searchRevision property, notified whenever searchChanged
is emitted for rebuilding or advancing results. Each QML range binding explicitly
uses this revision in its result expression before invoking the bounded query
with its projection length. Zero denotes pristine state; revision increments skip
zero on wrap. Qt 6.8 can elide an unused local read, so merely assigning the
revision to an unused variable is insufficient.
Images query their description interval directly; remove full-list mapping/filtering.
Search remains independent of selection and preserves existing paint order.

## QML composition

Extract private BlockSequence.qml from the inline sequence. Keep a lazy Component
factory in MarkdownView and pass it explicitly to each sequence: QML file types
cannot instantiate themselves, even inside Component. Extract TextBlock.qml,
ImageBlock.qml and ListBlock.qml; use a shared SelectableText.qml for FormattedText
selection, search and registration setup, including code text. Keep small quote,
thematic-break and literal-HTML delegates in BlockSequence; share the small
HeadingSeparator.qml between sequence and text presentation. Retain TableBlock and
CodeSurface. Internal QML types stay in the existing QMarkdown module with internal
resource aliases; only native helpers are registered under QMarkdown.Private.

Pass coordinator, MarkdownStyle, body font/color, alignment, spacing, heading
decoration suppression and applicable extension settings explicitly. Pass action
Component, renderer registry, source identity, viewport availability and search
colors/state explicitly through sequence boundaries. Relay linkActivated and
codeBlockCopyRequested to MarkdownView. Default typography continues to follow
style bindings; table header/body overrides and zero-width model suppression remain.
Keep Layout.js and QML ownership of all visual measurements and interactions.

TableBlock retains an explicit sequenceComponent input. Cast each Loader.item to
BlockSequence before binding properties; warn clearly and skip assignment if the
private component is incompatible. Use typed MarkdownStyle inputs throughout.

## Code lifecycle

CodeActionHost receives actions, codeText, source identity and viewport availability
and emits a copy request. Retain the captured source/component and active validity
token checks around callbacks. CodeSurface receives renderer registry, search
state/colors, explicit action inputs and coordinator. Keep createObject, Qt.binding,
validation, invalidation and destruction in QML. Preserve synchronous construction
replacement checks and presentationReset handling, including source fallback while
selection or search is active. Do not move host objects across creation contexts.

Add language to PresentationBlock and a private BlockModel role. Extract the first
non-whitespace token from infoString during projection using behavior equivalent
to the existing JavaScript expression; preserve the original infoString payload.

## Typed registration

Add SelectionRegistration QObject implementing QQmlParserStatus. Expose typed
ViewState coordinator, QQuickItem item, SelectionItemKind kind, integer offset and
length, and image-link string/linked boolean properties. Register only after
componentComplete. Setters update the active metadata and request geometry refresh;
coordinator/item replacement detaches the previous registration first. Use QPointer
for both dependencies and destruction connections for prompt cleanup. Destroying
the helper unregisters automatically; coordinator destruction is safe.

ViewState tracks registrations and consumes their metadata for hit testing,
selection positions, image links and navigation. FormattedText remains responsible
for cursor/range geometry. TableCellSelection scopes hit testing; ActionSelection
excludes gestures; neither contributes text/navigation ranges. Connect geometry
notifications for the registration's current item and disconnect them on detach.
Remove old manual QML register/unregister calls and dynamic projection reads after
migration. Register the helper only under QMarkdown.Private and describe it in
private.qmltypes, following existing native adapter registration conventions.

## Delivery

Update src/QMarkdown/CMakeLists.txt resources/install lists, private metadata,
scripts/release/source-files.txt and packaging/archive expectations together.
Preserve the existing unrelated root CMakeLists.txt working-tree change.
Capture advisory baseline search activation/next-result measurements before runtime
edits, varying block and match counts; compare after implementation without timing
thresholds. Follow the repository development workflow for a single focused PR.

## Implementation refinements within approved scope

BlockSequence dispatches on stable renderKind. TextBlock and the literal-HTML
Loader switch native/searchable text explicitly from selection/search inputs;
CodeSurface receives selection separately. Qt 6.8 can evaluate a changed
DelegateChooser role after a synchronous model reset, so changing role names is
not a reliable mode transition. The stable dispatch preserves the native Text fast
path and requires no search-triggered model rebuild.

CodeActionHost rebuilds on sourceIdentity changes as well as action/text changes;
explicit bindings may settle after model replacement, and captured copy callbacks
must use the final source identity. TextBlock clears its pressed link on
presentationReset so a release cannot activate a replaced document's link.
