# Verification

The supplied plan authorizes this migration on 2026-10-13. Planned checks include
offline tooling tests for source inclusions/exclusions, licenses and deterministic
bytes; publication eligibility and upload/hash failures; documentation validation;
focused extracted source builds on both Qt versions and Arch disposable-container
checks; the final required Qt matrix. Actual results are recorded below after execution.

No new release, tag mutation or historical asset change is authorized by this migration.

## Actual local verification — 2026-10-13

Base commit: `cc3c74d0b1f3c5cc5765fcd315a2d3d0b23510d4` plus this migration's working
changes. These are development checks, not new published release evidence.

- `python3 -m unittest discover -s tests -p 'test_*.py'`: 52/52 passed. Covers
  deterministic library-only archives and exact license bytes, selected-commit
  isolation, host-qmake synchronization, Arch recipe/payload boundaries, final
  PR CI eligibility, missing/stale gates, tree/Qt/version/tag/main mismatches,
  bounded partial upload retries, differing assets and failed downloaded hashes.
  Fake GitHub publication exercises draft/upload/stable/latest ordering and a
  main move during upload; no network writes or test release occurred.
- Tests also ran from `/tmp` to verify independence from CTest's working directory.
- `scripts/check-docs.py`, Python compilation, workflow YAML parsing and embedded
  shell syntax, `bash -n`, `task sync-qt --dry`, actual selected `qmake6` synchronization
  (Qt 6.12.0; no configuration change), and `git diff --check` passed.
- Working-source allowlisted archive SHA-256:
  `1f25ccb58d969e32de10f76e7bbe10272d6234161d166dad0d27b990c4fc40fa`.
  Extracted default library-only Release builds/installations passed in disposable
  containers on Qt 6.8.0 and 6.12.0, including CMake metadata, license installation,
  no private headers and disabled tests/examples. Qt 6.12 emitted the existing
  optional QmlAssetDownloader/TaskTree SDK warning; configuration/build/install passed.
- Arch consumed those same source bytes and passed makepkg, namcap,
  metadata/dependency/payload/integrity checks, root/pacman ownership, exact-version
  linked/plugin-only GFM consumers and removal once. No full suite or temporary
  upgrade package ran. Qt lower bounds were 6.12.0. Image identity:
  `sha256:5a30b4acc83ff841682eb21795e00f7ff030796f5e6fd2a127decd7a45ba7cef`.
  Package SHA-256: `edc0626b0bf88bbb2cc514b92d49009979c453ece599294de2ebeef514490d6f`.
  Detailed local logs are in ignored `build-delivery-039/arch/`; source checks
  are in `build-delivery-039/source-minimum/` and `source-development/`.
- Read-only GitHub metadata confirmed completed merged PR runs can have empty
  `pull_requests` arrays. The gate artifact now pins the PR number explicitly.

Final hosted required Qt matrix is to run on the implementation PR's final
revision; its actual result will be available in the PR checks and delivery
response. Local results above do not imply hosted CI success. No main-push,
prepublication or publication full suite is requested. Historical releases,
assets, tags and verification records are unchanged.
