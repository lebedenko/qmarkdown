# Specifications

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
