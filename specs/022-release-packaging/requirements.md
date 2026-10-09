# Requirements

Approved on 2026-10-10 through the explicit instruction to implement the supplied plan. Scope includes repository rules and release tooling; creating a tag or publishing the first release is excluded.

- R1: Active main ruleset: PRs, zero approvals, resolved conversations, strict GitHub Actions checks packaging (6.8.0)/(6.11.3), no deletion/force push/bypass. Preserve merge methods.
- R2: Library-only Ubuntu 24.04 x86_64 Release shared archive built with Qt 6.8.0, /usr multiarch layout via DESTDIR; include QML metadata, CMake exports, symlinks and notices. Reject path leaks, private headers and bundled Qt.
- R3: Validated manifest and architecture, root real installation, unprivileged absolute --destdir, tracked hashes/targets, conflict and modification refusal before mutation, upgrade obsolete removal, conservative uninstall, root ownership and ldconfig.
- R4: Stable published release trigger, matching vX.Y.Z and main ancestry, read-only builds, upload-only write permission, pinned actions, serialized tags, identical-only asset reruns, manual build without publication, retained failure evidence.
- R5: Both existing suites plus archive/installer checks, disposable-container /usr installation, installed linked/plugin consumers on Qt 6.8.0 and the same binary package on 6.11.3. Document prerequisites, discovery, ownership and source alternative.
