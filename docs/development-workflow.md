# Development workflow

This is the repository-owned guidance for current QMarkdown development and delivery. It supersedes historical delivery procedures in the [specification records](../specs/README.md), including separate integration, evidence, and post-release upkeep initiatives. Historical approvals, scope, and actual verification records remain intact.

## Scope and approval

Follow the cycle in [AGENTS.md](../AGENTS.md): requirements → design → tasks → user approval → implementation → verification. Before feature implementation, resolve blocking design decisions, map requirements to tasks and checks, and obtain explicit approval of requirements, design, and tasks. Record approval and its scope in the numbered feature documents. Changes to approved scope require updated documents and renewed approval; routine fixes within approved scope may proceed.

Integration, evidence recording, and publication are delivery tasks within the meaningful change. Do not automatically create separate numbered initiatives for them. Publication still requires authorization for that action; including it in delivery guidance does not grant authorization.

## Changes and review

Use one PR per meaningful change and squash merge by default. Keep implementation, tests, documentation, and verification together so reviewers can assess behavior and evidence in the same change. Prefer small, reviewable changes and preserve existing conventions.

Respect existing repository protections and required checks. Do not bypass checks or rewrite published history. A merge preference does not override repository protections.

## Verification

Run the narrowest relevant checks first. Repeat broader checks when their inputs change, checks fail, or unresolved concerns justify another run; do not rerun them merely to record previous results.

For prose-only changes, use lightweight checks such as document links, whitespace, and consistency. Runtime tests are unnecessary unless the documentation change exposes a behavior concern. For build-affecting changes, perform full verification relevant to the affected build, packaging, and supported environments. Record actual results separately from planned checks, including any checks that could not run and the exact reason.

PR CI retains the required `packaging (minimum)` and `packaging (development)` jobs.
Documentation-only PRs run documentation validation through these jobs. Release
notes/changelog changes and all build/tooling/runtime inputs run the complete
shared/static matrix on exact committed minimum/development Qt versions. There
are no automatic main-push, manual prepublication or publication-triggered full
suite runs. Before PR verification, run `task sync-qt` (or
`python3 scripts/ci-toolchain/qt_versions.py --sync-host /path/to/qmake`) to align
the development target with the selected host SDK; review and commit that change.
CI never silently chooses the latest Qt.

## Release delivery

Finalize package/QML versions, release notes, changelog date and current-version
documentation together in one release PR, including complete release summaries,
compatibility guidance, download URLs, asset names and installation commands.
Mark each current summary with `<!-- current-release:start -->` and
`<!-- current-release:end -->`; headings and command blocks inside are all checked.
Keep historical approvals and verification outside those boundaries. Separate
changes added after the release tag under an explicit development heading.
Current release descriptions must
contain no candidate, preparation or pending-publication wording. Describe the
release behavior; do not prewrite successful verification claims. Record actual
checks separately from planned checks.
Run `python3 scripts/check-docs.py --release` in the verified release PR.
The workflow repeats this before distribution creation and the publisher repeats
it before any tag or release writes. Failure reports the document and reason and
stops publication. Ordinary validation permits a development package newer than
the published summary. No automatic follow-up documentation commit is needed.
The final PR's successful full Qt matrix
is the complete release verification gate.

After authorized squash merge, manually dispatch `release.yml` with the merged
release PR number. It requires the latest final-head CI run to pass, retained
full-gate inputs to match both configured Qt targets and the merged Git tree,
and main to remain at that merged commit. A moved main or changed inputs require
fresh verification through an updated PR. Existing tags must match the verified
commit and package version; tags are never moved.

Build the deterministic `qmarkdown-X.Y.Z.tar.gz` from the explicit file allowlist.
It includes library sources, private parser, CMake integration, licenses/provenance
and standalone instructions. It excludes viewer, examples, tests, benchmarks,
specifications and CI tooling. Build/install its extracted contents on both Qt
targets, then build the Arch package from the same archive and run focused
metadata/dependency/payload/ownership/consumer/removal checks once. These are
distribution checks, not another full-suite run; makepkg does not run CTest.

Only after artifact checks pass, create the version tag at the verified merged
commit, create a draft release, upload source/Arch archives and their SHA-256
companions, and publish stable/latest. Download all four assets and compare their
SHA-256 values against the locally checked bytes. Release completion means
publication **plus successful downloaded hash verification**. Do not rerun
consumer/lifecycle tests after download or attach summaries/logs as release assets.
Ordinary execution logs remain in Actions.

Stop on a failing stage and report it. Upload retries refresh remote state and
accept only identical existing assets. Never overwrite differing published bytes,
move a tag or claim completion with failed hashes. A published difference requires
an explicit repair decision.

Do not create evidence branches, evidence-only PRs or automatic post-release upkeep
commits. Preserve historical releases, assets, tags, approvals and verification
records. This workflow migration does not release a new version or change runtime
APIs; existing Ubuntu instructions remain historical guidance.
