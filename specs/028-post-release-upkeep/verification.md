# Verification

## Planned

Review the complete diff against origin/main for documentation-only scope. Check whitespace, local Markdown links outside fenced illustrative examples, evidence JSON validity and both retained runtime log SHA-256 values. Cross-check summaries against iterations 024–027. Require packaging (minimum) and packaging (development) on the final PR revision and resulting main SHA. Verify integrated evidence/current summaries and unchanged v1.1.0 release commit. Retain final actual results on a separate documentation evidence branch.

## Actual

Local review completed on 2026-10-12. Refreshed origin; main had no newer commits, so the ordinary merge reported already up to date. The complete diff contains only Markdown, publication JSON and the two retained logs. Iteration 027 matches origin/docs/v1.1.0-publication-evidence byte for byte. Cross-checked current summaries against iterations 024–027, preserving separate semantic, native presentation and binary evidence contracts.

- Local Markdown links outside fenced illustrative examples: 245 checked, all resolve.
- Publication evidence JSON parses successfully; both runtime log hashes match publishedAssetLifecycle.logSha256: Qt 6.8.0 `c308146b6715175ec362933de983645735b0f41b283b0b5fbafe8a9897b86e60`, Qt 6.12.0 `00d306640bb944d2d30219d35d861820a46428ca7bd694602409021dad7f743c`.
- Markdown/JSON whitespace check passes. The full diff whitespace check reports pre-existing trailing spaces in CMake output in both retained logs; these bytes remain unchanged to preserve their recorded hashes.
- Local v1.1.0 resolves to original release commit `b71740c7e0f26b0709f5e929a79e8d01dba56071`; remote annotated tag object remains `4cf88984cce5c431a7dedf778524c0ae27fed749`.

Hosted PR, protected merge and post-merge checks remain pending. Final results will be retained separately after execution. No new local runtime tests were run for prose changes.
