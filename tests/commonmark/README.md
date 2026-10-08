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

The second comparison calls production `QMarkdownPrivate::parse` independently. Its raw blocks, metadata and UTF-16 ranges are retained in every JSON report entry. For IDs 148–167 and 169–191, complete raw models are independently authored from pinned Markdown and CommonMark section 4.6 in `html-block-expectations.json`, including literal text, native boundaries, nesting and metadata. Every ID has a review rationale; exact IDs, corpus checksum and model shape are validated. Other expected models are derived from official HTML using an independent, deliberately restricted `HTMLParser` oracle, never from probe HTML or production model output. Report schema 2 records `source-authored` or `html-projection` per example; the historical `projection-pass` status means the chosen comparison passed. Native renderer/resource tests provide separate presentation evidence.

Short malformed comments (`<!-->` and `<!--->`) are explicitly uncheckable because Python releases differ in their recovery rules. The oracle does not repair malformed HTML or silently ignore unsupported tags. It verifies complete HTML token consumption. It distinguishes character-reference newlines from serialization newlines when HTML preserves that distinction, and checks authored source cases where HTML loses it. Balanced unknown inline tags preserve exact literal opening/closing spelling; unknown block boundaries and ambiguous raw semantic tags are reported as uncheckable.

The comparison preserves text, meaningful whitespace, block order, nesting, heading level, list start/tightness, formatting flags, hard-break newlines, soft-break spaces, link ranges and image metadata. It strips only the generated line separators adjacent to native block children or following `<br />`. For URL comparison, it percent-encodes native destinations using the HTML serializer's safe set, preserving existing escapes, and compares decoded HTML attributes. Raw decoded strings stay unchanged in probe/report output and have independent authored source assertions. This avoids guessing whether an HTML `%20` originated from a literal space or an existing escape.

## Reviewed gaps

The baseline recorded on 2026-10-08 contains 652 parser passes, 626 model passes (583 HTML projections plus 43 source-authored raw models), zero model mismatches and 26 uncheckable model examples. No parser or adapter defect was established by these checks. This is incomplete model semantic evidence, not full CommonMark conformance.

| Distinction | Examples | Interpretation and guard |
| --- | --- | --- |
| Raw HTML/token/boundary ambiguity | 25 | HTML-only oracle cannot safely distinguish native literal HTML from semantic HTML output. The remaining ambiguous examples retain reviewed raw-model hashes; the 43 HTML-block examples now have independent complete expectations. |
| Entity-produced LF versus soft break | 1 (39) | Official HTML contains literal LFs after entity decoding. Production preserves these LFs; treating them as soft breaks would be a false failure. Authored assertion and raw-model hash protect this case. |
| Complete fence info and code origin | 78 | HTML preserves only the first info word and cannot identify fenced versus indented origin. Raw full-info hashes and authored full-info assertions supplement the independent text/language comparison. |
| Ordered-list delimiter | 26 | HTML has start/tightness, but loses `.` versus `)`. Source metadata hashes and authored delimiter/start cases supplement comparison. |
| Original URL escape spelling | 131 | HTML encoding cannot distinguish all original destinations. Encoded comparison, raw hashes and authored original-space/escape/Unicode assertions protect this boundary. |
| Image-description structure/formatting | 22 | HTML alt text loses description formatting and nested structure. Exclude only description interiors from independent range comparison; retain text, image offsets, URL/title/enclosing-link metadata and formatting outside descriptions. Raw hashes and authored Unicode/adjacent/empty/formatted image cases guard the excluded fields. |
| Soft-break kind projected into text | 64 | Native model uses spaces; semantic break identity is unavailable. |
| Link title omitted | 29 | Current native link spans retain destination/range, but omit nonempty titles. |
| Repeated emphasis depth flattened | 19 | Effective style flags are preserved; repeated identical nesting depth is lost. |
| Empty link omitted | 2 (484, 487) | No native link span exists for a zero-length text label. |
| Inline HTML identity projected | 4 | Literal spelling is preserved, but an inline HTML node is indistinguishable from ordinary text. |

Counts overlap: 250 examples have comparison limits and 113 have detected semantic information loss. Uncheckable examples may have additional distinctions that this oracle cannot classify; absence of a listed loss is not evidence of absence. All IDs and reasons are in `ledger.json` and the report, rather than section-wide exclusions.

## Ledger review

`ledger.json` records every exception by exact example ID: expected statuses, limits/losses, review note, raw model SHA-256 and, for known mismatches, an evidence SHA-256. Projection passes without limitations/losses implicitly expect a complete comparison pass. A raw hash for opaque fields guards regressions; it is not an independent semantic expectation. Even an improved status requires review so stale exceptions cannot remain hidden. There is deliberately no automatic ledger-update option.

To review a change, inspect the pinned Markdown and official HTML, the raw native model and differences in the report; determine whether the change is an oracle correction, an intended projection, or a production regression. Add/update exact IDs and explanatory review notes only after that inspection. Unexpected parser/model failures, missing fixtures, invalid schemas, unknown fields and changed opaque model fields fail the baseline. Fixture updates require explicit maintenance of provenance and licensing as well as ledger review.

## Recommended follow-up

Feature 012 supplies independent expectations for all 43 HTML-block IDs, including nested quote/list cases 174/175. Each removed ledger entry was previously an HTML-oracle uncheckable case and is now a complete raw-model pass; no other ledger entries changed. Example 148 specifically retains inline closing `pre` in its paragraph because type 7 cannot interrupt a paragraph. Controlled faults cover missing fixtures, altered literal text, boundaries, nesting, metadata, comparison methods and stale ledger entries.

Address the remaining 26 oracle cases: 21, 31, 39, 201, 308, 309, 344, 475–477, 491, 494, 524, 536, 613–616, 623, 626, 628–631, 642, 643. Separately design whether a private semantic representation should preserve soft-break identity, repeated emphasis depth, empty links and link titles for the v1.0 gate. Those decisions require later approval; Feature 011 does not implement fixes or richer semantics.
