# Requirements — custom fenced-block renderers

Status: **approved on 2026-10-14** by the user response “approved”. Requirements
R1–R8, design and tasks T1–T5 are authorized for implementation and verification.
Package 1.7.0 / QML 1.7 is retained; no version bump, tag or publication is authorized.
See [requirements](requirements.md), [design](design.md), [tasks](tasks.md) and
[verification](verification.md).

## Approved requirements

- R1: Add `MarkdownView.fencedBlockRenderers`, a default-empty language-to-QML-Component
  map. Match the first whitespace-delimited token of the parser-decoded info string,
  case-sensitively. Empty or unmatched tokens use ordinary code. Updates require
  whole-map reassignment; nested map mutations are unsupported.
- R2: Preserve fenced-versus-indented origin and full parser-decoded info through
  private semantic and presentation models, including recursive lists/quotes.
  Indented blocks never dispatch, regardless of any apparent language text.
- R3: Each renderer must create an `Item` declaring required string properties
  `codeText`, `infoString`, `language`, and a required `markdownStyle` property.
  Supply parser-produced code unchanged, including whitespace and trailing newlines.
- R4: Replace only code content within the existing code surface. Preserve styling,
  padding and host action headers. Assign available content width and reserve finite
  nonnegative `implicitHeight`; support width and live style changes, including empty code.
- R5: When document selection is enabled, show ordinary selectable source instead
  of custom visuals, even with an empty selection. Selection and code-action copy
  requests continue to contain source text and retain host clipboard ownership.
- R6: Null entries, invalid Components, contract violations and creation failures
  fall back to ordinary code with diagnostics. Destroy instances on document or
  selected-component replacement and view teardown. Selection toggling must safely
  replace the content and permit recreation when selection is disabled.
- R7: Host components are trusted application code. Add no engine, network access,
  resource permission or public C++ API. Bundled highlighting, Mermaid, math and
  inline extensions remain deferred.
- R8: Provide a dependency-free native QML playground example and installed-consumer
  coverage. Preserve shared/static packaging, relocation, private parser boundaries
  and supported Qt targets.

## Scope boundary

Implementation retains package 1.7.0 / QML 1.7, including the new QML map.
A future version change remains a separate delivery decision. No tagging or publication is authorized. Planned tests
are listed in [verification](verification.md); they do not establish existing behavior.
