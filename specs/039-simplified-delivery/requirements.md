# Requirements — simplified CI and release delivery

Approved on 2026-10-13 through the user-supplied implementation plan. Scope includes
implementation, tests and verification of this workflow migration; no runtime API
change, version bump, new release, distro submission or historical asset mutation.

- R1: One complete PR matrix on exact minimum/development Qt, preserving required
  job names; lightweight documentation-only jobs; no main/manual/publication full suites.
- R2: Explicit local host-qmake synchronization; documentation validation rejects
  stale current-release wording and retains historical records.
- R3: Final release PR full CI authorizes focused distribution checks only when the
  latest final-head run passed, Qt/version inputs and tested/merged trees match and
  main remains at the merged commit. Reject stale or changed inputs.
- R4: Deterministic library-only source archive from an explicit file allowlist;
  standalone CMake builds/installations on both Qt versions; same source feeds Arch.
- R5: Arch runs focused metadata/dependency/payload/ownership/consumer/removal checks
  once; no full-suite makepkg checks or future Ubuntu/custom installer/runtime binary checks.
- R6: After artifact checks, tag exact commit, upload draft source/Arch/checksum
  assets, publish stable/latest, verify downloaded hashes. Retry only missing uploads;
  differing published bytes or tags require explicit repair. No evidence attachments.
- R7: Correct released 1.6.0 summaries and installation guidance, preserve all
  historical verification records, tags, releases/assets and development viewer.
