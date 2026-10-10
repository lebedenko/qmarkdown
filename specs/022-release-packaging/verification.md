# Verification

## Planned gates

API-read the active ruleset; run both existing pinned container suites and new
archive/installer verification; consume the same Qt 6.8 archive on both pinned
Qt runtimes; lint workflows and exercise upload rerun policy without publishing.
Real `/usr` operations must run only inside disposable containers.

## Actual results — 2026-10-10

Implementation and local verification passed on `feat/release-packaging`.
At completion of initial local verification, no commit, push, tag or release had
been created. The user subsequently authorized committing and creating a PR.
Hosted checks run on that PR; release publication remains outside this scope.

- Created [Protect main](https://github.com/lebedenko/qmarkdown/rules/24818691)
  with the GitHub API and independently read it back. Assertions against the
  [saved response](main-ruleset.json) confirm active main targeting, zero approvals,
  resolved conversations, strict required checks from GitHub Actions app 15368,
  deletion/force-push blocking and no bypass actors. API reports this administrator's
  normal-operation bypass as `never`. Repository merge settings were not changed.
  Administrators retain permission to edit rules. No direct push was attempted.
- `python3 scripts/run-ci.py --qt 6.8.0 --artifact-dir /tmp/qmarkdown-022-qt680`
  and the corresponding `--qt 6.11.3 --artifact-dir /tmp/qmarkdown-022-qt6113`
  both passed. After installer record validation and archive-header normalization
  corrections, repeated the full 6.8.0 suite with
  `--artifact-dir /tmp/qmarkdown-022-final680`: passed in 211 seconds.
- Each suite passed all seven CTest entries in shared, static and Release archive
  builds (21/21 entries per suite), QML lint, parser symbol privacy, relocated linked
  and plugin-only consumers, version-discovery checks, library-only configuration
  and benchmark smoke. In each of the six latest shared/static/archive CommonMark
  reports, parser and model pass all 652 examples with zero failures, mismatches,
  uncheckable cases, aggregate limits or losses.
- The final archive has 16 payload entries: 14 files and two library symlinks.
  Verified the external SHA-256, one enclosing directory, normalized root-owned
  archive headers, executable entry scripts, payload modes/hashes/link targets,
  x86_64 ELF headers, resolved dependencies, relative plugin RPATH, CMake/QML
  metadata without source/build/SDK path leaks, notices, and absence of static
  libraries, Qt libraries, private headers, examples and tests. Its installer bytes
  match the final repository installer.
- Installer lifecycle checks passed: fresh/repeat install, synthetic previous-version
  content upgrade, obsolete owned-file removal, modified-file upgrade refusal before
  mutation, preservation and diagnostics on uninstall, clean uninstall, unrelated
  file/directory conflict refusal and symlink-ancestor refusal. Staging uses an
  absolute root and retains the `/usr` layout.
- Ran the final 6.8.0-built archive through the latest verifier in disposable
  containers with Qt 6.8.0 and 6.11.3. Both report
  `PASS: archive, installer lifecycle and installed consumers`. Each built and ran
  linked and plugin-only consumers in staging and real `/usr`, verified no QMarkdown
  dependency in the plugin-only executable, checked root ownership, repeated real
  install, and uninstalled successfully. Only the Qt SDK library directory was added
  to `LD_LIBRARY_PATH`; QMarkdown resolves through installed linker/plugin paths.
  Real operations invoked `ldconfig`. The host `/usr` was never modified.
- `python3 tests/test_release_installer.py`: 8/8 passed (including corrupt payload,
  traversal, escaping links, payload symlink ancestors, record temporary conflicts,
  modified symlink preservation and architecture rejection).
  `python3 tests/test_release_upload.py`: 4/4 passed (identical, differing, missing
  assets and prerelease rejection using fake GitHub responses).
  `python3 tests/test_ci_runner.py`: 13/13 passed.
- `actionlint` 1.7.7 passed both workflow files. Python compilation, Bash syntax,
  workflow YAML/embedded-shell syntax and `git diff --check` passed.

[Machine-readable results and exact cross-runtime commands](run-evidence.json)
record toolchain identities, logs, archive size and SHA-256. Local builds use a
snapshot including uncommitted feature changes; their metadata source commit is
its parent HEAD. Published builds use the exact checked-out release-tag commit.
Temporary artifacts are local evidence, not GitHub release assets.

Existing optional Vulkan/Qt TaskTree discovery warnings, informational unused QML
imports, and mixed font-unit warnings remain nonfatal. Qt 6.11.3 results establish
compatibility for this archive and tested environment only. Live asset uploads and
hosted workflow execution were not performed; upload policy was tested without
network writes. No v1.0.0 tag or first release was created.

## Subsequent hosted verification

[PR #1](https://github.com/lebedenko/qmarkdown/pull/1) merged as
`1277478aebbc9e76c0d6f4efc4c9220824f19aae`.
[Run 37999664817](https://github.com/lebedenko/qmarkdown/actions/runs/37999664817)
verified PR head `e65b4d3193ed158dedfc3184484875a0f6ffdeb3`: both
`packaging (6.8.0)` and `packaging (6.11.3)` succeeded. The historical
local-only statements above describe the original verification session.
First stable publication is now explicitly authorized under Feature 023.
