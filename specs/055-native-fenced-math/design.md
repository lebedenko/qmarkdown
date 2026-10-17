# Design — Native fenced math

Approval: requirements, design and tasks approved by the user response “approved” on 2026-10-16. Dependency audit and verification remain implementation tasks; no success is implied. One cohesive 1.13.0 / QML 1.13 release PR is planned; merge, tagging and publication require separate authorization.

Release delivery approval — 2026-10-17: the user authorized the release plan for PR #36, including documentation completion, fresh final-head Qt 6.8.0/6.12.0 CI and retained full gate, squash merge through normal protections, `release.yml` dispatch with `release_pr=36`, tagging `v1.13.0`, stable/latest publication and downloaded hash verification of exactly four assets. Retain package 1.13.0 / QML 1.13 and existing APIs; no runtime changes. Stop on failed gates, changed inputs or differing published bytes. Historical approvals and results below remain unchanged; this approval supersedes earlier statements that delivery still requires authorization.

## Existing integration points

The private document adapter preserves code text, info string and fenced status. MarkdownView passes the renderer registry to BlockSequence and CodeSurface. CodeSurface owns host renderer creation, source modes, decorations and viewport behavior. Extend these boundaries rather than changing the public renderer contract. Keep native C++ types and the engine private.

Dispatch order is source mode, registered host renderer, eligible native math, ordinary source. A registered but invalid host renderer keeps existing source fallback rather than silently selecting native math. Backend absence does not disable the opt-in parser behavior of feature 056.

## Rendering and lifetime

A private math service owns a single serialized worker for the application lifetime. GUI-thread initialization registers embedded fonts before requests run. Each request captures source, resolved size/color and an identity/generation token. The worker validates the restricted dialect, creates fresh formula state, lays out and records painter commands. Publish an immutable recording and natural bounds to the GUI thread; a private QQuickPaintedItem replays it without engine calls during paint. Replacement/destruction invalidates tokens; queued stale requests are dropped. Cache only successful recordings under bounded entry/byte limits with source and effective style in the key.

Use embedded Qt resources, with a private resource adapter for engine reads. Do not expose filesystem paths or engine configuration through document commands. Audit the command registry transitively; use an explicit permitted command/environment set so aliases cannot bypass restrictions. Reject unsupported syntax before execution, and reject the entire result on backend diagnostics or nonfinite/oversized geometry. Reset or replace all engine mutable formula state between requests.

Proposed limits for review: 16 KiB UTF-8 source, 64 nesting levels, 4096 parser nodes, 16384 logical pixels per axis, 64 queued requests, 64 cache entries and 16 MiB cached recordings. Queue saturation shows source and permits later retry. Instrument parser/layout operation budgets (initial target 100000 operations); wall-clock timeout alone is insufficient for an uninterruptible backend. Bound recording bytes before caching. Never terminate a worker thread forcibly. Document the maximum remaining application shutdown wait from any audited uninterruptible operation.

A shared private formula viewport is used by fenced and standalone forms. It clips horizontal overflow, reserves the existing scrollbar gutter, centers natural-width content when it fits, and propagates unused wheel movement to the host. Preserve antialiasing, use display-scale-aware painted texture resolution and avoid extra texture smoothing, consistent with feature 054. Math styling changes invalidate results without changing source projections.

## Dependency audit resolution — 2026-10-16

Pinned MicroTeX 0e3707f6dafebb121d98b53c64364d16fefe481d (MIT), with
bundled font notices (OFL, Knuth and dsrom notices) and TinyXML-2
11.0.0 / 9148bdf719e997d1f474be6bcc7943881046dba1 (zlib). The
repository wrapper compiles its Qt Gui backend under C++17 without demos.
Qt resources replace filesystem reads; fonts register before worker dispatch.
A preflight allowlist rejects all document commands outside the restricted dialect.
Strict parsing rejects incomplete input. Fresh formulas and clearing mutable
predefined-formula atom caches isolate inputs. Internal default macros remain
trusted engine definitions, not document-defined commands.

The private patches instrument shared allocations, parser entry/loops and atom
layout entry/loops. Recordings use normalized glyph outlines because upstream
QPicture drawText replay dropped glyphs. Budgets remain those proposed above,
with a 1 MiB per-recording, 4 megapixel painted-area and 512 px effective text-size cap. These are operation
and geometry limits, not a hard wall-clock guarantee: Qt font/path operations,
standard-library calls and trusted-resource initialization remain uninterruptible.
Shutdown joins the active worker; no fixed maximum wait is claimed. See the
[math guide](../../docs/math.md) and [provenance](../../third_party/microtex/PROVENANCE.md).
