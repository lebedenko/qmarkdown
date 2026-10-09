# CommonMark 0.31.2 official examples

- Source: <https://spec.commonmark.org/0.31.2/spec.json>
- Specification: <https://spec.commonmark.org/0.31.2/>, version 0.31.2 (2024-01-28), by John MacFarlane.
- Acquisition date: 2026-10-08.
- SHA-256 of unchanged `spec.json`: `d431b29d97b6f73e69d547109cf5081578fac931e72afe95639ebe766c1b2a20`.
- Contents: 652 examples, IDs 1–652; Markdown, HTML, section and source-line records retained unchanged. `fixture.json` records the machine-readable provenance.
- License: Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0), as stated by the specification. See the accompanying [license text](LICENSE-CC-BY-SA-4.0.txt) and <https://creativecommons.org/licenses/by-sa/4.0/>. This fixture is not covered by the repository's MIT license.

Acquisition commands (maintenance only, never executed by configure/build/test):

```sh
curl --fail --location --silent --show-error https://spec.commonmark.org/0.31.2/spec.json -o tests/commonmark/spec.json
curl --fail --location --silent --show-error https://creativecommons.org/licenses/by-sa/4.0/legalcode.txt -o tests/commonmark/LICENSE-CC-BY-SA-4.0.txt
sha256sum tests/commonmark/spec.json
```

Authored test code and review notes retain the repository's MIT license. The ledger stores authored review classifications and model fingerprints, not modified copies of upstream fixtures. Tests, fixture notices and reports are not installed with the QMarkdown package; the existing bundled cmark notices remain unchanged.

`html-block-expectations.json` includes adapted literal Markdown from the pinned CommonMark examples and is distributed under the same CC BY-SA 4.0 license, attributed to John MacFarlane. Native-model boundaries/metadata and per-ID review notes were independently authored by qt-markdown contributors on 2026-10-08; no production output generated these expectations.

`remaining-expectations.json` adapts literal Markdown from the same pinned corpus under CC BY-SA 4.0, attributed to John MacFarlane. Native-model expectations, per-ID rationales and semantic annotations were independently authored by qt-markdown contributors on 2026-10-09 from the Markdown and CommonMark 0.31.2 rules; no production output generated the expectations.
