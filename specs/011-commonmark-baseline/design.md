# 011: CommonMark conformance baseline design

**Approval:** On 2026-10-08 the user explicitly approved these requirements, design and tasks. Scope: T1–T6, test infrastructure and documentation only; package/module 0.7.0/0.7 and production behavior remain unchanged. Production fixes require subsequent approval.

## Inputs and dependency boundaries

Acquire https://spec.commonmark.org/0.31.2/spec.json once during implementation. Preserve its bytes under `tests/commonmark/`, with provenance, checksum and CC BY-SA 4.0 attribution/license information. Validate unique example identifiers and required fields; derive and record the example count from the pinned input rather than assuming it. Acquisition is an explicit maintenance step, never a CMake/test-time download. Use existing Qt Test/C++17 and Python standard-library tooling; introduce no library dependency. Fixtures and reports are test inputs, not installed assets.

## Two independent layers

Add a private test executable linked like the existing parser test, compiling production document/inline adapters and the same prefixed cmark object library. It emits structured JSON for fixture inputs and supports two operations:

1. Bundled parser evidence: parse with `CMARK_OPT_DEFAULT`, render with test-only `cmark_render_html` and the flag needed to include recognized raw HTML (`CMARK_OPT_UNSAFE`). Compare exact HTML bytes with the official expectation. This flag applies only to serialization in the test executable; it does not change production parsing or execute HTML. Free cmark allocations correctly and preserve symbol prefixing. Production code continues to use no HTML rendering.
2. Native model evidence: call production `parse(QString)` and serialize its recursive blocks and inline metadata before resource loading. Include all meaningful fields, UTF-16 ranges, list delimiter/start/tightness, code info and image/link metadata. Do not reconstruct absent information or use cmark's HTML output as the actual model result.

A Python runner reads the fixture and invokes the executable in batches. It uses a narrowly scoped standard-library HTMLParser oracle to convert official expected HTML into a canonical native expectation. The oracle is independent of the production adapter. Reject unsupported constructs explicitly rather than guessing; write small authored oracle fixtures for tight/loose lists, literal HTML, formatting nesting, links, images and escaped attributes before applying it to the official corpus.

Compare only specified projections: soft breaks become spaces; HTML becomes literal text when its boundaries can be established; decoded URLs remain original destinations in raw probe/report output; compare only their HTML-encoded form when the official serializer loses original escape spelling; emphasis ranges describe effective flags rather than nesting depth. Retain exact comparisons for text and meaningful block/code whitespace. Derive UTF-16 offsets explicitly, including astral characters. HTML cannot recover all source details, such as ordered-list delimiter or complete fence info: use independent authored Markdown-to-model assertions for those fields and label the official comparison's limits.

## Accounting and limits

Each official example receives separate parser and model results. Parser status is pass/fail. Model status is projection-pass, mismatch, or uncheckable, with reason and the compared/lost distinctions. A projection-pass means correctness of the declared native projection, not full semantic preservation. Maintain a reviewed gap ledger with exact identifiers, layer, reason and expected status; no blanket section exclusions or automatic acceptance of current output. Missing/duplicate IDs, unknown output fields and unexpected oracle errors fail validation. Explicitly uncheckable constructs remain counted in the ledger. Retain every raw native model in the report; fingerprint opaque source fields and uncheckable models in the reviewed ledger so changes remain visible even when independent comparisons cannot cover them. These fingerprints are regression guards, not semantic expectations. Short malformed comments are explicitly uncheckable to avoid Python-version-dependent HTMLParser recovery.

Baseline mode fails on unexpected changes from the reviewed ledger, including status improvements that require ledger review; it still prints known mismatches and uncheckable counts. Strict mode rejects parser failures, model mismatches/uncheckable results and unresolved semantic information loss. Do not reinterpret a green baseline test as a green release gate. Report the checksum, tool versions, options, per-layer totals, per-example details and stable ordering; keep nondeterministic timing outside the deterministic result payload.

## Native rendering and follow-up

Reuse existing native layout, links, images and resource tests as separate presentation evidence. Add only focused authored model/oracle checks needed to make this baseline trustworthy; do not expand rendering behavior. Produce a gap inventory distinguishing bundled-parser issues, adapter defects, oracle limitations and intentional presentation policies. Rank the next proposed fix slice by affected examples and user-visible impact.

Keep version 0.7.0 and imports 0.7: no public contract changes justify a version bump. The trade-off is deliberate: this baseline gives broad parser evidence and bounded projection evidence without redesigning the flattened model. A richer semantic model, if required for v1.0, needs its own design and approval.
