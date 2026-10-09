# CommonMark baseline

After building the tests, run both evidence layers offline:

```sh
python3 scripts/verify-commonmark.py
```

The default probe is `build-shared/tests/qmarkdown-commonmark-probe`; the default JSON report is `build-shared/tests/commonmark-report.json`. For another build:

```sh
python3 scripts/verify-commonmark.py --probe build-static/tests/qmarkdown-commonmark-probe --report build-static/tests/commonmark-report.json
```

`--strict` fails if any official parser comparison fails, any model example is mismatched/uncheckable, or any comparison limit/information loss remains. An unchanged reviewed baseline can pass CTest while strict conformance fails. Exit codes: 0 for an unchanged baseline (and complete evidence if strict was requested), 1 for baseline changes or incomplete strict evidence, 2 for invalid input/output/ledger or execution errors. Strict mode still writes the full report.

CTest runs `qmarkdown-commonmark-oracle` (authored oracle/model and controlled-fault checks) and `qmarkdown-commonmark` (all official examples). Python ≥3.9 is needed only when `BUILD_TESTING=ON`. The runner uses only its standard library, makes no requests and invokes the private probe once for the full corpus. Shared/static packages, imports and version remain 0.7.0/0.7.

## Evidence boundaries

The parser comparison uses the repository's bundled cmark objects, `CMARK_OPT_DEFAULT`, and exact official expected HTML. `CMARK_OPT_UNSAFE` is used only in the test serializer so recognized raw HTML is included instead of hidden. No HTML is executed, and production still has no HTML renderer.

The second comparison calls production `QMarkdownPrivate::parse` independently. Its raw blocks, owned recursive inline trees, metadata and UTF-16 ranges are retained in every JSON report entry. For IDs 148–167 and 169–191, complete raw models are independently authored from pinned Markdown and CommonMark section 4.6 in `html-block-expectations.json`, including literal text, native boundaries, nesting and metadata. Every ID has a review rationale; exact IDs, corpus checksum and model shape are validated. Feature 015 adds the remaining 26 complete raw models in `remaining-expectations.json`, with the same provenance and per-ID review plus explicit `limits` and `losses` arrays. Feature 016 extends all 69 source-authored cases with inline trees reviewed from Markdown/rules; their five superseded loss annotations are removed. Source comparisons normalize adjacent Text siblings only. Other expected models are derived from official HTML using an independent, deliberately restricted `HTMLParser` oracle, never from probe HTML or production model output. Probe schema 2, authored-fixture schema 2 and report schema 3 reject stale formats. The report records `source-authored` or `html-projection` per example; the historical `projection-pass` status means the chosen comparison passed. Native renderer/resource tests provide separate presentation evidence.

Short malformed comments (`<!-->` and `<!--->`) remain unsupported by the HTML-only oracle because Python releases differ in their recovery rules; source-authored expectations now cover official example 626. The oracle does not repair malformed HTML or silently ignore unsupported tags. It verifies complete HTML token consumption. It distinguishes character-reference newlines from serialization newlines when HTML preserves that distinction, and checks authored source cases where HTML loses it. Balanced unknown inline tags preserve exact literal opening/closing spelling; unsupported block boundaries and malformed raw tags are reported as uncheckable. Generated semantic tags and source literal tags can have the same HTML: successful comparisons declare that source ambiguity explicitly.

The comparison preserves text, meaningful whitespace, block order/nesting, heading level, list start/tightness, formatting flags, hard-break newlines, soft-break spaces, link ranges and image metadata. Inline tree comparisons retain Text, Code, Emphasis, Strong, SoftBreak, HardBreak, Link, Image and Html identity, including nesting, decoded targets/titles and empty children. Adaptation omits cmark’s empty Text artifacts left by trailing-space trimming while preserving empty Link/Image containers. Normalization preserves standalone empty Text nodes and merges adjacent Text nodes alone; containers are never flattened and breaks are never merged into text. Production renders soft breaks as spaces, repeated emphasis as effective flags, empty links without a clickable area and HTML literally. Titles gain no activation or tooltip behavior. It strips only the generated line separators adjacent to native block children or following `<br />`. For URL comparison, it percent-encodes native destinations using the HTML serializer's safe set, preserving existing escapes, and compares decoded HTML attributes. Raw decoded strings stay unchanged in probe/report output and have independent authored source assertions. This avoids guessing whether an HTML `%20` originated from a literal space or an existing escape.

## Reviewed gaps

Feature 016 (2026-10-09) records **652 parser passes, 652 model passes (583 HTML projections plus 69 source-authored models), zero mismatches/uncheckable and zero reported semantic losses**. Pre/post comparison confirms all 652 presentation models unchanged. Raw fingerprints now include owned inline trees. Strict mode exits 1 because independent comparison limits remain; full CommonMark conformance is not established.

| Distinction | Examples | Interpretation and guard |
| --- | --- | --- |
| HTML/token/boundary cases | 69 source-authored | Complete source/rule-reviewed models check exact literal text/tokens, block boundaries, inline identity and metadata; includes entity-LF example 39. |
| Complete fence info and code origin | 78 | HTML preserves only the first info word and loses fenced/indented origin. Raw hashes and authored full-info assertions supplement text/language checks. |
| Ordered-list delimiter | 26 | HTML loses `.` versus `)`. Raw hashes and authored delimiter/start assertions supplement comparison. |
| Original URL escape spelling | 131 | Encoded HTML cannot identify all original targets. Compare encoded destinations and preserve decoded raw strings/hashes, with authored space/escape/Unicode assertions. |
| Image-description structure/formatting | 22 | HTML alt text cannot recover source interior nodes. Compare description text, outer image URL/title and enclosing-link metadata; exclude interior formatting ranges only. Dedicated source assertions verify nested emphasis, links/images and empty descriptions; links/nested images stay inert in presentation. Raw trees remain fingerprinted. |
| Literal inline HTML versus generated semantic tags | 258 | The same official `<em>`, `<strong>`, `<code>`, `<a>`, `<img>` or `<br>` token can come from raw source HTML. Compare generated semantic trees without hiding nodes, declare source ambiguity, and check literal identity separately with source-authored cases. |
| Soft break versus entity-decoded LF | 64 | A serialized literal LF can have either source origin. HTML oracle compares explicit SoftBreak nodes under the generated-HTML interpretation; source-authored example 39 and dedicated mixed entity/soft/hard-break assertions check exact source distinctions. |
| Five previous semantic losses | 0 remaining annotations | Soft breaks (65 previous IDs), link titles (29), repeated emphasis depth (19), empty links (2) and inline HTML identity (25) are now independently checked retained nodes. Counts overlap across 134 IDs. |

390 examples have comparison limits (counts overlap). The two newly explicit source ambiguities expose existing HTML evidence limits; they do not mask or flatten semantic containers or breaks. Full semantic evidence for opaque image interiors and the remaining source distinctions still needs later work. Each exception and rationale is pinned by ID in `ledger.json`; changes are detailed in [the individual review](../../specs/016-inline-semantic-retention/ledger-review.md).

## Ledger review

`ledger.json` records every exception by exact example ID: expected statuses, limits/losses, review note, raw model SHA-256 and, for known mismatches, an evidence SHA-256. Projection passes without limitations/losses implicitly expect a complete comparison pass. A raw hash for opaque fields guards regressions; it is not an independent semantic expectation. Even an improved status requires review so stale exceptions cannot remain hidden. There is deliberately no automatic ledger-update option.

To review a change, inspect the pinned Markdown and official HTML, the raw native model and differences in the report; determine whether the change is an oracle correction, an intended projection, or a production regression. Add/update exact IDs and explanatory review notes only after that inspection. Unexpected parser/model failures, missing fixtures, invalid schemas, unknown fields and changed opaque model fields fail the baseline. Fixture updates require explicit maintenance of provenance and licensing as well as ledger review.

## Subsequent work

Feature 012 independently covered 43 HTML-block IDs; Feature 015 covered 26 remaining cases. Feature 016 preserves their source-reviewed boundaries/metadata while extending semantic trees and rejecting prior formats. Tests cover stale expectations/reports, wrong break kinds, dropped titles/empty links, flattened nesting, HTML converted to Text, exact normalization, decoded newlines, image-description semantics, and existing presentation/ledger faults.

Resolve the remaining independent comparison limits before claiming the v1.0 gate. GFM, streaming, selection/copy, highlighting and performance optimization remain deferred.
