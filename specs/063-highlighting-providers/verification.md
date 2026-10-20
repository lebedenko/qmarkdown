# 063 — Verification

## Planned checks

- Provider/request: exact source/language, all twelve roles, immediate and async
  completion, empty success, decline, invalid arrays/fields/roles/ranges/overlaps/
  surrogate boundaries, sorted spans and real five-second timeout.
- Lifecycle: replacement/disablement/delegate/view/provider destruction, duplicate
  and stale completion, cancellation flag, retained async wrappers, shared providers,
  partial/all-subscriber cancellation, reentrant invalidate/replace/destruction.
- Cache/presentation: identity/generation/source/language isolation, invalidation,
  success-empty caching, limits, live palette recoloring without retokenization,
  unknown languages, custom visual/source fallback, indented/plain cases, math and
  footnote bodies, exact copy/Find offsets.
- Delivery: built-in ON/OFF and availability semantics, older imports, public-QML
  installed shared/static consumers, playground example, source archive, affected
  suites, lint/docs and exact Qt 6.8.0/6.12.0 packaging. Record native-input acceptance
  independently from offscreen automation.

## Actual results

The user approved requirements, design and T1–T7 on 2026-10-20, conditional on
Feature 062's required CI. That dependency passed minimum Qt 6.8.0, development
Qt 6.12.0 and release-gate at 9398444 before implementation began:
[Feature 062 CI](https://github.com/lebedenko/qmarkdown/actions/runs/39328551808).
No merge or publication of the dependency was performed.

Implementation and focused host Qt 6.12.0 verification on 2026-10-20:

- Eight affected suites pass: imports, view, custom renderers, highlighting units,
  highlighting view, math, footnotes and providers (`/tmp/063-affected.log`).
- Final provider/highlighting focused checks pass, including strict numeric and
  UTF-16 validation, all twelve roles, empty success, decline/invalid fallback,
  real timeout and a blocked-thread deadline, cache entry/memory limits,
  coalescing, identity/generation isolation and reentrant invalidate/destruction.
- Built-in highlighting ON focused checks pass 5/5 (`/tmp/063-final-highlight.log`).
  Both optional backends OFF checks pass 6/6 (`/tmp/063-final-off.log`). The provider
  view checks run even without the compiled highlighting backend.
- Native QML tests pass for palette-only recoloring, exact source/copy/Find,
  custom visual/source precedence, footnote code, provider/view destruction and
  host-retained wrappers after cancellation. A discovered Qt child-wrapper
  invalidation issue was fixed by keeping pending requests outside the provider's
  QObject subtree. `/tmp/063-providers.png` was visually inspected; this is
  offscreen evidence, not native-input acceptance.
- Initial backend-ON fallback tests exposed an unsafe test assertion indexing
  asynchronous empty ranges; it was corrected to wait before indexing. The final
  backend-ON checks pass. No passing claim relies on that failed run.
- The working-tree 1.17.0 source archive includes both new private provider files.
  Host toolchain sync remains 6.12.0. Ordinary documentation and whitespace checks
  pass. `all_qmllint` passes (`/tmp/063-lint.log`), retaining only the existing
  import-only example’s unused-import informational message. Package/QML development metadata is 1.17.0 / 1.17; published 1.14 records
  remain unchanged.

Full shared/static packaging, relocated public-QML consumers and final-head
hosted Qt gates are delivery checks. Their final results are recorded in the
implementation PR, preserving final-head evidence without an evidence-only source
commit. At this record's creation the full local packaging driver is still running;
focused passes above do not imply those remaining gates have completed.

The unchanged local Docker/buildx preparation barrier recorded in Feature 061
prevents local exact-container gates; hosted Qt 6.8.0/6.12.0 gates remain required.
Native desktop input acceptance is unavailable in this session. Merge, tagging
and publication remain excluded.

## 1.17.0 release delivery

On 2026-10-20 the user requested “release the latest version”, authorizing the
release PR, squash merge, tagging and publication of the existing 1.17.0 package,
including approved features 061–063. This extends the earlier delivery exclusion;
no runtime scope changes are included. Delivery follows docs/development-workflow.md:
finalize current documentation and changelog, require final-head full Qt CI,
squash merge, dispatch release.yml, and require downloaded-asset hash verification.

Release preparation checks on 2026-10-20:
- Host Qt synchronization resolves to unchanged Qt 6.12.0.
- `python3 scripts/check-docs.py --release` passes for 1.17.0.
- `git diff --check` passes.
- `python3 -m unittest discover -s tests -p 'test_release_*.py'` passes 53 tests.
  Publication messages in this test output use fixtures; they are not evidence of
  an actual 1.17.0 publication.

Final full Qt matrix and distribution/publication results belong to the release
PR and linked Actions runs. These local results do not claim those gates passed.
