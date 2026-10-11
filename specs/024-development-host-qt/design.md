# Design

Approved on 2026-10-11 through the explicit instruction to implement the supplied v1.1.0 plan.

Store role/version JSON and its strict standard-library helper inside scripts/ci-toolchain. Validate exact numeric versions and increasing role order. Expose individual roles, exact-version validation and an include matrix. A prerequisite workflow job checks out the same source revision and publishes the matrix and minimum archive target. Packaging names use roles; images and caches remain keyed by exact versions and recipe fingerprints. Docker copies helper/config beside qt.sh; source-side container initialization validates against the same configuration. Local CLI makes exact version and role mutually exclusive.

Ruleset migration is contingent on passing new hosted checks and a reviewable PR. Preserve the existing minimum-built release archive selection in compatibility/upload jobs. SDK failures remain explicit; no alternative SDK is authorized.
