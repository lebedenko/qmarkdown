# Requirements

Approved on 2026-10-11 through the explicit instruction to implement the supplied v1.1.0 plan.

- R1: One committed configuration pins minimum Qt 6.8.0 and development Qt 6.12.0; no floating SDK selection or silent substitutions.
- R2: Standard-library validation and role resolution serve local runs, provisioning, container verification and hosted matrices. Preserve exact --qt selection, provenance and cache isolation; fingerprint the configuration and helper.
- R3: Provide ci-minimum, ci-development and the retained ci-6.8 alias; document replacement of ci-6.11.
- R4: Stable hosted packaging (minimum)/(development) contexts. After both pass, review the live main ruleset and replace only the two old required contexts, preserving protections. Prepare a reviewable source PR and ruleset update before external mutation.
- R5: Release archives remain built on minimum Qt; verify identical minimum-built bytes on both runtimes. No product version bump.
- R6: Exact 6.12.0 Ubuntu 24.04 SDK provisioning, helper/runner tests, complete suites on both targets, hosted checks and release cross-runtime verification must pass. Provisioning failure blocks completion.
