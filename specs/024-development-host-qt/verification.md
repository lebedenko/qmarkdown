# Verification

## Planned

Helper/runner tests; both complete container suites; hosted stable packaging checks; release minimum-built archive on both runtimes; review live ruleset before context migration.

## Actual

Host qmake6 reports 6.12.0. Configuration/helper tests: 3/3 pass; runner tests: 15/15 pass. Exact Qt 6.12.0 SDK provisioning succeeded on the digest-pinned Ubuntu 24.04 image using aqtinstall 3.3.0. Initial run evidence: `build-ci/6.12.0/20261011T134449Z-sa6nglo1/{environment,result}.json` (complete immutable image identity retained there). The initial container shared suite passed all product checks but failed the newly registered runner test because Git is intentionally absent from the container. Runner tests remain a separate host check; their CTest registration was removed. Full reruns remain pending. Hosted checks, release cross-runtime verification and ruleset migration remain pending. Product version remains 1.0.0.
