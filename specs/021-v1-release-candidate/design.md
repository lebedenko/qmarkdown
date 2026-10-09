# Design

Approved on 2026-10-09 by explicit user approval of requirements, design and tasks. Scope is the local candidate preparation and verification below; no commit, tag, push or publication.

## Parser correction

Patch only third_party/cmark/src/node.h and blocks.c: store fence_length as bufsize_t and assign the complete scanned opener length without the 255 clamp. The existing closer comparison then enforces the [CommonMark 0.31.2 fence rule](https://spec.commonmark.org/0.31.2/#fenced-code-blocks). This private storage change avoids upgrading the dependency or introducing adapter workarounds. Amend PROVENANCE.md to name both patched files and rationale while preserving acquisition details and notices.

Extend the existing Qt parser fixtures and CommonMark probe tests with independently source-derived HTML and model expectations. Generate fence strings from explicit lengths; never derive expected models from production output. First run corrected assertions against the old parser and retain failure evidence, then apply the fix. Supplementary examples remain outside the official corpus and ledger.

## Versions and compatibility

Update top-level project version and package-version policy, public qt_add_qml_module version/SOVERSION, private registrations/metadata, active imports, executable tests, installed consumers, examples and current integration documentation. Preserve historical specification versions and results. Use [CMake SameMajorVersion](https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html#generating-a-package-version-file): a requested version must share major 1 and not exceed the installed version. Thus this candidate accepts 1.0 and exact 1.0.0, but rejects 0.7, 1.1 and 2.0.

Public compatibility covers MarkdownView, MarkdownStyle and MarkdownResourcePolicy plus documented CMake targets/integration. Private classes/URI are explicitly excluded. SOVERSION identifies the new release line without promising public C++ ABI or portable binaries. Test explicit 1.0 and unversioned imports for all public types and reject 0.7; add package-request checks to existing packaging verification so pinned tasks exercise them too.

## Documentation and evidence

Add an unreleased 1.0 candidate changelog with import/package migration instructions. Update current project documents and viewer text to distinguish passed semantic/native evidence, accepted presentation/resource/lifecycle limits, supported environments and deferred work. Preserve iteration 020 and historical records. Report host Qt and pinned Ubuntu amd64 environments separately; do not turn offscreen results into desktop claims.

Reuse existing build, lint, privacy, packaging and task CI workflows. Run native view assertions using the Wayland backend, selecting layout/container/link/image-policy/teardown fixtures after inspection. Record actual platform, display and Qt details, assertions and any manual observations. An unavailable or failing desktop gate keeps readiness pending.

## Candidate snapshot

Stage a filtered source snapshot using an explicit inclusion/exclusion policy that includes untracked authorized iteration 020/021 sources and excludes .git, build/install/evidence outputs, credentials and secrets. Keep artifacts/evidence outside the staged source. Use a per-file SHA-256 inventory to compare implementation with successful host/pinned snapshots and extracted archive contents.

Complete final prose before archiving. Build extracted sources in fresh shared/static trees and run their verification/relocated consumers; retain installed trees and logs. Record final archive hash in a separate local manifest/evidence record to avoid self-referential hashes. If final source changes after verification, repeat affected gates and regenerate the archive. Candidate readiness requires every local gate; hosted execution is a separately documented follow-up, with no publication action in this iteration.

## Routine verification corrections within approved scope

Actual runs required test-only corrections: observe dedicated future completion without Qt global-pool waiting during application teardown; wait for aggregate image layout before sampling heights; wait for desktop exposure and move synthetic input away before a first-line link hit. These preserve lifecycle/presentation behavior and strengthen deterministic observation. Actual evidence is recorded separately in verification.md.
