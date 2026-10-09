# Requirements

Approved on 2026-10-09 by explicit user approval of requirements, design and tasks. Scope is the local candidate preparation and verification below; no commit, tag, push or publication.

## Purpose and scope

Prepare a verified Linux amd64 1.0 release candidate while preserving the existing iteration 020 work. Leave changes reviewable in the working tree; do not commit, tag, push or publish.

- R1: Correct bundled cmark 0.31.2 fence handling: a closer must use the opener's character and be at least as long as the complete opener. Preserve upstream version, acquisition checksum, notices, symbol prefixing and the unchanged official 652-example corpus/ledger.
- R2: Set package version 1.0.0, public QML module 1.0, private runtime/tooling versions 1.0, shared-library SOVERSION 1 and CMake SameMajorVersion compatibility. Require migration from QML 0.7 and CMake 0.7 requests; support unversioned imports without legacy aliases.
- R3: Guarantee compatibility of documented public QML properties, signals, behavior and CMake integration throughout 1.x. Native implementation classes and QMarkdown.Private stay private; install no C++ headers and promise no public C++ ABI. Consumers must build against a compatible Qt/toolchain; portable prebuilt binaries are not promised.
- R4: Support Linux amd64, Qt >=6.8, C++17 and CMake >=3.21. Retain pinned Qt 6.8.0 and 6.11.3 verification and record host Qt separately.
- R5: Document completed evidence, accepted limits and deferred capabilities consistently in CHANGELOG, README, overview, standards, roadmap, current coverage and viewer status. Retain image rows, PNG/JPEG restrictions, literal HTML, full document replacement, narrow-glyph overhang and uninterruptible codecs. Two occupied decoder workers can delay fresh work; application shutdown can wait for codecs.
- R6: Verify long-fence regressions, imports, package compatibility, strict CommonMark, host shared/static CTest, QML lint, symbol privacy, relocated consumers and both pinned CI tasks. Require zero test failures and exactly 652 official parser/model passes with zero unresolved mismatches, uncheckable cases, limits or losses.
- R7: Run native desktop checks on the available Wayland session for layout, containers, links, image-policy transitions and decoder teardown. Record display/backend/Qt separately from offscreen results; unavailable desktop evidence leaves the candidate gate pending.
- R8: Prepare qmarkdown-1.0.0-candidate.tar.gz from the final source snapshot without Git metadata, build outputs or sensitive files. Include documentation, bundled sources and notices; produce a SHA-256 manifest. Retain shared/static install trees and evidence locally, match implementation against tested snapshots, and verify extracted-source builds and relocated consumers.
- R9: Record actual commands, counts, artifacts, warnings and limits separately from planned checks. Complete the candidate record only after all local gates pass. Hosted GitHub execution remains a pre-publication follow-up requiring a later publication request.

Deferred: optional syntax, streaming, selection/copy, renderer extensions, performance optimization and broader platforms. No parser upgrade, rendering architecture change or release publication.
