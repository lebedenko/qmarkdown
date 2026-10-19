# 059 — Tasks

Approved on 2026-10-19 by the user-supplied “Refresh the playground and README” implementation plan. Approval covers requirements, design, mapped tasks, implementation and verification on a local feature branch; no push, PR, merge or release.

- R1: Convert samples and tests; verify loading and obsolete entry removal.
- R2: Add scrolling/task fixtures and artwork control; run scrolling, task, small-window, style, palette, scale and selection checks.
- R3: Bind availability, add task and guidance; test backend OFF/ON and rendered palettes where available.
- R4: Refresh README and guides; run docs checks.
- Run focused tests, shared/static suites and lint; attempt configured Qt packaging gates and record actual limitations separately.

Implementation tasks R1–R4 are complete. Actual checks and unconfirmed direct-sample/physical input behavior are recorded in verification.md; configured container packaging gates were attempted and blocked by Docker image preparation. No publication action was taken.

- R5 (explicitly approved 2026-10-19): update shared demo startup defaults and guidance; verify actual loaded options with backend OFF/ON, existing toggle/reset behavior, QML lint and documentation checks.

- R6 (approved 2026-10-19): the subsequent combined 1.14 table-presentation plan authorizes including the entire pending d11635a commit in one development PR. This extends delivery authorization without changing the historical local-only approval above. No merge, tag or publication is authorized.
