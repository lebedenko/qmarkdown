# Specifications

## Current release

<!-- current-release:start -->
Package **1.13.0 / QML 1.13**. Release assets: [1.13.0 downloads](https://github.com/lebedenko/qmarkdown/releases/tag/v1.13.0).
It includes all five independent opt-in GFM extensions on CommonMark 0.31.2,
native tables, inline-code padding, opt-in document-wide desktop selection and
host-owned plain-text copy requests, custom fenced-block renderers, repeated-click
selection and host-owned selection edge autoscrolling. Exact GFM 0.29-gfm conformance is not claimed.
See the [release notes](../docs/release-notes-1.13.0.md) and
[installation instructions](../docs/release-install.md).

Linux amd64 requires Qt >=6.8, C++17 and CMake >=3.21. Configured verification
targets are Qt 6.8.0 and 6.12.0. Public QML behavior and CMake integration remain
compatible throughout 1.x. Package 1.13.0 accepts compatible 1.0–1.13 requests and
EXACT 1.13.0; exact earlier versions require historical packages. Native classes
and QMarkdown.Private remain private; no public C++ ABI is promised.
Hosts own visual identity, scrolling, navigation, clipboard and resource policy.

Reading tools include literal Find with independent highlights, a recursive heading
outline and explicit fragment navigation. Hosts own controls and scrolling. See
the [reader guide](../docs/reading-navigation.md).

QML 1.13 retains opt-in keyboard link traversal, visible focus and one accessible
Link per logical link, including text/image splits. Tab exits through the host
focus chain; Enter activates through host-owned link handling. Replaceable task
indicators include a rounded fallback; the playground demonstrates the active
Controls style. Keyboard traversal defaults off; accessible links remain available
without keyboard opt-in. Task indicators remain read-only. Private QML
responsibilities are refactored without a new public C++ surface.

Optional fenced-code syntax highlighting uses a private KSyntaxHighlighting backend.
It defaults off per view; source builds default the dependency off, while Arch
enables it and requires `syntax-highlighting>=6.10.0`. Hosts control a separate
Light/Dark token palette. See the [syntax guide](../docs/syntax-highlighting.md).

Code-block usability adds language-aware headers, view-wide wrapping, bounded
source viewports with internal scrolling and optional decorative source-line
numbers. Defaults retain wrapped natural-height source with numbers off. Hosts
own header composition, clipboard and scrollbar styling. See the
[code-block guide](../docs/code-blocks.md).

Native block math is opt-in per view, using a bundled private MicroTeX backend.
It supports exact `math` fences and standalone dollar-delimited display blocks;
selection/Find and failures show literal TeX source. See the [math guide](../docs/math.md).
<!-- current-release:end -->

## 1.11 syntax highlighting

[049 requirements](049-syntax-highlighting/requirements.md),
[design](049-syntax-highlighting/design.md), [tasks](049-syntax-highlighting/tasks.md)
and [actual verification](049-syntax-highlighting/verification.md) record the approved
optional backend, independent token palette, asynchronous rendering and release PR.

## 1.10 keyboard reading

[048 requirements](048-keyboard-reading/requirements.md),
[design](048-keyboard-reading/design.md), [mapped tasks](048-keyboard-reading/tasks.md)
and [verification](048-keyboard-reading/verification.md) specify keyboard link
traversal, focus presentation and per-link accessibility. Requirements, design and
tasks were approved on 2026-10-15. Implementation and local verification on both
configured Qt shared/static targets are complete. Release documentation and authorized delivery are tracked within feature 048;
see the [roadmap](roadmap.md) for delivered capabilities and unscheduled work.

## 1.10 task-indicator presentation

[047 requirements](047-task-indicator-presentation/requirements.md), [design](047-task-indicator-presentation/design.md), [tasks](047-task-indicator-presentation/tasks.md), and [actual verification](047-task-indicator-presentation/verification.md). Approved through the supplied implementation plan on 2026-10-15. Package 1.10.0 / QML 1.10 supersedes feature 035’s presentation contract only.



## 1.10 QML boundary refactor

[046: requirements](046-qml-boundaries/requirements.md),
[design](046-qml-boundaries/design.md), [tasks](046-qml-boundaries/tasks.md)
and [verification](046-qml-boundaries/verification.md) record the behavior-preserving
QML responsibility and private contract refactor. Requirements, design and tasks were approved on 2026-10-15. The refactor is
implemented and locally verified on both configured Qt shared/static targets.

## 1.9 reading and navigation

[045: requirements](045-reading-navigation/requirements.md),
[design](045-reading-navigation/design.md), [tasks](045-reading-navigation/tasks.md)
and [verification](045-reading-navigation/verification.md) record the reader toolkit
approved on 2026-10-15, implemented and locally verified on both configured Qt
targets. Merge, tagging and publication require separate authorization.

## 1.8 implementation and historical feature records

[044: 1.8 desktop reading improvements](044-v1.8-release/requirements.md) was explicitly approved on 2026-10-14 for implementation and verification. See its [design](044-v1.8-release/design.md),
[tasks](044-v1.8-release/tasks.md) and [verification plan](044-v1.8-release/verification.md).
It delivers renderers, repeated-click selection and host-owned selection autoscrolling
with the release documentation gate. Tagging and publication require separate authorization.

[043: release documentation gate](043-release-documentation-gate/requirements.md) was approved through the supplied implementation plan on 2026-10-14. See [design](043-release-documentation-gate/design.md), [tasks](043-release-documentation-gate/tasks.md) and [verification](043-release-documentation-gate/verification.md).

Historical 1.7.0 downloads exclude custom fenced-block renderers.

Feature 042: [custom fenced-block renderer requirements](042-custom-fenced-block-renderers/requirements.md),
**approved on 2026-10-14**. See [design](042-custom-fenced-block-renderers/design.md),
[mapped tasks](042-custom-fenced-block-renderers/tasks.md) and
[planned/actual verification](042-custom-fenced-block-renderers/verification.md).
Requirements, design and T1–T5 are authorized by the user response “approved”.
Host QML renderer dispatch is implemented and locally verified on both configured
Qt targets; protected delivery is recorded separately. Package 1.7.0 / QML 1.7 is retained; tagging and publication
are outside this change.

The [development workflow](../docs/development-workflow.md) governs current delivery.

## Historical records

Current workflow change: [039: simplified CI and release delivery](039-simplified-delivery/requirements.md), approved through the supplied implementation plan on 2026-10-13. Historical records below retain their original scope.

Feature 041: [library maintainability requirements](041-library-maintainability/requirements.md),
approved on 2026-10-14; implemented and locally verified. See [design](041-library-maintainability/design.md),
[mapped tasks](041-library-maintainability/tasks.md) and
[planned/actual verification](041-library-maintainability/verification.md).
Addresses the project review while preserving package 1.7.0 / QML 1.7 behavior.

Feature 040: [document-wide selection requirements](040-document-selection/requirements.md),
**approved on 2026-10-14; implemented and locally verified**. See [design](040-document-selection/design.md),
[mapped tasks](040-document-selection/tasks.md) and [planned/actual verification](040-document-selection/verification.md).
Targets package 1.7.0 / QML 1.7; implementation and protected squash delivery are authorized; publication is excluded.

Feature 021: [1.0 release candidate requirements](021-v1-release-candidate/requirements.md), **approved on 2026-10-09; implemented and locally verified**. See [design](021-v1-release-candidate/design.md), [mapped tasks](021-v1-release-candidate/tasks.md) and [planned/actual verification](021-v1-release-candidate/verification.md). Prepares Linux amd64 package 1.0.0 / QML 1.0, a private long-fence correction, compatibility policy and verified local candidate artifacts; actual candidate gates are recorded separately; no release publication is authorized.

Repository-owned specifications are the source of agreed scope. The initial design input was `/tmp/qt-markdown.md`; that temporary brief is not required to read or maintain these documents.

- [Project overview](overview.md): accepted architectural direction and unresolved project decisions.
- [Standards and conformance](standards.md): pinned CommonMark release gate, selected extensions, and future verification.
- [Roadmap](roadmap.md): staged intentions, not delivery commitments.
- [001: static text requirements](001-static-text/requirements.md)
- [001: design](001-static-text/design.md)
- [001: tasks](001-static-text/tasks.md)
- [001: verification](001-static-text/verification.md)
- [002: project scaffold requirements](002-project-scaffold/requirements.md)
- [002: design](002-project-scaffold/design.md)
- [002: tasks](002-project-scaffold/tasks.md)
- [002: verification](002-project-scaffold/verification.md)
- [003: inline formatting requirements (implemented and verified)](003-inline-formatting/requirements.md)
- [003: design](003-inline-formatting/design.md)
- [003: tasks](003-inline-formatting/tasks.md)
- [003: verification](003-inline-formatting/verification.md)

Each numbered feature contains these four records. Requirements define observable behavior; design describes how to deliver it; tasks trace the work to requirements; verification distinguishes planned checks from recorded results. Feature 001 requirements, design, and tasks were approved on 2026-10-07 and its bounded renderer is implemented; feature 002 scaffolding was approved on 2026-10-07. Feature 003 was approved on 2026-10-07 and implements the bounded inline-formatting slice in version 0.2.0. Future features must repeat the approval cycle defined in [AGENTS.md](../AGENTS.md).

- [004: fenced code block requirements (implemented and verified)](004-fenced-code-blocks/requirements.md)
- [004: design](004-fenced-code-blocks/design.md)
- [004: tasks](004-fenced-code-blocks/tasks.md)
- [004: verification](004-fenced-code-blocks/verification.md)

- [005: demo playground requirements](005-demo-playground/requirements.md)
- [005: design](005-demo-playground/design.md)
- [005: tasks](005-demo-playground/tasks.md)
- [005: verification](005-demo-playground/verification.md)

- [006: core leaf block requirements](006-core-leaf-blocks/requirements.md)
- [006: design](006-core-leaf-blocks/design.md)
- [006: tasks](006-core-leaf-blocks/tasks.md)
- [006: verification](006-core-leaf-blocks/verification.md)

Feature 007, approved for implementation on 2026-10-08, supersedes unsupported-container/resource-source fallback with native recursive lists/quotes and inert formatted labels in package 0.5.0 / module 0.5. Production parsing now adapts one cmark tree. Hard breaks and reference definitions follow cmark; HTML stays literal. Full conformance and resource rendering remain deferred. See [requirements](007-container-blocks/requirements.md), [design](007-container-blocks/design.md), [tasks](007-container-blocks/tasks.md) and [verification](007-container-blocks/verification.md).

- [007: native container requirements](007-container-blocks/requirements.md)
- [007: design](007-container-blocks/design.md)
- [007: tasks](007-container-blocks/tasks.md)
- [007: verification](007-container-blocks/verification.md)

- [008: font units and application typography](008-font-units/requirements.md)
- [008: design](008-font-units/design.md)
- [008: tasks](008-font-units/tasks.md)
- [008: verification](008-font-units/verification.md)

Feature 008 was approved on 2026-10-08 through the explicit implementation request. It supersedes earlier fixed pixel font defaults; historical approvals remain intact.

Feature 009: [host-controlled links](009-host-controlled-links/requirements.md), approved on 2026-10-08 for 0.6.0/0.6. See [design](009-host-controlled-links/design.md), [tasks](009-host-controlled-links/tasks.md), and [verification](009-host-controlled-links/verification.md). Hosts own navigation; resource rendering remains deferred.

Feature 010: [images and host resource policy](010-images-and-resource-policy/requirements.md), approved on 2026-10-08 for 0.7.0/0.7. See design, tasks and verification in that directory.

Feature 011: [CommonMark conformance baseline requirements](011-commonmark-baseline/requirements.md), **approved on 2026-10-08; implemented and verified**. See [design](011-commonmark-baseline/design.md), [tasks](011-commonmark-baseline/tasks.md), and [verification](011-commonmark-baseline/verification.md). Test-infrastructure iteration; full conformance remains unverified.

Feature 012: [verification hardening requirements](012-verification-hardening/requirements.md), approved on 2026-10-08 by the explicit implementation request. See [design](012-verification-hardening/design.md), [tasks](012-verification-hardening/tasks.md) and [actual verification](012-verification-hardening/verification.md). Package/module remain 0.7.0/0.7; CI target compatibility awaits successful workflow execution.

Feature 013: [efficient inline format preparation](013-inline-format-performance/requirements.md), approved on 2026-10-08 through the explicit implementation request. See [design](013-inline-format-performance/design.md), [tasks](013-inline-format-performance/tasks.md) and [verification](013-inline-format-performance/verification.md). Scope is preparation only; package/module remain 0.7.0/0.7.

Feature 014: [local CI tasks](014-local-ci/requirements.md), approved on 2026-10-09; implemented and locally verified for both Qt versions. See [design](014-local-ci/design.md), [tasks](014-local-ci/tasks.md) and [verification](014-local-ci/verification.md). Uses the same pinned Ubuntu container and shared verification commands for local tasks and GitHub; remote execution remains pending.

Feature 015: [close remaining oracle gaps](015-oracle-gaps/requirements.md), approved on 2026-10-09 through the explicit implementation request. See [design](015-oracle-gaps/design.md), [tasks](015-oracle-gaps/tasks.md) and [verification](015-oracle-gaps/verification.md). Independent expectations cover all official examples; Feature 016 subsequently adds inline semantic retention while full conformance remains unverified. Tests/documentation only, with versions unchanged at 0.7.0/0.7.

Feature 016: [preserve inline semantics](016-inline-semantic-retention/requirements.md), approved on 2026-10-09 through the explicit implementation request; implemented and locally verified on host/shared/static and both Qt CI versions. See [design](016-inline-semantic-retention/design.md), [tasks](016-inline-semantic-retention/tasks.md), [verification](016-inline-semantic-retention/verification.md) and [individual ledger review](016-inline-semantic-retention/ledger-review.md). Private owned inline trees retain all five known losses with unchanged native presentation and 0.7.0/0.7 interfaces/versions. Remaining independent evidence limits keep the v1.0 gate open.

Feature 017: [complete independent semantic evidence](017-complete-semantic-evidence/requirements.md), approved on 2026-10-09 through the explicit implementation request. See [design](017-complete-semantic-evidence/design.md), [tasks](017-complete-semantic-evidence/tasks.md), [verification](017-complete-semantic-evidence/verification.md) and [individual ledger review](017-complete-semantic-evidence/ledger-review.json). Complete source checks supplement all 390 previously limited HTML checks; corpus semantics and v1.0 release readiness remain distinct. Production behavior and 0.7.0/0.7 versions remain unchanged.

Feature 018: [cache CI toolchain images](018-cache-ci-toolchains/requirements.md), approved on 2026-10-09 through the explicit implementation request. See [design](018-cache-ci-toolchains/design.md), [tasks](018-cache-ci-toolchains/tasks.md) and [actual verification](018-cache-ci-toolchains/verification.md). Library interfaces and versions remain unchanged.

Feature 019: [native presentation evidence](019-native-presentation-evidence/requirements.md), **approved on 2026-10-09; implemented and verified**. See [design](019-native-presentation-evidence/design.md), [tasks](019-native-presentation-evidence/tasks.md) and [verification plan](019-native-presentation-evidence/verification.md). Adds an assertion-level [native coverage assessment](019-native-presentation-evidence/coverage.md), focused regression checks and current-status documentation corrections; host shared/static and both pinned Qt CI tasks pass. Production changes and v1.0 release decisions remain outside scope.

Feature 020: [responsive image-decoder lifecycle](020-responsive-decoder-lifecycle/requirements.md), **approved on 2026-10-09; implemented and verified**. See [design](020-responsive-decoder-lifecycle/design.md), [tasks](020-responsive-decoder-lifecycle/tasks.md) and [planned/actual verification](020-responsive-decoder-lifecycle/verification.md). Implements an application-owned two-worker FIFO scheduler and responsive controller/view/engine teardown; host shared/static and both pinned Qt CI tasks pass with unchanged public APIs and 0.7.0/0.7 versions. Application teardown may wait for active codecs.

Feature 022: [release packaging](022-release-packaging/requirements.md), approved on 2026-10-10 through the explicit implementation request. See [design](022-release-packaging/design.md), [tasks](022-release-packaging/tasks.md) and [actual verification](022-release-packaging/verification.md). Tag and first-release creation remain separate.

Feature 023: [stable 1.0.0 release](023-stable-release/requirements.md), approved on 2026-10-10 through the supplied release plan. See [design](023-stable-release/design.md), [tasks](023-stable-release/tasks.md) and [verification](023-stable-release/verification.md). Includes protected PR merge, exact-commit verification, tagging, publication and published-asset checks.

Feature 024: [development-host Qt verification](024-development-host-qt/requirements.md), approved on 2026-10-11 through the supplied implementation plan. See design, tasks and actual verification in that directory. Implemented and verified on Qt 6.8.0 and 6.12.0; both hosted role checks and release cross-runtime checks pass, and the required-context migration is applied.

Feature 025: [styling and presets](025-styling-presets/requirements.md), approved on 2026-10-11 through the supplied implementation plan. See design, tasks and actual verification in that directory. Styling is implemented and verified on host shared/static, both configured Qt container targets, hosted packaging and same-archive runtime checks. Integration into main completed in iteration 026; publication completed in iteration 027.

Feature 026: [v1.1 integration](026-v1.1-integration/requirements.md), approved on 2026-10-11 through the supplied implementation plan. See [design](026-v1.1-integration/design.md), [mapped tasks](026-v1.1-integration/tasks.md) and [planned/actual verification](026-v1.1-integration/verification.md). Integrates CI and styling through protected merge commits, prepares release notes and verifies the exact final main candidate on Qt 6.8.0/6.12.0. Tagging and publication subsequently completed in iteration 027.

Feature 027: [stable v1.1.0 publication](027-stable-v1.1-release/requirements.md), approved on 2026-10-11 through the supplied implementation plan. See [design](027-stable-v1.1-release/design.md), [mapped tasks](027-stable-v1.1-release/tasks.md) and [planned/actual verification](027-stable-v1.1-release/verification.md). Includes candidate-evidence integration, release documentation, fresh exact-commit checks, annotated tagging, stable/latest publication and downloaded-asset verification on Qt 6.8.0/6.12.0.

Feature 028: [post-release upkeep requirements](028-post-release-upkeep/requirements.md), approved on 2026-10-12 through the supplied implementation plan. See [design](028-post-release-upkeep/design.md), [mapped tasks](028-post-release-upkeep/tasks.md) and [planned/actual verification](028-post-release-upkeep/verification.md). Documentation-only evidence integration and current-status reconciliation, including protected merge; no new release or feature selection.

- [029: Arch release packaging requirements](029-arch-release-packaging/requirements.md), approved on 2026-10-12. See [design](029-arch-release-packaging/design.md), [mapped tasks](029-arch-release-packaging/tasks.md) and [planned/actual verification](029-arch-release-packaging/verification.md). Future stable release tooling; existing published assets remain unchanged.

Feature 030: [Arch packaging integration](030-arch-packaging-integration/requirements.md), approved on 2026-10-12 through the supplied implementation plan. See [design](030-arch-packaging-integration/design.md), [mapped tasks](030-arch-packaging-integration/tasks.md) and [verification](030-arch-packaging-integration/verification.md). Completion requires fresh final-PR and exact-main distribution gates; final evidence is retained separately.

Iteration 030 final exact-main gates passed for `45fd78e973b64b2f719d5bc07b4f6e0c22225b29`; final results and retained logs are published separately on `docs/arch-packaging-integration-evidence`. Existing v1.1.0 assets remain unchanged.

Iteration 031: [host-controlled code-block copy actions](031-code-block-copy-actions/requirements.md), approved on 2026-10-12 under the original local iteration-030 plan; renumbered because main owns 030 for Arch integration. Implementation and historical local verification are retained in [design](031-code-block-copy-actions/design.md), [tasks](031-code-block-copy-actions/tasks.md) and [verification](031-code-block-copy-actions/verification.md). Package 1.2.0/QML 1.2 is integrated; publication completed in iteration 033.

Iteration 032: [1.2 candidate integration](032-v1.2-integration/requirements.md), approved on 2026-10-12 through the supplied plan. See [design](032-v1.2-integration/design.md), [mapped tasks](032-v1.2-integration/tasks.md) and [planned/actual verification](032-v1.2-integration/verification.md). Integration is complete; iteration 033 records successful stable/latest v1.2.0 publication and independent downloaded-asset acceptance.

Iteration 032 final candidate gates passed on `a65f537e5b2134e026bf328a1ec4a307c48588ef`. [Actual results](032-v1.2-integration/verification.md) and machine-readable evidence were retained on `docs/v1.2-integration-evidence` and are integrated by iteration 033. Moving main requires fresh release gates. Stable/latest v1.2.0 publication subsequently completed in iteration 033.

Iteration 033: [stable v1.2.0 publication](033-stable-v1.2-release/requirements.md), approved on 2026-10-12 through the supplied plan. See [design](033-stable-v1.2-release/design.md), [mapped tasks](033-stable-v1.2-release/tasks.md) and [planned/actual verification](033-stable-v1.2-release/verification.md). Includes protected release preparation, fresh exact-main gates, annotated tagging, Ubuntu/Arch stable/latest publication and independent downloaded-asset acceptance. Publication and independent downloaded-asset acceptance are complete.

Iteration 034: [post-v1.2 release upkeep](034-post-v1.2-release-upkeep/requirements.md), approved on 2026-10-12 through the supplied plan. See [design](034-post-v1.2-release-upkeep/design.md), [mapped tasks](034-post-v1.2-release-upkeep/tasks.md) and [planned/actual verification](034-post-v1.2-release-upkeep/verification.md). Integrates completed publication evidence and reconciles current status through protected merge; no new feature or release.

## 036 — Opt-in GFM strikethrough

Approved for implementation and verification on 2026-10-13. [Requirements](036-gfm-strikethrough/requirements.md), [design](036-gfm-strikethrough/design.md), [tasks](036-gfm-strikethrough/tasks.md), [verification](036-gfm-strikethrough/verification.md). Candidate 1.4.0 / QML 1.4 is unpublished.

## 1.12 code-block usability

Features [050 headers](050-language-aware-headers/requirements.md),
[051 wrapping](051-code-block-wrapping/requirements.md),
[052 scrolling](052-code-block-scrolling/requirements.md), and
[053 line numbers](053-code-block-line-numbers/requirements.md) implement the
approved 2026-10-16 plan in one release change. Each directory retains requirements,
design, mapped tasks and actual verification separately. Merge, tagging and
publication of 1.12 require separate authorization.

## 054 — Text rendering sharpness

Approved on 2026-10-16 through the supplied implementation plan.
[Requirements](054-text-rendering-sharpness/requirements.md),
[design](054-text-rendering-sharpness/design.md),
[mapped tasks](054-text-rendering-sharpness/tasks.md), and
[actual verification](054-text-rendering-sharpness/verification.md).
Bounded texture-sampling correction; no system-package replacement or release publication.

## Approved 1.13 native block math

Features [055 native fenced math](055-native-fenced-math/requirements.md) and
[056 standalone display math](056-standalone-display-math/requirements.md) are
approved for implementation on 2026-10-16. Each directory includes design, mapped tasks
and verification records. Implementation, both Qt candidate matrices, native Wayland acceptance and
distribution checks are complete; final PR-head gates remain required. The proposed delivery is one 1.13.0 / QML 1.13 release PR.
Merge, tagging and publication require separate authorization.
