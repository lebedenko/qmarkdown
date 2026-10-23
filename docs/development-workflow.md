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

PR CI retains `packaging (minimum)` and `packaging (development)` on exact committed Qt targets. The complete PR diff (including both sides of renames) selects additive coverage:

| Profile | Inputs | Verification |
| --- | --- | --- |
| Light | Documentation, specs, standalone experiments/evidence | Documentation links, changed evidence/configuration syntax and Qt-independent Python tests once |
| Core | Shipping C++/QML, runtime tests, examples | Shared runtime suite, QML lint, symbol privacy and grouped installed consumers on both Qt targets |
| Full | Build/package/dependency/toolchain/CI inputs, consumer infrastructure, benchmark tooling, release notes/changelog/version changes, unknown inputs | Shared/static × syntax OFF/ON, both math-disabled linkage variants, one library-only build per target and development-Qt Release benchmark smoke |

Core syntax/provider/code-block inputs add a shared syntax-enabled configuration; math inputs add both math-disabled linkage checks. The `ci:full` PR label only escalates coverage. Label changes reclassify; superseded PR runs are cancelled, while matrix failures do not cancel the other target. Standalone experiments stay outside shipping CI; their own local verification plans must execute exact-Qt harnesses and retain results.

Ninja builds use two workers and serial GUI tests. Build-only ccache is shared across configurations, keyed by recipe/Qt/architecture, and persisted between hosted runs. A miss changes speed only. Image preparation has 20 minutes; Core verification has 20 minutes and Full 45 minutes. Outer jobs allow 50/75 minutes, and the supervisor leaves time to collect container timeout results. Available logs, phase durations and cache statistics are collected unconditionally with existence checks and 14-day retention. Forced runner termination can prevent artifact upload.

Before changing the configured development target, run `task sync-qt` (or `python3 scripts/ci-toolchain/qt_versions.py --sync-host /path/to/qmake`) and review/commit that change. CI never silently chooses latest Qt. There are no main-push or publication-triggered full-suite runs. Only successful Full evidence with schema `qmarkdown-ci-v2` qualifies for publication; Core/Light never qualify.

### Local verification and hooks

`task check` runs documentation, configuration/evidence syntax and Python tooling checks without Docker. Install Python 3, PyYAML (`python3-yaml` or `python3 -m pip install PyYAML==6.0.3`) and Git first. `task install-hooks` sets the local repository hook path and refuses unrelated hook configurations or existing active hooks; integrate those manually when necessary.

Pre-commit checks staged whitespace/conflict markers and validates the staged snapshot. Pre-push checks each distinct non-deleted commit supplied by Git, including non-HEAD tips and multiple refs. Each commit is classified against its merge-base with locally available `origin/main`; missing ancestry conservatively requires a build. `task check-push` checks HEAD explicitly. Shipping/build/runtime-test inputs run an incremental shared build and full runtime suite on configured development Qt. Documentation, experiments and CI/Python-only inputs use lightweight checks.

Hooks export Git blobs into a managed `build-ci/hooks/source` cache, retain timestamps for unchanged files and adjacent incremental build outputs, and use an exclusive nonblocking lock. They never stash or alter the checkout. Set `QMARKDOWN_QMAKE=/path/to/qmake` for the exact development SDK; install CMake and Ninja before building. Missing/mismatched tools fail with instructions; hooks never download or synchronize Qt. Standard `git commit --no-verify` and `git push --no-verify` bypass hooks. Local success assists development and cannot replace hosted release evidence.

`task ci-minimum` and `task ci-development` remain explicit comprehensive Full container checks. For focused reproduction use `python3 scripts/run-ci.py --qt-role development --profile core`, adding `--syntax`/`--math` when relevant. Full performance measurements remain explicit through `task benchmark`. Record cold/warm compiler-cache conditions separately; the 8–12 minute Core target is a goal, not a guaranteed result.

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
