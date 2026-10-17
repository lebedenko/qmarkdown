# Tasks — Standalone display math

Approval: requirements, design and tasks approved by the user response “approved” on 2026-10-16. Dependency audit and verification remain implementation tasks; no success is implied. One cohesive 1.13.0 / QML 1.13 release PR is planned; merge, tagging and publication require separate authorization.

Release delivery approval — 2026-10-17: the user authorized the release plan for PR #36, including documentation completion, fresh final-head Qt 6.8.0/6.12.0 CI and retained full gate, squash merge through normal protections, `release.yml` dispatch with `release_pr=36`, tagging `v1.13.0`, stable/latest publication and downloaded hash verification of exactly four assets. Retain package 1.13.0 / QML 1.13 and existing APIs; no runtime changes. Stop on failed gates, changed inputs or differing published bytes. Historical approvals and results below remain unchanged; this approval supersedes earlier statements that delivery still requires authorization.

- [x] T056.1 Obtain approval of requirements/design/tasks together with feature 055.
- [x] T056.2 Add container-aware cmark syntax option/node and private adapter/model support, retaining ordinary parsing on unmatched input (R056.1–R056.3).
- [x] T056.3 Add source projections and decoration-free shared formula presentation; handle toggles, Find/selection and backend OFF (R056.4).
- [x] T056.4 Add mapped fixtures and QML regressions; check lookahead complexity and run the default-disabled 652-case semantic corpus.
- [x] T056.5 Include documentation/playground and final Qt/package verification in feature 055's single release PR; record actual results (R056.5).

## Requirement-to-check map

| Requirement | Planned checks |
|---|---|
| R056.1 | Top-level/list/quote delimiters; zero–three and four-space indentation; trailing whitespace and nested containers |
| R056.2 | Missing/cross-container closure; blank lines; paragraph non-interruption; fenced/indented code and literal HTML; each GFM extension; 652 default-disabled CommonMark semantic passes |
| R056.3 | Literal TeX/backslashes/Markdown punctuation; newline normalization; excluded dollar and bracket forms; exact body projection |
| R056.4 | Backend ON/OFF, errors/limits, centering/overflow/wheel handoff, no code header, selection/copy/Find and restoration, live opt-in toggle/replacement |
| R056.5 | Shared exact Qt matrix, lint/imports, relocated/extracted consumers, Arch payload, docs and final-head packaging gate |

Delivery: [PR #36](https://github.com/lebedenko/qmarkdown/pull/36), shared with feature 055. Actual local/distribution checks are in [verification](verification.md); final-head results are tracked by the required PR checks.

## Authorized release delivery

- [x] Complete release documentation and pass local release-documentation/whitespace checks with host Qt 6.12.0.
- [ ] Push PR #36 final head and require both packaging jobs plus the retained full release gate.
- [ ] Squash merge and dispatch `release.yml` with `release_pr=36`; require unchanged verified tree/main.
- [ ] Pass extracted-source checks on both Qt targets and Arch distribution checks, then tag and publish stable/latest with source/Arch archives and SHA-256 companions.
- [ ] Require successful downloaded hashes for all four assets; keep execution results in Actions without an evidence-only PR or post-release documentation commit.
