# Tasks — Native fenced math

Approval: requirements, design and tasks approved by the user response “approved” on 2026-10-16. Dependency audit and verification remain implementation tasks; no success is implied. One cohesive 1.13.0 / QML 1.13 release PR is planned; merge, tagging and publication require separate authorization.

Release delivery approval — 2026-10-17: the user authorized the release plan for PR #36, including documentation completion, fresh final-head Qt 6.8.0/6.12.0 CI and retained full gate, squash merge through normal protections, `release.yml` dispatch with `release_pr=36`, tagging `v1.13.0`, stable/latest publication and downloaded hash verification of exactly four assets. Retain package 1.13.0 / QML 1.13 and existing APIs; no runtime changes. Stop on failed gates, changed inputs or differing published bytes. Historical approvals and results below remain unchanged; this approval supersedes earlier statements that delivery still requires authorization.

- [x] T055.1 Audit/pin upstream snapshot, licenses, fonts, XML dependencies, restricted command registry, resource access, mutable state and work-budget patch points; resolve design gates and obtain explicit approval of both feature specifications.
- [x] T055.2 Vendor audited payload and repository-owned CMake/resource wrapper; implement ON/OFF packaging and private engine service (R055.1, R055.5).
- [x] T055.3 Implement bounded worker/cache, isolated requests, immutable recordings, teardown and failure paths (R055.3, R055.5).
- [x] T055.4 Add QML properties, grouped style, dispatch and shared viewport; preserve host renderer precedence and source modes (R055.2, R055.4, R055.6).
- [x] T055.5 Add mapped parser/service/QML/visual tests and record actual results separately from planned checks.
- [x] T055.6 Deliver feature 056, playground, API/dialect guidance, 1.13 versions and release documentation; prepare one release PR and run final verification (R055.7). Do not merge/tag/publish without separate authorization.

## Requirement-to-check map

| Requirement | Planned checks |
|---|---|
| R055.1 | ON/OFF shared/static builds, historical imports, relocated consumers, extracted-source and Arch payload/license checks |
| R055.2 | QML defaults/availability, exact info-token matching, host precedence and failed-host source fallback |
| R055.3 | Formula-family fixtures, unknown commands, malformed syntax, missing resources, whole-source fallback |
| R055.4 | Live color/font/scale, inherited colors, narrow widths, scrollbars, wheel boundary handoff, scale captures and sharpness regression |
| R055.5 | Source/depth/node/operation/geometry/queue/cache limits, forbidden commands, cross-view state, replacement/destruction during pending work |
| R055.6 | Selection/copy and Find projections, source mode transitions and formula restoration, engine OFF/error paths |
| R055.7 | Documentation checks, examples, exact Qt 6.8.0/6.12.0 matrix and required final-head packaging jobs |

Delivery: [PR #36](https://github.com/lebedenko/qmarkdown/pull/36). Local and distribution results are recorded in [verification](verification.md); required final-head packaging results remain in the PR checks. Merge/tag/publication remain separately authorized actions.

## Authorized release delivery

- [x] Complete release documentation and pass local release-documentation/whitespace checks with host Qt 6.12.0.
- [ ] Push PR #36 final head and require both packaging jobs plus the retained full release gate.
- [ ] Squash merge and dispatch `release.yml` with `release_pr=36`; require unchanged verified tree/main.
- [ ] Pass extracted-source checks on both Qt targets and Arch distribution checks, then tag and publish stable/latest with source/Arch archives and SHA-256 companions.
- [ ] Require successful downloaded hashes for all four assets; keep execution results in Actions without an evidence-only PR or post-release documentation commit.
