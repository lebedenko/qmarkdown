# Verification

## Planned checks

Both documentation PR packaging jobs, manual stable workflow on final main, archive checksum/manifest review, publication workflow success, downloaded asset checksum and disposable Ubuntu installation/removal with installed consumers.

## Actual results — 2026-10-10

- [Preparation PR #2](https://github.com/lebedenko/qmarkdown/pull/2) passed both required Qt packaging jobs in [run 38079312354](https://github.com/lebedenko/qmarkdown/actions/runs/38079312354) and merged as `b83cd2354eb7fed929fa3b08a21282c1e9f9bfff`. README, changelog and release notes were reviewed against existing compatibility and installation behavior; `git diff --check` passed. Historical candidate records remain intact.
- [Manual stable-package run 38081830236](https://github.com/lebedenko/qmarkdown/actions/runs/38081830236) verified that exact main commit. Both build jobs and both binary-compatibility jobs passed on Qt 6.8.0 and 6.11.3. Upload was intentionally skipped for manual verification. Downloaded both binary-check artifacts and reviewed their successful staging and real `/usr` install, linked/plugin-only consumers and uninstall results before tagging.
- Reviewed the downloaded Qt 6.8.0 archive: checksum matched, metadata identified the exact commit, 1.0.0, Ubuntu 24.04 x86_64 and Release build; expected 16 payload entries and root-owned archive headers were present. Manual archive SHA-256: `43a004a7f032039f74e78b921e6711c76da271eda51dedf3e7956b1bdc3ac0da`.
- Created annotated `v1.0.0` at `b83cd2354eb7fed929fa3b08a21282c1e9f9bfff` and published [QMarkdown 1.0.0](https://github.com/lebedenko/qmarkdown/releases/tag/v1.0.0) as a stable release using the prepared notes. CMake already declared 1.0.0.
- [Publication run 38082688908](https://github.com/lebedenko/qmarkdown/actions/runs/38082688908) passed both builds, both cross-runtime binary-consumer jobs and upload. Downloaded the archive and checksum from the published release, independently verified SHA-256 and checked its source-commit metadata. This separate workflow rebuild has its own bytes/checksum; no bit-for-bit reproducibility claim is made.
- Published asset: `qmarkdown-1.0.0-ubuntu24.04-x86_64-qt6.8.tar.gz`, 327766 bytes. SHA-256: `5138203a9af44dcdfa728d4fcbcee581cf6c9c177531b056d342f44bb731876a`.
- Ran the existing archive verifier against those downloaded published bytes in a disposable pinned Ubuntu 24.04 amd64/Qt 6.8.0 container. Archive integrity, installer conflict/upgrade/modification lifecycle, staging consumers, repeated real `/usr` installation, root ownership, linked and plugin-only consumers and clean uninstall all passed (exit 0). The host `/usr` was never modified. See the [saved output](published-asset-verification.log). Nonfatal Qt font-unit warnings remain accepted.

[Machine-readable evidence](run-evidence.json) records workflow job URLs, exact commits, release asset details, archive metadata/manifest, checksum and the container identity. No production code, API or feature scope changed. All release-plan gates are complete.
