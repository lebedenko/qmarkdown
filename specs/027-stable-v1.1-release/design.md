# Design

Approved on 2026-10-11 through the explicit instruction to implement the Iteration 027 plan, including documentation integration, exact-commit verification, annotated tagging, stable GitHub publication and published-asset checks on both runtimes.

Start a release-preparation branch from current main and merge the pushed iteration 026 evidence branch. Finalize documentation for intended UTC release date 2026-10-11, including absolute v1.1.0 links in the release notes and 1.1 archive commands. Historical candidate/release records retain their original claims and commits. Current documentation points to this approved release iteration without claiming publication before it occurs.

After both protected packaging checks pass, use an ordinary merge commit. The resulting main SHA is the new release candidate; run hosted main packaging and manual release dispatch through main, checking the resolved headSha. Review downloaded minimum archive and both runtime evidence artifacts before creating an annotated tag and stable/latest GitHub release. Existing publication workflow rebuilds and uploads assets; do not substitute manual-run bytes. Distinct builds may have distinct checksums.

Download published bytes and run the existing verifier with --real-install in separate disposable pinned Ubuntu toolchain containers on both exact runtimes. Mount source/archive read-only; install only inside container /usr. Record actual results on a separate evidence branch, leaving tag and release commit fixed. Failed pre-tag gates block tagging; any correction requires fresh checks. After publication, never move the tag or replace differing assets; use existing recovery only where immutable-asset rules permit it.
