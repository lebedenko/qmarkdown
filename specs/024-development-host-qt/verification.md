# Verification

## Planned

Helper/runner tests; both complete container suites; hosted stable packaging checks; release minimum-built archive on both runtimes; review live ruleset before context migration.

## Actual — 2026-10-11

- Configuration/helper tests: 3/3 pass. Runner tests: 15/15 pass. Host qmake6 reports 6.12.0. Exact official Qt 6.12.0 provisioning succeeds using pinned aqtinstall 3.3.0 in the digest-pinned Ubuntu 24.04 amd64 recipe. No SDK substitution.
- Complete isolated-commit container suites pass on Qt 6.8.0 and 6.12.0: shared/static CTest, QML lint, installed/relocated consumers, symbol privacy, library-only build, benchmark smoke, archive/installer lifecycle, staged and real disposable-container install/consumption/uninstall. Outputs: build-ci/024-minimum and build-ci/024-development. Each variant's CTest suite passes 8/8.
- An initial development run failed only the newly registered runner test because Git is intentionally absent inside the image; runner tests remain separate host checks. Their CTest registration was removed and both complete suites reran successfully. Python bytecode is excluded from the image recipe hash and Docker context.
- [CI preparation PR #4](https://github.com/lebedenko/qmarkdown/pull/4) has successful packaging (minimum) and packaging (development) checks in [run 38155634638](https://github.com/lebedenko/qmarkdown/actions/runs/38155634638), at commit 34b09492403cc9778fadd2ab7ca9d08ff9d90a18.
- [Manual release run 38155796438](https://github.com/lebedenko/qmarkdown/actions/runs/38155796438) passes both builds and both binary-compatibility jobs at the same commit. Both compatibility jobs consume the same minimum-built archive. Downloaded archive/checksum and both compatibility evidence artifacts; independently checked SHA-256 and reviewed successful staged/real installed consumers and uninstall. Upload is skipped in manual runs; nothing was published.
- Reviewed the live main ruleset after both new hosted packaging checks passed, prepared a payload replacing only the two old required contexts, then applied the approved migration. Returned writable fields exactly match the proposed payload. Strict checking, PR requirements, conversation resolution, no bypass, deletion/force-push protections and allowed merge methods are preserved. See before/after snapshots.

Exact SDK versions, immutable local/hosted image identities, fingerprints, archive metadata/checksum and hosted job URLs are retained in [run-evidence.json](run-evidence.json). Product version remains 1.0.0 in this iteration. The source PR remains independently reviewable; no merge or release publication is claimed.
