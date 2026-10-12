# Verification

Approved on 2026-10-12 through the explicit instruction to implement the supplied Iteration 034 plan. Approval covers documentation/evidence integration, commits, pushes, protected ordinary PR merge and post-merge checks.

## Planned

Validate retained evidence hashes and JSON, current Markdown links and status consistency; check authored whitespace while preserving raw logs. Compare live release/tag/asset identities and historical release/protection baselines. Require both packaging contexts on final PR revision and resulting main SHA, with no unresolved review threads. Reuse iteration 033 downloaded-package lifecycle results if consistent. Retain actual post-merge results on docs/post-v1.2-release-upkeep-evidence.

## Actual

Local verification on 2026-10-12: 213 retained iteration 032/033 hashes match; all 121 specification JSON files parse; 298 current prose local Markdown links resolve. Iteration 033 records are byte-identical to the publication-evidence branch. Authored changes pass git diff --check; raw logs retain original bytes and tool-emitted whitespace.

Live stable/latest release ID 410884660, publication timestamp 2026-10-12T18:16:58Z, annotated tag object b2ef4a95362de259f4ec1c6ecfa0f2488ea03737 and all four asset IDs/sizes/digests match iteration 033. Historical release bodies/assets and protection ruleset 24818691 match retained baselines. No discrepancy requires runtime acceptance repetition; completed downloaded Ubuntu Qt 6.8.0/6.12.0 and Arch lifecycle evidence is reused.

Scope review: only current prose and iteration 034 preparation records are authored; the ordinary publication-evidence merge retains historical records and raw artifacts. No API, production, version, dependency, workflow or protection changes. Existing checkout and branches are preserved. GitHub baseline retrieval initially used an incorrect ruleset ID (404); reading the ID from retained evidence and retrieving the actual ruleset passed.

[PR #13](https://github.com/lebedenko/qmarkdown/pull/13) passed both required packaging checks on final head `03d947eeea11f02f1d1f18ba09f904571a23d427` in [PR run 38297427988](https://github.com/lebedenko/qmarkdown/actions/runs/38297427988). Review-thread enumeration was complete with no unresolved threads. Main remained at the release commit; GitHub reported CLEAN readiness. Ordinary protected merge completed at 2026-10-12T19:33:31Z, producing `8f1a5f3ff7a7dd444aa77ed0f2b11be9865140a7`; no bypass, branch deletion or history rewrite occurred.

Both `packaging (minimum)` and `packaging (development)` passed on that exact resulting main SHA in [post-merge run 38298446967](https://github.com/lebedenko/qmarkdown/actions/runs/38298446967). The merged tree includes byte-identical iteration 033 evidence, 298 resolving current prose links and passing authored whitespace; all 134 changed files are documentation/evidence. Post-merge live release/latest/tag/historical-release/protection JSON equals the pre-merge baseline. Annotated tag and four asset identities/digests remain fixed. No local runtime tests were repeated for prose changes because retained independent downloaded-package lifecycle evidence is complete and consistent.

Iteration 034 is complete. [Machine-readable results](run-evidence.json) retain exact PR/main workflow and job URLs, complete review-thread results, live before/after baselines and validation counts with retained-file hashes. Final execution records are retained separately on `docs/post-v1.2-release-upkeep-evidence`; main retains the preparation record, while tag and release assets stay fixed. Existing checkout and branches remain intact.
