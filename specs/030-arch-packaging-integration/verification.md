# Verification

## Planned

Focused Arch recipe and uploader tests; shell syntax; workflow structure; documentation links; JSON and historical log hashes; whitespace (excluding unchanged hashed historical log whitespace). Both packaging checks and complete manual release workflow on final PR revision and exact main merge SHA. Independently validate downloaded package/checksum pairs and source provenance. Preserve protections and existing release/tag. Retain final evidence separately.

## Actual

Local checks on 2026-10-12: all 40 Python tests passed (focused Arch recipe 5/5 and uploader 9/9 first); Bash syntax passed; release YAML parsed and upload dependencies/manual-run exclusion passed; prose documentation links passed; all iteration 029 retained evidence and corrected helper hashes matched. Tracked-change whitespace check passed. Hashed historical logs are preserved byte-for-byte. Hosted gates remain pending. Historical iteration 029 results remain in that iteration and do not satisfy these hosted integration gates.

Initial PR revision `3ebad6c293a0e0f6992aaef1a5b6480c8f84a38d`: packaging run 38239864288 failed because Ubuntu CI provisioning omitted Git, required by the archive-selection regression test. The minimum shared CTest run passed 11/12 checks; the archive test could not launch Git. Added Git to disposable CI provisioning; this does not change product dependencies. All revision-specific gates must run fresh. Failure evidence is retained by the hosted workflow and will be referenced in final evidence.

## Final exact-revision results — 2026-10-12

Iteration complete. [PR #9](https://github.com/lebedenko/qmarkdown/pull/9) merged through unchanged protections using ordinary merge commit `45fd78e973b64b2f719d5bc07b4f6e0c22225b29`. Final PR head was `930a23e21ccf35587b55e7898365432e77546862`.

| Revision | Packaging checks | Complete manual release |
| --- | --- | --- |
| Final PR | [38243782694](https://github.com/lebedenko/qmarkdown/actions/runs/38243782694), both passed | [38243776914](https://github.com/lebedenko/qmarkdown/actions/runs/38243776914), passed |
| Exact main merge | [38246540795](https://github.com/lebedenko/qmarkdown/actions/runs/38246540795), both passed | [38246584474](https://github.com/lebedenko/qmarkdown/actions/runs/38246584474), passed |

Each run records the expected headSha. Ubuntu 24.04 packaging passed on configured Qt 6.8.0 and 6.12.0, including shared/static tests, CommonMark, QML lint, installed consumers and benchmark smoke. The minimum-built archive passed actual installation, consumers and removal on both runtimes. Arch passed 12 source CTest checks, QML lint/private parser symbols, metadata/payload checks, namcap (warnings retained, no errors), fresh pacman installation and zero altered files, four installed consumers, temporary 1.1.0-2 upgrade and removal. Upload was skipped in both manual runs.

Downloaded Ubuntu and Arch package/checksum pairs independently passed SHA-256 on both final PR and main. Ubuntu metadata identifies the expected source commit and Qt 6.8.0. Arch environment identifies the expected source commit; recipe/source hash, committed helper hashes and checkout/build evidence agree. A locally regenerated gzip archive differed from the hosted archive hash; no cross-Git/compressor byte identity is claimed. Hosted source identity remains the per-run recorded hash.

Main package SHA-256:

- Ubuntu: `5808ce5dd38284eea53c453b6b92a21a528678faad48b853fc59a10538eb641c`
- Arch: `bf170be1ed3305af5800873aec9e5efaf2aac6ef21a624bec79e07c9c2091c19`

[Machine-readable evidence](run-evidence.json) retains workflow/job URLs, commit identities, distribution/Qt/image records, package hashes, lifecycle results and hashes for every retained evidence file. [Evidence directory](evidence/) retains full hosted success/failure logs, Arch package versions/metadata/recipe/lifecycle logs, both runtime lifecycle/image records, checksum companions and release/protection/tag baselines. Final evidence is separately published on `docs/arch-packaging-integration-evidence`, based on verified main. Downloaded artifacts and expanded build trees remain local under `/tmp/qmarkdown-030-evidence/`.

The initial revision failed Ubuntu gates due to missing Git in the CI image; both original workflow records/logs are retained under evidence/failures. Git was added only to CI provisioning and all gates were repeated on the correction. The original Arch success did not substitute for corrected-revision or exact-main verification.

The v1.1.0 tag, release body/assets and main protection ruleset matched retained baselines after merge. Package 1.1.0, QML 1.1, public APIs, product dependencies and renderer behavior are unchanged. Original working tree and iteration 028 evidence branch were preserved. No publication, asset upload, tag creation, host package operations, AUR submission or protection modification occurred. Rolling Arch and namcap warnings remain documented limitations; existing v1.1.0 has no published Arch asset.
