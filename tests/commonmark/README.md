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

CTest runs `qmarkdown-commonmark-oracle` (authored oracle/model and controlled-fault checks) and `qmarkdown-commonmark` (all official examples). Python ≥3.9 is needed only when `BUILD_TESTING=ON`. The runner uses only its standard library, makes no requests and invokes the private probe once for the full corpus. Current shared/static package/module versions are 1.0.0/1.0. Supplementary long-fence probe tests use source-derived HTML and complete model expectations outside the unchanged 652-example official corpus and ledger.

## Evidence boundaries

The parser comparison uses the repository's bundled cmark objects, `CMARK_OPT_DEFAULT`, and exact official expected HTML. `CMARK_OPT_UNSAFE` is used only in the test serializer so recognized raw HTML is included instead of hidden. No HTML is executed, and production still has no HTML renderer.

The second comparison calls production `QMarkdownPrivate::parse` independently. Its raw blocks, owned recursive inline trees, metadata and UTF-16 ranges are retained in every JSON report entry. For IDs 148–167 and 169–191, complete raw models are independently authored from pinned Markdown and CommonMark section 4.6 in `html-block-expectations.json`, including literal text, native boundaries, nesting and metadata. Every ID has a review rationale; exact IDs, corpus checksum and model shape are validated. Feature 015 adds the remaining 26 complete raw models in `remaining-expectations.json`, with the same provenance and per-ID review plus explicit `limits` and `losses` arrays. Feature 016 extends all 69 source-authored cases with inline trees reviewed from Markdown/rules; their five superseded loss annotations are removed. Source comparisons normalize adjacent Text siblings only. HTML comparison models are derived from official HTML using an independent, deliberately restricted `HTMLParser` oracle, never from probe HTML or production model output. Probe schema 2, existing authored fixtures schema 2, new semantic fixture schema 1 and report schema 4 reject stale formats. Feature 017 adds complete source-reviewed models for the exact 390 iteration 016 ledger IDs in `semantic-expectations.json`; its checksum, complete model shapes and per-ID reviews are validated. Existing 69 authored models remain unchanged. Each report model records a `checks` array with method, status, limits/losses and mismatch evidence. All required checks must pass: the 390 new cases retain both source and HTML checks; 69 use existing source checks and 193 use HTML alone. Only passing complete source evidence clears aggregate limitations; the HTML check keeps its own annotations. Native renderer/resource tests provide separate presentation evidence.

Short malformed comments (`<!-->` and `<!--->`) remain unsupported by the HTML-only oracle because Python releases differ in their recovery rules; source-authored expectations now cover official example 626. The oracle does not repair malformed HTML or silently ignore unsupported tags. It verifies complete HTML token consumption. It distinguishes character-reference newlines from serialization newlines when HTML preserves that distinction, and checks authored source cases where HTML loses it. Balanced unknown inline tags preserve exact literal opening/closing spelling; unsupported block boundaries and malformed raw tags are reported as uncheckable. Generated semantic tags and source literal tags can have the same HTML: successful comparisons declare that source ambiguity explicitly.

The comparison preserves text, meaningful whitespace, block order/nesting, heading level, list start/tightness, formatting flags, hard-break newlines, soft-break spaces, link ranges and image metadata. Inline tree comparisons retain Text, Code, Emphasis, Strong, SoftBreak, HardBreak, Link, Image and Html identity, including nesting, decoded targets/titles and empty children. Adaptation omits cmark’s empty Text artifacts left by trailing-space trimming while preserving empty Link/Image containers. Normalization preserves standalone empty Text nodes and merges adjacent Text nodes alone; containers are never flattened and breaks are never merged into text. Production renders soft breaks as spaces, repeated emphasis as effective flags, empty links without a clickable area and HTML literally. Titles gain no activation or tooltip behavior. It strips only the generated line separators adjacent to native block children or following `<br />`. For URL comparison, it percent-encodes native destinations using the HTML serializer's safe set, preserving existing escapes, and compares decoded HTML attributes. Raw decoded strings stay unchanged in probe/report output and have independent authored source assertions. This avoids guessing whether an HTML `%20` originated from a literal space or an existing escape.

## Complete corpus semantic evidence

Feature 017 records **652 parser passes and 652 model passes**, zero mismatches/uncheckable examples and zero aggregate limits/losses. Strict mode exits 0 without changing its acceptance criteria. Source evidence covers 459 examples, including complete models for every former limited ID; 193 examples require HTML alone. HTML checks still run for all 390 newly covered cases (583 HTML checks total). Every raw production model equals iteration 016, including presentation fields and recursive trees.

| Former HTML limitation | IDs reviewed | Complete source evidence |
| --- | ---: | --- |
| Full fence info / code origin | 78 | Recognition, exact code content and full decoded info. Both fenced and indented syntax map to CodeBlock; syntax origin is outside the retained model contract. |
| Ordered-list delimiter | 26 | Exact `.`/`)`, start, tightness, nesting and contents. |
| Original URL spelling | 131 | Exact decoded destinations in trees and presentation spans; existing escapes preserved. |
| Image-description interiors | 22 | Complete recursive description trees and formatting ranges, including nested inert links/images and empty children. |
| Literal HTML / generated tags | 258 | Source-reviewed Text/Html/Code/container identity and exact literal spelling. |
| Soft break / decoded LF | 64 | Source-reviewed Text LF versus SoftBreak/HardBreak identity. |

Counts overlap across the exact 390-ID union. The new fixture was authored with official HTML corroborating structure, then source/rule review supplying opaque fields and confirming the complete trees; no production output, bundled-parser tree or fingerprint authored expectations. Presentation spans were counted from reviewed inline trees. Initial source fixture URL spelling errors in 526, 538 and 603 were corrected after re-reading their autolink source. The [individual ledger removal record](../../specs/017-complete-semantic-evidence/ledger-review.json) maps every former limitation to its complete source evidence. The accepted ledger has no exceptions.

This completes evidence for the pinned corpus semantics. Native presentation, resource behavior and release readiness remain separate requirements; this iteration does not declare v1.0 ready. GFM and other deferred work remain outside scope.

## Ledger review

`ledger.json` records every exception by exact example ID: expected statuses, limits/losses, review note, raw model SHA-256 and, for known mismatches, an evidence SHA-256. Projection passes without aggregate limitations/losses require every check to pass. HTML limitations remain visible in individual checks even when source evidence resolves them. A raw hash for opaque fields guards regressions; it is not an independent semantic expectation. Even an improved status requires review so stale exceptions cannot remain hidden. There is deliberately no automatic ledger-update option.

To review a change, inspect the pinned Markdown and official HTML, the raw native model and differences in the report; determine whether the change is an oracle correction, an intended projection, or a production regression. Add/update exact IDs and explanatory review notes only after that inspection. Unexpected parser/model failures, missing fixtures, invalid schemas, unknown fields and changed opaque model fields fail the baseline. Fixture updates require explicit maintenance of provenance and licensing as well as ledger review.

## Subsequent work

Feature 012 independently covered 43 HTML-block IDs; Feature 015 covered 26 remaining cases. Feature 016 preserves their source-reviewed boundaries/metadata while extending semantic trees and rejecting prior formats. Tests cover stale expectations/reports, wrong break kinds, dropped titles/empty links, flattened nesting, HTML converted to Text, exact normalization, decoded newlines, image-description semantics, and existing presentation/ledger faults.

Feature 017 resolves the six remaining HTML evidence categories through complete source checks and tests truncated info, wrong delimiters, altered URL spelling, flattened image interiors, Html changed to Text, SoftBreak changed to decoded LF, missing/failing source checks and failed HTML despite passing source. Corpus semantics alone do not establish the v1.0 gate. GFM, streaming, selection/copy, highlighting and performance optimization remain deferred.
