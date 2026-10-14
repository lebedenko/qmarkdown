# 043: Release documentation gate requirements

Approved on 2026-10-14 through the supplied “Prevent stale documentation after release” implementation plan and explicit instruction to implement it. Scope includes implementation, tests, documentation repair and verification in one PR. No publication, tag/asset mutation or runtime/version changes are authorized.

- R1: Validate complete, explicitly bounded current release sections, including commands and compatibility guidance; preserve historical approvals/evidence and separate development after the release tag.
- R2: Strict release validation requires consistent package/QML versions, release URLs and asset names; rejects stale versions, future downloads and publication disclaimers.
- R3: Ordinary development validation permits development packages newer than published summaries.
- R4: Validate before distribution creation and within the publisher before remote writes; retain eligibility, immutable assets, retry and downloaded-hash behavior.
- R5: Repair current 1.7.0 instructions; exclude custom fenced-block renderers from that release. Report stale remote release description separately.
