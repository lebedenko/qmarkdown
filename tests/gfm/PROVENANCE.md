# Selected GFM task-list examples

- Source: [GitHub Flavored Markdown specification](https://github.github.com/gfm/#task-list-items-extension-), version 0.29-gfm (2019-04-06), GitHub, based on John MacFarlane's CommonMark specification.
- Acquisition date: 2026-10-13. Markdown for section 5.3 examples 279 and 280 was transcribed from the published versioned specification. No upstream HTML or parser output was used to generate model expectations.
- `task-lists.json` retains those two Markdown inputs and example IDs; ordinary/unchecked/checked numeric states and depth-first label expectations were independently authored by qt-markdown contributors from the specification.
- The adapted fixture is distributed under the specification's Creative Commons Attribution-ShareAlike 4.0 International license, not the repository's MIT license. The full [license text](../commonmark/LICENSE-CC-BY-SA-4.0.txt) is retained in this repository. Authored test code retains the repository's MIT license.
- This fixture covers task-list examples only; it does not establish full GFM conformance. It is not installed with the library. Acquisition is maintenance-only; builds/tests remain offline.

The fixture checksum is recorded below; recompute with `sha256sum tests/gfm/task-lists.json` after an intentional fixture revision.

SHA-256 of `task-lists.json`: `1832c006004151abc3b790476f0c7f39994be8e726efbf085a243785856835f0`.

## Selected strikethrough examples

`strikethrough.json` transcribes Markdown inputs and IDs 491–493 from [GFM 0.29-gfm §6.5](https://github.github.com/gfm/#strikethrough-extension-) (2019-04-06), acquired 2026-10-13. Attribution: GitHub, based on John MacFarlane's CommonMark specification. Text and UTF-16 formatting expectations were independently authored from the syntax rules; no upstream HTML/parser output generates expectations. This adapted fixture is CC BY-SA 4.0; see the [full license](../commonmark/LICENSE-CC-BY-SA-4.0.txt). Tests remain offline; this is selected extension evidence, not full GFM conformance.

SHA-256: `9040dc9e2509e48e1c92b9abed592d326de9dce640101071527ee8d3f18f6d6a`.

## Selected extended-autolink examples

`extended-autolinks.json` transcribes all 14 Markdown inputs and IDs 622–635 from [GFM 0.29-gfm section 6.9](https://github.github.com/gfm/#autolinks-extension-) (2019-04-06), acquired 2026-10-13. Attribution: GitHub, based on John MacFarlane's CommonMark specification. Displayed text, link destinations and UTF-16 spans were independently authored from the specification and explicit selected labels, without production or upstream parser output. Tests assert ordinary semantic link/text nodes as well as projected spans. This adapted fixture is CC BY-SA 4.0; see the [full license](../commonmark/LICENSE-CC-BY-SA-4.0.txt). Authored test code remains MIT. The fixture is not installed; tests remain offline and establish selected extension evidence only.

SHA-256: `c7d007666dd272321497fb1867024de1837bca3561b6333d2edcd53024b1525f`.
