# Verification

## Planned

Review the complete diff against origin/main for documentation-only scope. Check whitespace, local Markdown links outside fenced illustrative examples, evidence JSON validity and both retained runtime log SHA-256 values. Cross-check summaries against iterations 024–027. Require packaging (minimum) and packaging (development) on the final PR revision and resulting main SHA. Verify integrated evidence/current summaries and unchanged v1.1.0 release commit. Retain final actual results on a separate documentation evidence branch.

## Actual

Local review completed on 2026-10-12. Refreshed origin; main had no newer commits, so the ordinary merge reported already up to date. The complete diff contains only Markdown, publication JSON and the two retained logs. Iteration 027 matches origin/docs/v1.1.0-publication-evidence byte for byte. Cross-checked current summaries against iterations 024–027, preserving separate semantic, native presentation and binary evidence contracts.

- Local Markdown links outside fenced illustrative examples: 245 checked, all resolve.
- Publication evidence JSON parses successfully; both runtime log hashes match publishedAssetLifecycle.logSha256: Qt 6.8.0 `c308146b6715175ec362933de983645735b0f41b283b0b5fbafe8a9897b86e60`, Qt 6.12.0 `00d306640bb944d2d30219d35d861820a46428ca7bd694602409021dad7f743c`.
- Markdown/JSON whitespace check passes. The full diff whitespace check reports pre-existing trailing spaces in CMake output in both retained logs; these bytes remain unchanged to preserve their recorded hashes.
- Local v1.1.0 resolves to original release commit `b71740c7e0f26b0709f5e929a79e8d01dba56071`; remote annotated tag object remains `4cf88984cce5c431a7dedf778524c0ae27fed749`.

No new local runtime tests were run for prose changes.

## Actual integration and post-merge verification — 2026-10-12 UTC

- [PR #8](https://github.com/lebedenko/qmarkdown/pull/8), “docs: reconcile published v1.1 status and integrate evidence.”, contains only the reviewed documentation/evidence files. Final revision `190dbfea1176bffec6d234287c8ceeb23f90e330` passed both required packaging checks in [run 38227439595](https://github.com/lebedenko/qmarkdown/actions/runs/38227439595). Reviewed hosted logs show 10/10 CTest passes across both SDK packaging builds.
- Ordinary protected merge completed at 2026-10-12T09:15:54Z, producing `0e75af8a8935b0faac1915adc6db80add5332972`, with original main and reviewed PR head as its two parents. No bypass or history rewrite was used.
- Exact-main [run 38228755606](https://github.com/lebedenko/qmarkdown/actions/runs/38228755606) passed packaging (minimum) and packaging (development). Independently confirmed its headSha equals the merge SHA. Both configured Qt 6.8.0/6.12.0 suites passed; hosted logs are retained locally with hashes in the evidence JSON.
- Main's README/specifications match the reviewed PR head, and iteration 027 matches the original publication-evidence branch byte for byte. Complete publication evidence and corrected summaries are integrated; historical claims and deferred capabilities retain their scope.
- Post-merge release metadata, published body/assets and main protection response exactly match captured pre-merge snapshots. Remote annotated v1.1.0 remains object `4cf88984cce5c431a7dedf778524c0ae27fed749`, targeting original release commit `b71740c7e0f26b0709f5e929a79e8d01dba56071`. Main remains at the verified merge commit after both checks.

Final actual results are retained on docs/post-release-upkeep-evidence, separately from main and the immutable release commit. [Machine-readable evidence](run-evidence.json) records PR/merge SHA, complete job results/URLs, log hashes, release asset identities and unchanged protections. Local snapshots and hosted logs are retained under /tmp/qmarkdown-028-evidence; this temporary directory is not a durable artifact. No release was created or next feature selected.
