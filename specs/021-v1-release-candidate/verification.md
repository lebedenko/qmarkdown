# Verification

Approved on 2026-10-09 by explicit user approval of requirements, design and tasks. Planned checks and actual execution remain separate.

## Planned checks

| Requirements | Checks and acceptance |
| --- | --- |
| R1,R6 | Backticks and tildes with opener lengths 254, 255, 256, 300, 4096; shorter/equal/longer closers, unclosed input, trailing content, LF/CRLF/CR, representative list/quote containers and mismatched closer characters. Retain pre-fix failures and post-fix passes. Supplementary source-derived HTML/model expectations; official fixtures/ledger unchanged. |
| R2,R3 | Explicit 1.0 and unversioned imports instantiate all three public QML types; 0.7 imports fail. CMake 1.0 and EXACT 1.0.0 succeed; 0.7, 1.1, 2.0 fail. Private metadata/runtime agree; no installed C++ headers or leaked parser symbols. |
| R4,R6 | Focused parser/import/resource/view checks first, then shared/static CTest, QML lint, privacy, relocated consumers, task ci-6.8 and task ci-6.11. Zero failures. Host Qt/toolchain recorded separately from pinned Ubuntu amd64 Qt 6.8.0/6.11.3. |
| R1,R6 | python3 scripts/verify-commonmark.py --strict: 652 official parser and model passes, zero unresolved mismatches/uncheckable cases/limits/losses. Retain reports and supplementary test counts separately. |
| R5,R7 | Native Wayland layout, containers, links, policy transitions and decoder teardown; record display/backend/Qt, commands, assertions and observations. Offscreen evidence remains separate; unavailable desktop checks leave gate pending. |
| R3,R5,R9 | Review active docs/migration/limits and historical preservation; Markdown links and git diff --check pass. Record build warnings without claiming warning-free verification. |
| R8,R9 | Final filtered archive includes documentation/vendor notices and no metadata/build/sensitive outputs; SHA-256 manifest and file inventory. Tested implementation snapshot matches archive. Extracted-source shared/static builds, tests and relocated consumers pass; retain install trees/logs. |

## Actual results (2026-10-09)

Explicit user approval preceded implementation. Package/module are 1.0.0/1.0, library SOVERSION is 1, and the generated package version uses SameMajorVersion. Private runtime registrations and metadata use 1.0 (encoded revision 256). No C++ headers are installed. Iteration 020 scheduler/controller changes are preserved; production behavior changes only for the two-file long-fence correction and versioning.

### Defect reproduction and focused checks

Corrected the existing `fences:long` expectation and added 150 Qt rows covering two markers, five lengths (254, 255, 256, 300, 4096), shorter/equal/longer/unclosed/wrong-character closers, trailing content and LF/CRLF/CR. Each row checks top-level, quote and list models. Supplementary probe expectations cover **450 inputs** across the same cases/endings/containers, with source-derived HTML and complete models outside the official corpus/ledger.

Pre-fix command `build-shared/tests/qmarkdown-parser-test fences longFences -o /tmp/021-prefix-parser.txt,txt`: **149 passed, 19 failed**. The 18 long-opener/shorter-closer rows plus the existing long case reproduce premature closure. `python3 tests/commonmark/test_baseline.py --probe build-shared/tests/qmarkdown-commonmark-probe` with the original storage/clamp and corrected supplementary expectations: **57 tests, 54 failed subcases**, exit 1; failures are the three lengths over 255 × two markers × three endings × three containers with shorter closers. Logs: `021-prefix-probe-corrected.txt` and `021-prefix-parser.txt` in retained local evidence. Initial fixture-authoring errors (Quote spelling, trailing container prefix and tight-list HTML newline) were corrected before this recorded reproduction.

Post-fix focused parser: **168 passed, zero failed/skipped**, including init/cleanup. Probe suite: **57 tests passed**, including all 450 supplementary inputs. Full parser suite now has **271 passes**. Official fixture/provenance/spec/ledger and upstream cmark version/checksum/notices/symbol prefixing are unchanged; vendor changes are limited to node.h and blocks.c, documented in bundled-parser provenance.

Explicit `1.0` and unversioned imports instantiate MarkdownView, MarkdownStyle and MarkdownResourcePolicy; all three reject `0.7`. Full import suite: **13 passes**, zero failures/skips. Active older 0.5 and 0.7 test imports now request 1.0; historical specification versions are preserved. Shared/static installed package requests accept 1.0 and EXACT 1.0.0, reject 0.7, 1.1 and 2.0, with retained configure logs verifying version rejection rather than an unrelated configuration error.

Focused offscreen resource/view checks, full resource suite (**27 passes**) and final link/image subset (**9 passes**) pass. Counts include init/cleanup. Malformed PNG rejection retains the expected libpng Read Error diagnostic.

### Verification fixture corrections

Repeated shutdown-child execution exposed an intermittent SIGSEGV. Retained debugger traces locate it in Qt's `QFutureInterfaceBase::waitForFinished()` from the test's release thread during application teardown. Replace that test-only wait with a bounded observation of the dedicated job's finished/cancelled state, avoiding consultation of Qt's global pool during its teardown. **100 separate child executions pass**, retaining the original cancellation/drain assertions and bounded cleanup. No scheduler behavior or public API change is introduced. [Qt 6.8 source](https://github.com/qt/qtbase/blob/v6.8.0/src/corelib/thread/qfutureinterface.cpp#L461) confirms that an unfinished future wait consults its pool; bounded state observation avoids that path during teardown.

A fresh host packaging run exposed sampling of the image document's height before the enclosing Column was polished. The fixture now waits for both text rows and the aggregate native row geometry before saving the initial height, and waits for the narrowed aggregate height. **20 image-policy/layout repetitions pass**.

Native desktop runs exposed synthetic hover reuse across newly created windows. Link checks now wait for window exposure, assert the chosen first-line point hits the link and move away before moving to that point. All six link cases pass on Wayland and offscreen. These are routine verification fixes within approved scope; original behavior/assertions remain covered.

### Host and pinned regression gates

Host: Linux x86_64, Qt **6.12.0**, GCC **16.2.1**, CMake **4.4.4**, Python **3.14.7** (exact queried versions are retained in environment evidence). Host results are separate from the supported minimum/pinned environments.

Commands and final results:

```sh
cmake -S . -B build-shared -DBUILD_TESTING=ON -DQMARKDOWN_BUILD_EXAMPLES=ON
cmake --build build-shared --parallel 2
ctest --test-dir build-shared --output-on-failure
cmake --build build-shared --target all_qmllint
python3 scripts/verify-cmark-symbols.py build-shared
python3 scripts/verify-commonmark.py --strict --report /tmp/021-strict-final.json
python3 scripts/verify-packaging.py --work-dir build-release-021/host-accepted
```

Final host shared CTest: **7/7 passed**, exit 0, **25.55 s**. QML lint passes with existing unused-import information; privacy check passes with **140 prefixed external cmark definitions**, no public parser exports. Strict corpus: **652 parser and 652 model passes**, zero mismatches, uncheckable cases, unresolved comparison limits or semantic losses, exit 0. All retained final shared/static corpus reports satisfy the same strict condition.

Host packaging run passes shared/static CTest **7/7 each** (26.04/25.71 s), lint/privacy, installed/relocated direct-link/plugin-only/static consumers, ten version-request decisions, notices/header privacy and library-only build. It predates the final test-only desktop input refinements; final host shared and pinned shared/static suites below exercise those refinements. Final extracted-source verification is tracked separately below.

Both final pinned runs use cached immutable toolchain images, fresh read-only source snapshots and the same GitHub verification commands. Sandbox attempts failed to access the Docker socket (Task exit 201); explicitly approved reruns pass.

| Task | Qt / result | Shared/static CTest | Elapsed | Local evidence |
| --- | --- | --- | --- | --- |
| task ci-6.8 | 6.8.0; exit 0 | 7/7 each; 24.45/24.21 s | 119.996 s | build-ci/6.8.0/20261009T194827Z-xz6kdptq/ |
| task ci-6.11 | 6.11.3; exit 0 | 7/7 each; 24.38/24.27 s | 128.630 s | build-ci/6.11.3/20261009T194818Z-i0rikxg4/ |

All four pinned suites have **14 format-interval, 13 import, 271 parser, 50 view and 27 resource passes**, zero failures/skips, plus **57 Python oracle tests** and strict 652-example corpus passes. Both tasks pass shared/static QML lint, parser symbol privacy, relocated direct-link/plugin-only/static consumers, all package requests, notices/no installed headers, library-only builds and validated advisory Release benchmark smoke checks. Image preparation takes 0.012 s each. result.json retains image IDs/fingerprints and timing; environment.json/tool-versions.txt, ci.log, packaging install trees/reports and benchmarks remain local.

Pinned environment: Ubuntu 24.04 amd64, GCC 13.3.0, CMake 3.28.3, Python 3.12.3, offscreen/Fusion, UID/GID 1000, bundled cmark 0.31.2. Existing Qt 6.8 QML type-resolution warnings, Qt 6.11 optional Qt6TaskTree discovery warnings, unavailable Vulkan headers and unused-import information remain. Warning-free builds are not claimed. Earlier authoring attempts failed for legacy 0.5 imports, fixture timing and CMake diagnostic wording; subsequent final snapshots pass all gates.

### Native desktop gate

The sandbox could not access Wayland (exit 134); an approved native run then exposed a host Qt text-input-v3 warning. `QT_IM_MODULE=compose` avoids that backend warning without suppressing test warnings. Native checks continue to use failOnWarning(). Final command:

```sh
QT_QPA_PLATFORM=wayland QT_IM_MODULE=compose QML_IMPORT_PATH="$PWD/build-shared/qml" build-shared/tests/qmarkdown-view-test nativeLayoutAndReplacement containerLayoutAndStyle leafLayoutAndStyle fencedLayoutAndStyle linkInteraction imageRowsAndPolicyLifecycle activeDecodeTeardown -o /tmp/021-wayland-release.txt,txt
```

**15 passed, zero failed/skipped**, exit 0, **1.684 s**, including init/cleanup. Coverage: wrapping/replacement/narrow widths, lists/quotes/empty containers, Setext/rules/indented/fenced code, all six link cases (paragraph/heading/nested/empty/autolink/image label), PNG policy admission/revocation/replacement, image geometry and view/engine teardown while decoding is held. Assertions run against shown native desktop windows; this is distinct from offscreen evidence.

Desktop: host Qt 6.12.0, Qt platform backend **wayland**, Wayland display **wayland-1**, Hyprland session, eDP-1 Sharp LQ180R1JW01 **2560×1600 at 240.001 Hz, scale 1.25**, XRGB8888/sRGB. Host Quick Controls style environment is Holonight; this is an external desktop setting, not a library dependency. A separate QSG_INFO=1 layout check passes (3 tests) without emitted scenegraph diagnostics; no specific GPU API or pixel-output review is claimed.

### Archive and source gates

A filtered preview archive contains **220 files**, including untracked authorized iteration 020/021 sources, changelog, specifications, bundled cmark and required license notices. Snapshot input is Git's tracked and nonignored working files, excluding Git metadata, build outputs, private tool/account directories, .env files, keys and credentials. A per-file SHA-256 inventory verifies extraction contents.

Preview extracted-source command:

```sh
python3 build-release-021/preview/extracted/qmarkdown-1.0.0/scripts/verify-packaging.py --work-dir build-release-021/preview-verification
```

Exit 0: shared/static CTest **7/7 each** (27.67/24.55 s), lint/privacy, all ten package decisions, relocated consumers, notices/header privacy and library-only builds pass. Preview implementation matches final pinned implementation; final test-only desktop fixture changes are covered by the final pinned snapshots. Preview is evidence, not the final candidate artifact.

Fresh extracted final-source verification also passes:

```sh
python3 build-release-021/source-verified/extracted/qmarkdown-1.0.0/scripts/verify-packaging.py --work-dir build-release-021/extracted-verified
```

Exit 0: shared/static CTest **7/7 each** (25.51/24.81 s), lint/privacy, all ten version decisions, relocated direct-link/plugin-only/static consumers, notices/header privacy and library-only builds pass. This snapshot includes every final implementation/test/tooling change, byte-identical to both final pinned source snapshots (**118 files**). Documentation is finalized afterward; implementation is frozen.

Final source artifact: `build-release-021/candidate/qmarkdown-1.0.0-candidate.tar.gz`, with `SHA256SUMS` and `source-sha256.json` alongside it. Final extraction is retained under `build-release-021/candidate/extracted/qmarkdown-1.0.0/`. A final packaging recheck runs from that extraction into `build-release-021/candidate-verification/`; shared/static relocated install trees, CTest/lint/privacy/version/consumer evidence and strict reports remain there. Final archive SHA-256, exact recheck command/exit status, exclusion/notices checks, per-file extraction/snapshot comparisons and local readiness are recorded in `build-release-021/candidate-record.json` outside the archive, avoiding a self-referential archive hash. That external record is the final artifact gate authority; candidate completion requires its status to be passed.

Final review: `git diff --check` and current-document Markdown link checks (excluding illustrative code links) pass. Official corpus fixtures/spec/ledger/provenance, cmark COPYING/symbol prefixing and AGENTS.md match HEAD. Historical feature records remain intact. All local candidate gates are covered by retained actual evidence; hosted execution and release publication remain separate follow-ups.

### Scope and remaining limits

Changed files: package/module/private registration and metadata configuration; two cmark source files and provenance; parser/probe/import/view/resource/installed-consumer tests; packaging verification; CHANGELOG, README, specification index, overview/standards/roadmap, current native coverage and iteration 021 documents. Existing iteration 020 source/controller changes and records are preserved. No commit, tag, push or publication is performed.

Supported candidate environment is Linux amd64 with compatible Qt >=6.8/toolchains. Public QML/CMake compatibility throughout 1.x excludes native/private classes and C++ ABI; portable prebuilt binaries are not promised. Separate image rows, PNG/JPEG restrictions, literal HTML, full replacement, narrow-glyph overhang and uninterruptible codecs remain accepted limits. Two occupied decoder workers can delay fresh work; shutdown may wait for codecs. Optional syntax, streaming, selection/copy, renderer extensions, optimization and broader platforms remain deferred. Accessibility, pixel identity across platforms and GPU-specific output are unverified. Hosted GitHub execution remains a documented pre-publication follow-up requiring a later repository publication request.
