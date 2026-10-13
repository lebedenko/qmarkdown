# Verification

Approved on 2026-10-13 by the user response "approved". Approval covers requirements, design, mapped tasks, local implementation and verification. Commit, PR creation, merge and publication remain separately authorized.

## Planned

Official fixture: all 14 GFM 0.29-gfm examples 622–635, with source attribution, CC BY-SA license, acquisition date and checksum. Independently author text, link-node, destination and UTF-16-span expectations; do not derive expectations from production output.

Focused parser checks: valid/invalid domains, boundaries, punctuation, balanced/unbalanced parentheses, entity tails, email suffixes, mailto/xmpp resources, surrounding Unicode, escaped characters, decoded entities, LF/CRLF/CR, explicit/reference-link precedence, image descriptions and literal code/HTML. Exercise all eight task-list/strikethrough/autolink combinations and default CommonMark behavior.

State/resource checks: default-off, both toggle directions, replacement while enabled, inert equal assignments, precise notifications, source preservation and cancellation/rebuild through the existing lifecycle.

Native checks: wrapped body links, headings, nested lists/tasks, inherited emphasis/strikeout, live font/color styling, emitted destinations, pointer drag cancellation and inert image-description presentation. Generate and inspect a representative playground capture.

Broader gates after focused checks: unchanged strict 652-example CommonMark parser/model gate; host shared/static CTest; QML lint; private cmark symbols; compatible historical imports/package requests; EXACT acceptance/rejection matrices; installed QML 1.5 shared linked/plugin-only/static consumers; configured Qt 6.8.0/6.12.0 container packaging checks. Exact-commit Arch lifecycle and publication verification remain later release-delivery work.

## Actual

2026-10-13: inspected repository workflow, iteration 036 records, parser lifecycle and existing extension integration. Working tree was clean before drafting these four documents. Consulted the pinned GFM specification page; fixture acquisition and checksum recording remain planned implementation tasks. Only specification documents were added; no runtime, parser, package or public API changes were made. Requirements/design/tasks were subsequently explicitly approved on 2026-10-13.

### Host implementation checks

Approved implementation completed on the existing working branch, with uncommitted candidate 1.5.0 / QML 1.5 changes. Host: Linux x86_64, Qt 6.12.0, GCC 16.2.1. No commit, PR, merge, tag or publication has been performed for iteration 037.

`QT_QPA_PLATFORM=offscreen build-shared/tests/qmarkdown-parser-test extendedAutolinks extendedAutolinkSemantics officialExtendedAutolinkExamples` passes 45 QtTest cases including initialization/cleanup. All 14 official examples have independently authored displayed text, semantic link/text nodes, destinations and UTF-16 spans. Focused rows run with LF/CRLF/CR and cover domains, boundaries, punctuation, parentheses, entities, source escapes versus decoded email escapes, XMPP resources, Unicode offsets, code/HTML, explicit/reference links and inert image projection. The semantic check exercises all eight extension combinations. Explicit link labels restore core escape/code/emphasis parsing after a source-web candidate was consumed. Additional regressions cover closing brackets inside code/escapes, nested explicit links and images. The final scanner yields at structural syntax within pending labels so those constructs resolve normally.

`QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software build-shared/tests/qmarkdown-view-test extendedAutolinksToggleAndRendering extendedAutolinksResourceCancellation playgroundExtendedAutolinks` passes 5 cases including initialization/cleanup. Native checks assert heading/body/list/task destinations, wrapped links, inherited bold/italic/strikeout, live fonts/colors, pointer activation and drag cancellation. State checks cover default-off, both toggle directions, replacement, no-op assignments/model resets, exact property notifications and preserved source. Held decoder workers exercise cancellation on an extension toggle and replacement; stale results do not publish, and subsequent image rendering recovers. The view requests no network resources.

Shared and static builds complete; `ctest --test-dir build-shared --output-on-failure` and its `build-static` equivalent each pass 12/12 checks. The unchanged `python3 scripts/verify-commonmark.py --strict` gate passes all 652 parser/model examples with zero mismatches, uncheckable examples, unresolved comparison limits or semantic losses. `cmake --build build-shared --target all_qmllint` passes with the existing informational unused import in the import-only example. `python3 scripts/verify-cmark-symbols.py build-shared` passes with 142 prefixed private external definitions and no cmark shared exports. Changed Python packaging scripts compile, and packaging-tool regression checks pass within CTest.

A representative capture was generated by `playgroundExtendedAutolinks` using `QMARKDOWN_AUTOLINK_SCREENSHOT=.../build-shared/037-autolinks.png` and inspected. It shows native blue/underlined wrapped web, email/protocol, heading and nested-list links, bold inheritance, literal inline code, explicit-link precedence and an inert image description. Task-list and strikethrough controls are off in this capture; their composition is asserted separately. This is software/offscreen evidence, not GPU or every-platform certification.

During development, a raw QML string in the new test failed moc parsing; assigning the Markdown through the existing state property resolved it. An initial view assertion expected an inert plain fallback to use FormattedText; correcting the expected native item type/count resolved the assertion. Final focused and full host suites pass. Both initial container runs passed, but subsequent review added the pending-label structural fix and regressions; fresh final container runs verify those changed parser/test inputs. The initial sandboxed minimum-container invocation failed at Docker socket permission; the configured gates were rerun with approved escalation.

### Container gates

Final `task ci-minimum` (Qt 6.8.0) passes with exit 0; retained artifacts: `build-ci/6.8.0/20261013T082130Z-n3wi9pwm`. Final `task ci-development` (Qt 6.12.0) passes with exit 0; retained artifacts: `build-ci/6.12.0/20261013T082136Z-1nzkve81`. Both reuse the configured toolchain images. Each passes shared/static CTest (12/12 each), the unchanged strict 652-example parser/model corpus, QML lint, symbol privacy, historical imports/consumers, QML 1.5 shared linked/plugin-only/static consumers and SameMajorVersion/EXACT matrices. Compatible historical 1.x requests and EXACT 1.5.0 succeed; earlier exact and future versions are rejected.

Both gates also pass library-only builds, benchmark smoke, candidate Ubuntu archive checks and installer ownership/repeated-install/uninstall checks, including 12 installed consumer configurations under staging and real disposable-container roots. No host system installation occurred. Existing optional Qt6TaskTree/QmlAssetDownloaderPrivate SDK discovery notices and the existing test `fgets` unused-result warning do not affect successful required builds/checks.

All 148 build/test/package inputs match both final source snapshots. The [machine-readable summary](verification-summary.json) records the input digest, container outcomes/identities, corpus totals and candidate archive SHA-256 values. Raw logs remain in the retained artifact directories. Final evidence and specification edits are prose-only and do not change verified build/package inputs. These are separately built candidate archives for each SDK; no same-archive cross-runtime or published-release claim is made. Exact-commit Arch lifecycle and publication gates remain deferred to authorized release delivery.

### Changed files and delivery boundary

Changes cover bundled cmark option/source-web/decoded-email parsing and provenance; private document/ViewState and QML alias/metadata; package/module versions; playground toggle/sample; parser/view/import tests, pinned GFM fixture/provenance and installed QML 1.5 consumer; relocated/Ubuntu/Arch compatibility consumers; README, changelog, extension guides and current specification summaries; iteration 037 requirements/design/mapped tasks and actual verification evidence.

Implementation and local verification are complete. At completion of verification, no commit, PR, merge, tag or publication had been performed for iteration 037. Commit was subsequently authorized by the user instruction "commit" on 2026-10-13; PR creation, merge, tagging and publication remain separately authorized. The Arch verifier now includes the 1.5 consumer, but exact-commit Arch package lifecycle cannot establish this uncommitted candidate's release evidence; it remains a later release-delivery task. Candidate 1.5.0 is unpublished.
