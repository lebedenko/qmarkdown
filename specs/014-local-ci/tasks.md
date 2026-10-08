# Tasks

Approved on 2026-10-09 by the user's explicit approval of requirements, design and T1–T4. Scope includes moving GitHub verification into the shared container; package/module remain 0.7.0/0.7.

- [x] T1 (R1–R2): Add the Docker wrapper and shared container entry point; resolve and pin the official Ubuntu 24.04 image digest; add the two Task aliases.
- [x] T2 (R2–R3): Make the GitHub matrix call the same wrapper; preserve triggers, 35-minute limit, pinned checkout/upload actions and failure evidence.
- [x] T3 (R4–R5): Document prerequisites, downloads, artifact ownership/paths and parity limits; ignore local artifacts through the existing `/build*/` rule; update specifications.
- [x] T4 (R1–R5): Check Task selection and scripts/workflow structure; verify source mounts, error propagation and artifact persistence; run both exact Task commands through Docker and record actual results separately. No GitHub workflow was dispatched or pushed.

Completed on 2026-10-09. Both Task commands passed the shared Ubuntu container pipeline; see verification.md for full results and the CMake 3.28 package-path defect found and fixed during verification. Remote workflow execution remains pending.
