# Verification

Approved on 2026-10-12 through the explicit instruction to implement the supplied Iteration 032 plan. Approval covers commits, pushes, PRs, ordinary protected merges, manual workflow dispatches and a separate evidence branch. Publication, tags, release uploads, AUR submission and host package installation are excluded.

## Planned

Focused view/import checks after transfer, followed by both configured Qt suites. Hosted Qt 6.8.0/6.12.0 packaging must cover shared/static tests, strict 652-example CommonMark checks, lint, private symbols, relocated 1.0/1.1/1.2 consumers and benchmark smoke. Complete manual release workflows on final feature head and exact main merge must pass both same-minimum-archive runtime checks and Arch installation, ownership, consumers, temporary upgrade and removal. Confirm upload skipped. Independently verify artifact/checksum pairs, versions/source SHA and lifecycle records; retain immutable image identities, distribution manifests and all success/failure workflow/job links separately on the evidence branch.

## Actual

Arch evidence [PR #10](https://github.com/lebedenko/qmarkdown/pull/10) merged through unchanged protection as `035ddff` after both packaging jobs passed in [run 38278677113](https://github.com/lebedenko/qmarkdown/actions/runs/38278677113). No unresolved review threads. All 65 retained Arch evidence files match their recorded hashes.

Reconciled-source host Qt 6.12.0 configure/build and focused view/import checks pass (2/2). Transferred source/test/helper bytes match the reviewed originals; historical iteration 031 records match except for the explicit numbering correction note. Current prose documentation links and whitespace pass. Existing main Git provisioning is retained; no Dockerfile addition. At transfer time, both configured suites and hosted feature/main gates were pending; the final results below supersede that initial status. Historical local checks do not substitute for these gates.

## Final hosted results — 2026-10-12

Iteration complete. [Feature PR #11](https://github.com/lebedenko/qmarkdown/pull/11) merged through unchanged strict protections with ordinary merge commit `a65f537e5b2134e026bf328a1ec4a307c48588ef`. Final feature head was `a3752b1409726ab7cd12b7c5039e6e3790025c5b`; main had not advanced before merge. Both PRs had no unresolved review threads. The original working tree and existing branches are preserved; no history rewrite or protection changes occurred.

| Revision | Packaging | Complete manual release |
| --- | --- | --- |
| Final feature | [38280434679](https://github.com/lebedenko/qmarkdown/actions/runs/38280434679), passed | [38280434770](https://github.com/lebedenko/qmarkdown/actions/runs/38280434770), passed |
| Exact main merge | [38282469583](https://github.com/lebedenko/qmarkdown/actions/runs/38282469583), passed | [38282474700](https://github.com/lebedenko/qmarkdown/actions/runs/38282474700), passed |

All workflow headSha values match the intended revision. Each configured hosted Qt 6.8.0/6.12.0 suite passed shared/static CTest (12/12 each), all_qmllint, symbol privacy, relocated 1.0/1.1/1.2 consumers, linked/plugin-only checks, library-only/custom-static-QML builds, compatible/exact package requests and validated benchmark smoke. Release archive builds passed 12/12 tests. All 652 CommonMark parser/model examples pass with no mismatches/uncheckable examples or unresolved limits/losses; the oracle complete-corpus test asserts strict_pass separately from the ordinary CLI corpus check.

Both complete manual release workflows passed installation, staged/real consumers and removal of the same minimum-built Ubuntu archive on exact Qt 6.8.0 and 6.12.0. Arch passed source tests/lint/private symbols, metadata/payload/namcap checks, fresh installation, root/pacman ownership, zero altered files, six linked/plugin-only 1.0/1.1/1.2 consumers, temporary 1.2.0-2 upgrade and removal. Upload jobs were skipped.

Independent validation of downloaded Ubuntu/Arch checksum pairs passed on final feature and exact main. Ubuntu metadata identifies version 1.2.0, expected source SHA and Qt 6.8.0; every payload manifest hash/link matches. Every regular file extracted by both runtime checks equals the downloaded minimum-built archive. Arch package metadata is 1.2.0-1 and matches retained .PKGINFO; environment source SHA, recipe source hash and helper hashes match the committed tooling. Packaging, release-build and runtime image identities agree for each SDK. Distribution manifests, package metadata, image recipe/immutable identities and full run/job URLs are in [run-evidence.json](run-evidence.json); checksum companions, lifecycle logs, distribution package inventories and source/protection baselines are retained under [evidence](evidence/).

Main package SHA-256:

- Ubuntu: `07224c9016d69945df2045614879982646b610c2eeaa641c5993ffff16e2338c`
- Arch: `1f9e3e0ec9fd592be6ec7490f763a59100a7b5192de62dcbb681bf7d0e3adf17`

Host focused view/import checks passed 2/2. The fresh local minimum Qt suite passed shared/static 12/12, lint, relocated consumers, benchmark smoke and disposable release install/removal. Its corpus reports 652 parser/model passes and zero unresolved limits/losses. The optional fresh local Qt 6.12.0 image download stalled during SDK provisioning and was safely interrupted (exit 130) after all exact-main hosted gates passed. No local development suite completion is claimed; its partial preparation log/environment/result are retained. Both configured hosted suites satisfy the candidate gates.

No corrective source changes or hosted verification failures occurred. Broad gh artifact download encountered a non-ZIP Docker build record; selecting only required packages/lifecycle artifacts succeeded. A retry refused overwriting existing artifacts; only missing artifacts were downloaded. Existing nonfatal Qt/font/private-QML and namcap warnings remain visible in retained logs. Arch is rolling; no byte reproducibility across runs or Git/compressor implementations is claimed. Historical 031 approval/evidence bytes and artifact paths remain intact, with the numbering correction explicitly noted.

Published v1.1.0 tag, release body/assets and the full protection ruleset match retained before/after baselines. No tag, release publication, asset upload, AUR submission or development-host package installation occurred. Actual evidence is committed separately on `docs/v1.2-integration-evidence`, based on verified candidate main; merging that evidence later would move main and require fresh verification for that new commit. Downloaded packages and expanded artifacts remain locally under `/tmp/qmarkdown-032-hosted/`; hosted artifacts have 14-day retention. Publication remains a separate iteration.
