# Requirements

Approved on 2026-10-09 through the explicit request to implement the supplied Iteration 018 plan. Approval covers requirements, design and tasks in this directory.

- R1: Reuse provisioned images for Qt 6.8.0 and 6.11.3 without changing Task commands or library interfaces/versions.
- R2: Preserve Ubuntu digest, dependencies, fonts and installer/SDK versions; validate SDK and retain image provenance.
- R3: Image identity depends only on provisioning inputs, Qt and platform. Missing images build automatically; explicit refresh builds uncached; metadata is read-only.
- R4: Use immutable image IDs, fresh snapshots/builds, UID/GID switching, timeouts and cancellation. Retain preparation failure logs without launching checks and record preparation/verification timings.
- R5: Hosted Buildx loads images with SHA-pinned actions and GitHub cache API v2, separate Qt/architecture scopes, mode=max and nonfatal export errors; no registry publication.
- R6: Test runner behaviors and cold/warm runs for both versions; document actual results separately. Hosted restoration must not be claimed without execution.

Compiler caching, persistent builds, upgrades, registry publishing and pruning are excluded.
