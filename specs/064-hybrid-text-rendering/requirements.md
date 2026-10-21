# 064 — Hybrid text rendering

Authorization: The owner explicitly requested implementation of the accepted production-hardening plan on 2026-10-21. Scope: isolated upstream candidate based on bbb0d0c and V10, implementation, tests, qualification and reviewable evidence. No API/version change, commit, publication or viewer adoption.

Eligible prepared layouts use public scene-graph text nodes. Padded code, rendered formula/footnote atoms and rounded code backgrounds retain corrected V9 raster rendering. Logical interaction geometry is independent of the selected surface. Only one surface exists. Overlay-only changes preserve layout and glyph nodes; scrolling only transforms eligible text. Qt owns node lifetime on the render thread. Preserve shaping, links, source mappings, selection, Find, focus and extension contracts. C++17, Qt >=6.8 and optional features remain supported.

Qualification requires shared/static Qt 6.8.0/6.12.0 and installed/source consumers, native OpenGL/Vulkan DPR 1/1.25/1.5/2, software correctness, fixed 28-trial V10 comparison, separate memory/lifetime measurements, packaged rebuild, viewer integration and task check. Missing environments are unverified. Perceived shimmer is unassessed.

## Transfer authorization (2026-10-21)

The owner explicitly requested the accepted transfer plan. The authoritative continuation workspace is
`/home/andrii/Projects/pet/qmarkdown`, on local branch `feat/hybrid-text-rendering`.
Transfer existing candidate sources, tests, specifications and immutable evidence; retain Feature 064
and follow the existing QMarkdown workflow. No warning fix, missing CI matrix, acquisition, commit,
push, PR, merge, release, version change or md-viewer dependency adoption is authorized by this transfer.
See [handoff](handoff.md) for the starting point and transfer verification.

## Qualification continuation approval (2026-10-21)

The owner explicitly approved implementation of the continuation plan in this session, including
these requirements, the ownership design below and the mapped tasks. This supersedes the
transfer-only restrictions on the warning fix and isolated toolchain acquisition; historical
approvals and results remain unchanged. Finish as uncommitted local work, without publication,
version/API changes, installed-package replacement or md-viewer dependency adoption.

Fix real built-in highlighting initialization without suppressing warnings: construct the KDE
repository lazily on the application thread before the first built-in job, detach its application
event filter, then transfer exclusive use and destruction to the highlighting worker. No worker-to-
GUI blocking call is permitted. Token processing, cancellation, caching and delivery stay unchanged.
Acquire missing exact Qt 6.8.0/6.12.0 container toolchains through existing provisioning. Fresh
checkout builds, warning-failing regressions, full packaging matrices, native/software correctness,
final-byte source distribution and isolated viewer checks are required. Missing environments block
full qualification. Reuse immutable replay/lifetime evidence only with source identities and
unchanged renderer, fixture and measurement inputs; repeat affected measurements otherwise.

## Local commit authorization

After completed qualification, the owner explicitly requested `commit`. This authorizes one local
commit containing the Feature 064 implementation, regressions, specifications and verification
records. Earlier no-commit statements describe their original scope. Push, PR, merge, release and
viewer adoption remain outside this authorization.

## PR and hosted CI authorization (2026-10-21)

The owner explicitly approved publishing `feat/hybrid-text-rendering`, opening one non-draft PR
against `main`, and completing the existing full hosted verification. Update delivery records in a
normal follow-up commit to `2e7f7d6`; resolve CI failures within approved scope if needed. Preserve
historical evidence and exact Qt 6.8.0/6.12.0 targets. No merge, auto-merge, tag, release, API/version
change, installed-package replacement, viewer dependency adoption or visual-acceptance claim is
included. Record final hosted results in the PR description and handoff to the owner, without an
additional evidence-only commit that would invalidate the checked revision.
