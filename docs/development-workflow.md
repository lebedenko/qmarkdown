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

These rules guide local verification effort. Current CI triggers and required checks remain unchanged until separately implemented; a prose-only PR must still satisfy the checks required by existing protections.

## Release delivery and evidence

Verify the final release packages and publish those exact bytes. If package inputs or package bytes change, verify the resulting packages before publication. Identify the evidence through the source tag and commit, successful workflow run, package checksums, and corresponding release assets. Checksums connect the verified packages to the published bytes.

Keep detailed logs in GitHub Actions and link the successful run from the release. If evidence needs longer retention than Actions provides, attach a compact verification summary to the release with the source tag, workflow run, checks performed and results, package checksums, and asset names.

Do not create evidence branches, evidence-only PRs, or automatic post-release upkeep commits. Commit after publication only for meaningful new work or corrections. Preserve existing release assets, historical evidence, and published Git history unless a separately authorized change requires otherwise.
