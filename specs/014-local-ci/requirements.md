# Requirements

Approved on 2026-10-09 by the user's explicit approval of the prepared requirements, design and T1–T4. Scope includes moving GitHub verification into the shared container. Package/module remain 0.7.0/0.7.

- R1: Provide the exact Task commands `ci-6.8` and `ci-6.11`, selecting Qt 6.8.0 and 6.11.3 respectively. Run the current working tree, including uncommitted edits, without a push or GitHub token.
- R2: Local tasks and GitHub verification use one digest-pinned Ubuntu 24.04 Linux amd64 container image and the same repository-owned provisioning and verification commands. Preserve the two-version matrix, workflow triggers, timeout and failure evidence. Changing GitHub execution from host VM tools to container tools is part of this scope.
- R3: Provision the existing Ubuntu dependencies and Noto fonts, and pin aqtinstall 3.3.0 / py7zr 0.22.0. Select the requested SDK explicitly. Run complete shared/static packaging, relocated direct/plugin/static consumers, library-only builds, QML lint, symbol checks and validated Release benchmark smoke checks.
- R4: Preserve logs and reports in separate ignored local artifact directories for each version/run, return failure if any stage fails, and report the image digest and actual environment. Keep source read-only inside the container, avoid host build/Qt/environment contamination, and leave artifacts owned by the invoking user.
- R5: Explain Docker requirements, first-run downloads, artifact locations and parity limits. A passing local run checks container commands, not hosted checkout/upload actions, the host kernel or future external package changes. No guarantee that every possible GitHub failure is prevented.

Excluded: application/library changes, package version changes, automatic commits/pushes, publishing images, registry credentials, act installation, other operating systems/architectures, and cache infrastructure.
