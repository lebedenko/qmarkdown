# Verification

Approved on 2026-10-12 through the explicit instruction to implement the supplied Iteration 032 plan. Approval covers commits, pushes, PRs, ordinary protected merges, manual workflow dispatches and a separate evidence branch. Publication, tags, release uploads, AUR submission and host package installation are excluded.

## Planned

Focused view/import checks after transfer, followed by both configured Qt suites. Hosted Qt 6.8.0/6.12.0 packaging must cover shared/static tests, strict 652-example CommonMark checks, lint, private symbols, relocated 1.0/1.1/1.2 consumers and benchmark smoke. Complete manual release workflows on final feature head and exact main merge must pass both same-minimum-archive runtime checks and Arch installation, ownership, consumers, temporary upgrade and removal. Confirm upload skipped. Independently verify artifact/checksum pairs, versions/source SHA and lifecycle records; retain immutable image identities, distribution manifests and all success/failure workflow/job links separately on the evidence branch.

## Actual

Arch evidence [PR #10](https://github.com/lebedenko/qmarkdown/pull/10) merged through unchanged protection as `035ddff` after both packaging jobs passed in [run 38278677113](https://github.com/lebedenko/qmarkdown/actions/runs/38278677113). No unresolved review threads. All 65 retained Arch evidence files match their recorded hashes.

Reconciled-source host Qt 6.12.0 configure/build and focused view/import checks pass (2/2). Transferred source/test/helper bytes match the reviewed originals; historical iteration 031 records match except for the explicit numbering correction note. Current prose documentation links and whitespace pass. Existing main Git provisioning is retained; no Dockerfile addition. Both configured suites and hosted feature/main gates remain pending; historical local checks do not substitute for them.
