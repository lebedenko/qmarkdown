# 063 — Highlighting provider design

Status: explicitly approved by the user on 2026-10-20. Implements [R1–R7](requirements.md).

## API and request lifetime (R1, R5)

Add a QML-registered `SyntaxHighlightingProvider` QObject at revision 1.17 with a
Role enum matching the existing private token-role ordering and SyntaxTheme palette.
Keep native headers private and uninstalled. The provider is host-owned; a view
stores a guarded pointer and never reparents it. Share it through ViewState to every
recursive CodeSurface, including footnote bodies.

Expose anonymous, nonconstructible request objects through the provider signal.
Source/language are immutable QString values. `cancelled` has notification;
`complete(QJSValue)` validates a JS array and `decline()` seals the request. Signal
callbacks and methods stay on the QML thread. Set terminal state before delivering
results so synchronous callbacks can replace/destroy providers or views safely.
Ignore subsequent terminal calls. A five-second GUI timer and elapsed-time check
on completion enforce the unanswered deadline; blocked host JS is not interrupted.

Pending requests are runtime-owned outside the provider’s QObject subtree, because
Qt invalidates child QML wrappers before the provider’s C++ destructor. On completion/cancellation, detach them and use
JavaScript ownership so a host-retained async wrapper can still read cancelled;
unreferenced settled wrappers can be collected. Ensure an engine wrapper exists
when releasing ownership so unhandled signals cannot leave uncollectable
parentless requests; without an engine, release settled objects with deleteLater. The runtime holds guarded pointers,
not permanent completed-request ownership. Cancel/detach active requests before
provider-private state destruction. Host-held wrappers/async work are host-owned
memory; they cannot deliver to destroyed subscribers.

## Provider dispatch and cache (R2, R5–R6)

Give each provider a private GUI-thread dispatcher and bounded LRU: 128 entries /
8 MiB, using source/language/span capacity costs like the existing built-in cache.
Object-local storage establishes identity isolation without pointer-reuse hazards.
Increment generation on invalidate; clear its cache, cancel pending requests, and
notify active SyntaxRequest subscriptions to refresh. A provider's destruction
cancels its work and discards its cache. Cache only valid completed spans, with an
explicit successful-empty state distinct from decline/failure.

Coalesce same-provider/generation/source/language active work into one request with
separate subscriber cancellation flags. Release a subscriber on replacement,
disablement or destruction; cancel the host request only when none remain. Fan out
immutable validated roles with generation/lifetime checks. Move callbacks to local
storage before delivery so reentrant edits cannot invalidate iteration.

Extend private SyntaxRequest with a guarded provider property and subscription
handle. Coalesce QML initialization through the existing zero-delay refresh.
Validate source/line limits before provider or backend work. Provider success supplies
roles; decline/invalid/timeout hands the still-current subscription to the existing
built-in scheduler. Without a compiled backend, deliver empty roles. The backend
retains its own cache, worker budget and lowercase language dispatch; provider
language remains exact. Reuse built-in source/language work safely only after each
provider's own dispatch decision; never mix host result caches across identities.

Palette changes still call rangesChanged/recolor against stored token roles, without
provider dispatch. Provider invalidation deliberately requests tokens again for
active sources. A provider removed from a view restores normal backend behavior.

## Validation and precedence (R3–R4)

Validate JS arrays before converting numeric fields. Require isNumber, finite,
integral values and safe source bounds before integer conversion. Require a known
role and positive length, then check both UTF-16 boundaries against adjacent
high/low surrogates. Limit array length before visiting spans, sort by start, and
reject any overlap. Do not coerce strings, NaN, Infinity, fractional values or
unknown enums; reject the whole result rather than partially coloring it. Never
accept source substitutions, fonts, sizes or arbitrary colors from a provider.

Keep CodeSurface's existing source-fallback eligibility: fenced source, enabled
master switch, no active custom visual/native math recording. Source fallback during
selection/Find remains eligible. Indented code and empty-language fences bypass the
provider. `syntaxHighlightingAvailable` remains compile-time built-in availability;
only dispatch logic changes to permit an assigned provider without that backend.
Plain copy/search strings and parser source are unchanged.

## Compatibility and delivery (R7)

Register the provider/enum at 1.17, add the nullable view property using established
module conventions, and prepare package 1.17.0 development metadata. Keep previous
imports and published-release records. Add a deterministic playground provider for
a made-up fenced language, including asynchronous completion and decline, without
requiring the optional backend. Document lifecycle, exact offsets, timeout, empty
success, invalidate, role palette and host-owned async execution.

Verify with deterministic fake-provider callbacks and the existing built-in test
seams where appropriate; include at least one real unanswered timeout and backend
ON/OFF integration. Installed consumers use only public QML, with shared/static and
shared plugin-only imports. Required final-head Qt packaging results are recorded
in the PR without an evidence-only follow-up commit. Merge/publication need separate
authorization.

Approval record: the user selected “Approve Feature 063” for requirements, design
and T1–T7 on 2026-10-20. Implementation and verification are authorized after
Feature 062's required CI checks pass. Merge, tagging and publication remain
excluded. No provider runtime implementation had started at approval time.
