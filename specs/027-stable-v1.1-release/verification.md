# Verification

## Planned

Protected PR and exact-main packaging: Qt 6.8.0/6.12.0 shared/static CTest, strict 652-example corpus assertions, QML lint, installed/relocated QML 1.0/1.1 consumers, package compatibility, privacy/library-only checks and benchmark smoke. Manual release run: both builds, both same-minimum-archive runtime checks, upload skipped. Independent archive checksum, metadata and image evidence review before tagging.

Publication: annotated v1.1.0 points to the verified main SHA; stable/latest release uses finalized notes; both builds, binary-compatibility jobs and upload succeed. Download published archive/checksum, independently verify hash/source metadata and run the existing full verifier on those bytes with --real-install in disposable Qt 6.8.0 and 6.12.0 Ubuntu containers. Retain actual workflow/job/release URLs, tag commit, exact SDK/image IDs, asset metadata, checksums and lifecycle logs separately from the tagged commit.

## Actual

Pending release preparation and exact-commit verification. Iteration 026 evidence describes the preceding candidate and does not verify the new release-preparation commit.
