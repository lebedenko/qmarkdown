# Verification — library maintainability

Status: implemented and verified on 2026-10-14.
The checklist below defines coverage; actual results are recorded separately.

## Planned checks

- V1: Parser/format-interval tests and CommonMark/GFM semantic fixtures; confirm
  named contracts preserve role values and payloads through existing view tests.
- V2: Resource lifecycle, cancellation, destruction, stale-result and ownership
  tests. Cover manager replacement and null/same-manager guards through observable
  lifetime/request behavior. Confirm parsed semantic data survives projection.
- V3: Import, view, styling and selection tests, including fractional scaling and
  style aliases/notifications/reset/override behavior; QML lint.
- V4: Preserve nested code actions, tables, image alt-text ranges, plain/formatted
  selection and gesture behavior. Run advisory existing rendering/inline benchmark
  smoke workloads after projection/component changes; record results without new
  timing thresholds.
- V5: Shared/static build, installed and relocated consumer checks, source archive
  checks and cmark symbol privacy; configured minimum/development Qt matrix using
  repository tooling. Confirm private components are bundled and no C++ headers
  become installed. Keep public module metadata consistent with existing API.
- V6: Documentation/version/link checker, local links in these specification files,
  and diff whitespace checks; manually compare API reference against declarations.

## Actual results

2026-10-14:

- Implemented named model/render/selection contracts, separate semantic and
  presentation trees, typed style values and explicit notification dispatch,
  private code/table/action components, internal layout helpers, parser RAII,
  mandatory policy references, documented/guarded manager ownership, and direct
  header dependencies. Added the public API reference and corrected current guides.
- Host Qt 6.12.0: full 16-target CTest suite passed during integration. QML lint
  passed with only the existing import-only example's unused-import information.
  The final private codeText rename also passed the relocated base consumer against
  the rebuilt host library; final matrix results below cover the complete runtime.
- Resource regressions cover manager replacement/destruction and ignored null/same
  assignments. Projection fixtures verify semantic image data survives projection.
  View tests reject public construction of extracted internal components. Geometry
  assertions now measure positions relative to code surfaces across private Loader
  boundaries. Existing preset/alias/reset and fractional selection coverage remains.
- Host Release rendering smoke output and complete inline benchmark output passed
  their JSON validators. Inline results also require equality with the previous
  algorithm. Advisory measurements ran alongside builds and are not performance
  claims. Model-replacement timing excludes the new projection-construction stage.
- Final source archive built from the working-tree allowlist, extracted, configured,
  built and installed on host Qt 6.12.0: PASS. Parser symbol check: 143 bundled
  definitions prefixed, no parser symbols/types exported. Artifacts:
  `build-archive-041-verified/`; build log: `/tmp/qm-archive-verified.log`.
- Initial container attempts exposed ownership-template parser type exports on the
  container compiler; hiding the cmark node forward declaration resolved this.
  Installed-consumer discovery exposed the extracted surface's ambiguous `text`
  property; renaming it to private `codeText` resolved the issue. Earlier failed
  evidence is retained separately, not represented as passing verification.
- Docker access initially failed under the sandbox. Approved Docker access enabled
  the configured matrix. Final runs use `build-ci-041-minimum-verified/` and
  `build-ci-041-development-verified/`. Their captured runtime, test and build sources
  match the working tree (generated Python caches excluded).
- Final configured Qt 6.8.0 and 6.12.0 matrix: PASS (exit 0 for both runners).
  Each ran all 16 CTest targets for shared and static builds: 64/64 total.
  QML lint, cmark symbol privacy, same-major/exact package requests, relocated
  public API consumers, shared plugin-only consumers, library-only builds and
  Release rendering benchmark smoke/JSON validation all passed. Retained runner
  results: `build-ci-041-minimum-verified/result.json` and
  `build-ci-041-development-verified/result.json`; corresponding `ci.log`,
  `packaging/` and `benchmarks/` directories contain detailed local evidence.
- Documentation checker, Feature 041 local-link validation and diff whitespace
  checks passed. No push, merge, tag or publication was performed.
