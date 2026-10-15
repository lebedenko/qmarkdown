# Verification — keyboard reading and link accessibility

## Planned checks

The [task matrix](tasks.md) maps requirements to implementation and checks.
Run focused parser/projection, reader, selection, focus and accessibility tests
before lint and the full configured Qt 6.8.0/6.12.0 shared/static matrix. Include
installed consumers, historical imports, compatible 1.x CMake requests, source
archive inventories and packaging. Verify focus outlines visually with wrapped,
bidirectional and linked-image fixtures. Inspect accessible tree membership and
notifications rather than relying only on accessible property values.

Cover source/effective-parser replacement, disabled navigation and destruction
while focused; retain old accessible handles/callbacks to prove stale activation
cannot emit an old destination. Test loading/failing/loaded images and geometry
changes without losing identity. Verify host action controls and boundary traversal
using a real QQuickWindow focus chain. Run ordinary documentation validation during
development and `python3 scripts/check-docs.py --release` after complete 1.10
release documentation is prepared.

## Actual results — 2026-10-15

Approval: the user response “approved” authorized requirements, design, tasks,
implementation and verification. Merge, tagging and publication remain excluded.

### Focused host verification

- Host Qt 6.12.0 built the library and new keyboard test. The final focused run
  passed keyboard, reading, selection, CommonMark oracle and strict CommonMark
  corpus checks (5/5 CTest entries). Both semantic layers retain all 652 official
  CommonMark passes with no mismatches or unresolved cases.
- Real QQuickWindow checks cover default-off opt-in, forward/reverse entry,
  boundary exit, no-link documents, modifier handling, nonrepeated Enter/Return,
  duplicate destinations, selection/copy and Find coexistence, native host action
  controls, and source replacement from a navigation handler.
- Native accessibility checks cover Link role, decoded names, destinations,
  nonempty global geometry, parent tree membership, focus state/notifications and
  press action with keyboard navigation disabled. Source and effective-parser
  changes reject retained old targets. Destruction while focused is exercised.
- Image checks cover denied descriptions, decoded images, adjacent empty-description
  linked images with identical destinations, one stop across text/image splits,
  wrapping/bidirectional content, resize and host focus movement during decoding.
- Style checks cover live default focus color, complete style replacement, stable
  target/focus identity and refreshed navigation requests when preceding layout
  moves a focused link. The regression failed before the rectangle-history fix
  and passed afterward.
- `build-shared/tests/keyboard-focus.png` was visually inspected. Two-pixel magenta
  outlines cover wrapped/bidirectional text fragments, the loaded image and
  trailing text; a separate unfocused link has no outline. A rendered image-border
  pixel assertion and unchanged content-height assertion also pass.
- Host QML lint passed with the existing import-only unused-import informational
  message. Ordinary and strict release documentation validation and whitespace
  checks passed. Final matrix lint results are recorded below.
- An installed host consumer passed actual Tab traversal, Enter,
  accessible tree/name/destination/geometry/activation and disabling Tab stops.
  New keyboard consumers are included in the configured packaging matrix for
  linked and plugin-only shared consumers and static consumers.
- The final working-tree library-only source archive was generated, extracted,
  configured, built and installed using host Qt 6.12.0. Artifacts are under
  `/tmp/qmarkdown048-final-source/`, `/tmp/qmarkdown048-final-extracted/`,
  `/tmp/qmarkdown048-final-archive-build/` and
  `/tmp/qmarkdown048-final-archive-install/`. The explicit source allowlist contains
  both new private coordinator files. An earlier pre-correction archive check is
  superseded by this final archive build.

### Configured matrix

Final local runs passed with exit 0 against the final runtime and packaging inputs:

| Target | Artifacts | Results |
| --- | --- | --- |
| Qt 6.8.0 | `build-ci-048-minimum-complete/` | 28/28 shared and 28/28 static CTest entries; packaging and benchmark smoke passed. |
| Qt 6.12.0 | `build-ci-048-development-complete/` | 28/28 shared and 28/28 static CTest entries; packaging and benchmark smoke passed. |

Both runners passed relocated installed consumers, including the new keyboard
consumer with and without direct shared-library linkage and with static linkage;
historical imports and compatible/exact package requests; QML payload inventories;
QML lint; cmark symbol privacy; library-only builds; and advisory benchmark JSON.
`result.json`, `ci.log` and `packaging/` retain actual evidence under each directory.
Both SDKs' shared native focus captures were visually inspected at
`packaging/build-shared/tests/keyboard-focus.png`.

Minimum-Qt lint retains the existing unresolved-native-type diagnostic categories,
including QColor diagnostics on the new focus-color bindings. Host/development
lint resolves those types and retains the import-only unused-import informational
message. Lint exits successfully on both targets.

Earlier `*-final/` runs passed before the rectangle-history correction; the
`*-complete/` runs supersede them. Runtime/packaging inputs were unchanged after
these final runs. A subsequent additional host keyboard regression passed for
selection-driven presentation rebuilding from a synchronous navigation handler;
the full final keyboard test also passed. The final-head hosted matrix includes
this additional coverage. Subsequent specification edits record results and
completion. The required final-head hosted PR checks remain the release gate under
[the development workflow](../../docs/development-workflow.md).

### Issues found and corrected

- Qt rejects removing Tab participation from an active focus item. Clear focus
  first; preserve focus across delegate replacement through the surviving view
  and across changes of the fragment that owns a target. Do not restore focus
  after a host control has taken it.
- Calling ensurePolished in every queued link refresh repeatedly triggered layout
  notifications. Consume the existing layout notifications instead.
- Rendering all outlines under one fragment could put later image rows above the
  outline. Text paints its own outlines, and image rows use a topmost border.
- Keeping zero-length image links in formatting spans violated the semantic oracle
  contract. Separate logical catalogs retain empty-description identities while
  parser formatting spans keep their original semantics.
- Native Loader focus scopes require explicit ID-qualified accessibility bindings.
  Keyboard Copy now reaches the view when its link target has active focus.
- Source replacement from a synchronous navigation handler can invalidate the
  target iteration. Snapshot target membership and check generation before
  continuing updates.
- Moving a focused item's ancestor changes mapped geometry before the refresh
  runs. Preserve the previously reported rectangle to detect the change and
  request visibility again.

The initial sandbox-blocked Docker attempt is retained at
`build-ci-048-minimum/`. The `*-verified/` runs failed the zero-length-link oracle
regression and are not passing evidence. No merge, tag or publication was performed.

## Completion and limits

T1–T7 implementation, documentation, focused verification and configured local
matrix checks are complete. The branch contains one focused PR change. All new
runtime APIs remain opt-in except accessible Link nodes. Native focus and
accessibility were exercised through Qt interfaces on Linux offscreen windows;
no external screen-reader session or additional platform run is claimed.
No merge, tag, release or downloaded-asset acceptance was performed.

## Authorized 1.10.0 delivery

The supplied release plan authorizes documentation updates and publication.
Historical results and exclusions above describe the earlier implementation run.

Planned checks: ordinary and strict release documentation validation, local links,
whitespace, and host Qt synchronization (selected SDK 6.12.0). Require both
packaging jobs and retained full release-gate artifact at the release PR final
head before squash merge. Dispatch release.yml with that merged PR while main
remains at its merge commit. Require source distribution builds on both Qt
targets, focused Arch checks, four stable/latest assets and downloaded hashes
matching the checked artifacts. Actual CI/publication results belong in Actions
logs; no evidence-only follow-up PR is planned.

Actual local documentation checks: ordinary and strict release validation passed,
including local-link checks; `git diff --check` passed. Host synchronization with
`/usr/bin/qmake6` confirmed 6.12.0 and made no configuration change. Existing
1.10 download URLs, four asset names and 1.x compatibility guidance were reviewed.
The UTC clock reports 2026-10-15, matching the finalized changelog date.

## Publication recovery and tooling repair — 2026-10-15 UTC

The supplied “Recover QMarkdown 1.10.0 and harden publication” plan explicitly
approved recovery, the requirements/design/task extension, one tooling fix PR,
and squash merge after its final-head configured CI passes. This is an extension
of feature 048 delivery; historical implementation exclusions above remain intact.

### Actual recovery

- Rechecked main and v1.10.0 at `efb2d35fbbbffbe7b45c7d11031072f753b4706d`.
  [PR #32](https://github.com/lebedenko/qmarkdown/pull/32) was merged; its
  [final-head gate](https://github.com/lebedenko/qmarkdown/actions/runs/38808154865)
  passed. Downloaded gate.json matched final head `ef9c22d`, tree
  `76c6f5059bce149658fe7824733f7605f86ad945`, package 1.10.0 and Qt 6.8.0/6.12.0.
- Source builds on both targets and focused Arch checks had passed. Retained
  distribution artifacts `12008740987` and `12008078577` were unexpired and were
  downloaded before retry; their source/Arch checksum companions matched.
  Draft `414298986` was stable, unpublished and empty. UTC October 15 matched
  the changelog before publication.
- Retried only failed job `116684585978` in
  [run 38810903894](https://github.com/lebedenko/qmarkdown/actions/runs/38810903894).
  [Publication retry job](https://github.com/lebedenko/qmarkdown/actions/runs/38810903894/job/116694751062)
  succeeded using the same retained artifact IDs. The overall run succeeded.
- Release ID `414298986` became published at `2026-10-15T22:02:01Z` and was returned
  by the latest-release endpoint, with prerelease=false and exactly four assets.
  [QMarkdown 1.10.0](https://github.com/lebedenko/qmarkdown/releases/tag/v1.10.0)
  preserves the original tag, commit and distribution bytes. Independently
  downloaded all four assets and compared SHA-256 against retained artifacts:

| Asset | SHA-256 | Result |
| --- | --- | --- |
| qmarkdown-1.10.0.tar.gz | `578f46218d2ee839d27f1360636e5c7a0b24bb235773a3d70dc38be9346a6cc6` | Match |
| qmarkdown-1.10.0.tar.gz.sha256 | `f1fba47275d7f399580bbc5b99ab86104bc6832e6511d51208da9e1968789a50` | Match |
| qmarkdown-1.10.0-1-x86_64.pkg.tar.zst | `a75126ee60fe5c135875e1ed99a11ef201db297f3672301c9c7abe3ebd5341fc` | Match |
| qmarkdown-1.10.0-1-x86_64.pkg.tar.zst.sha256 | `f345fa29db61835e0292d4b595708f680e9d0d71db5599d9ac346991b78ad3c0` | Match |

### Tooling verification

The delayed-draft-listing regression reproduced the original failure before the
fix: publication stopped at asset upload because tag rediscovery found no release.
The repaired publisher retains the creation response's numeric ID, and that
regression passes without another release-list request after creation.

Actual local checks:

- Upload tests: 22 passed. Cover paginated exact discovery, duplicate/missing
  matches, invalid IDs, changed identity/prerelease, identical bytes, partial
  uploads, stale asset lists, uncertain upload responses, mismatched bytes,
  unexpected/duplicate assets, permanent HTTP errors, transient HTTP/network
  failures, transport timeout and bounded discovery/refresh/upload exhaustion.
  Fake write boundaries reject any attempt to overwrite an existing name.
- Publication tests: 15 passed. Cover delayed draft visibility, uncertain creation
  with no second POST, delayed publication and uncertain PATCH with no repeated
  publication write, retry exhaustion, duplicate matches, existing draft and
  published-release recovery, moved main, differing tags/bytes and invalid docs.
- Release-delivery tests: 16 passed. Existing eligibility and deterministic source
  distribution contracts remain intact.
- Ordinary and strict release documentation validation and `git diff --check`
  passed. Host Qt synchronization confirmed 6.12.0 without configuration changes.

The tests use a fake GitHub service and patched sleeps; no new release or asset
was created to exercise the repaired tooling. Runtime, versions, dependencies,
workflow entrypoints/permissions and artifact flow are unchanged. Final-head
configured Qt CI remains required before the authorized tooling PR squash merge;
its actual hosted results remain in the PR checks and Actions logs, without an
evidence-only follow-up PR.
