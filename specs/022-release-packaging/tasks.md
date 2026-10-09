# Tasks

Approved on 2026-10-10 through the explicit instruction to implement the supplied plan. Scope includes repository rules and release tooling; creating a tag or publishing the first release is excluded.

| Task | Requirements | Checks |
| --- | --- | --- |
| Configure and API-read main ruleset | R1 | Saved API settings, no direct push |
| Build archive, manifest and installer | R2, R3 | Contents, hashes, modes, links, RPATH, conflict/lifecycle scenarios |
| Extend CI and add release workflow | R4, R5 | Both pinned suites, same archive consumers, disposable /usr lifecycle |
| Update installation/release docs | R5 | Review against actual commands |
| Record actual verification | R1–R5 | Separate executed results and outstanding hosted checks |

All implementation tasks and local checks are complete; see [actual results](verification.md). Hosted workflow execution awaits the PR. First-release creation remains excluded.
