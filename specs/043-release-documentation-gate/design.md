# Design

Approved with the supplied implementation plan on 2026-10-14.

Use paired `current-release` Markdown comments as machine-readable boundaries. Headings and command blocks within are validated together. Historical feature records and explicitly labelled post-tag development sections remain outside. This keeps historical approvals intact without guessing section lengths from headings.

Extend the existing standard-library checker with `--release`, concrete version/URL/asset checks and diagnostics naming the document. Ordinary mode takes the published version from README; strict mode takes the package version from CMake. Package and QML metadata must agree. Compatibility ranges and explicitly historical references remain allowed.

The release workflow runs strict validation before archive creation. The publisher calls the same validator after eligibility and before tag/release writes. Existing upload and hash checks remain unchanged. No automated documentation commits follow publication.
