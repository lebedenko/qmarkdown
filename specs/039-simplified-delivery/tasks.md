# Tasks

Approved on 2026-10-13 through the user-supplied implementation plan.

- T1 (R1–R2): Simplify PR jobs/container commands, add scope/docs checks and host Qt synchronization.
- T2 (R3): Implement and test final PR CI eligibility and retained tested-input gate.
- T3 (R4–R5): Implement deterministic allowlisted archive, library-only CMake defaults,
  focused dual-Qt source checks and archive-fed Arch lifecycle checks; retire Ubuntu tooling.
- T4 (R6): Implement/test draft upload, bounded retry, stable publication and downloaded hashes.
- T5 (R7): Correct released 1.6.0 summaries, installation and development guidance.
- T6 (R1–R7): Narrow offline tooling tests, disposable-container source/Arch checks,
  complete configured Qt matrix once on final inputs; record actual results separately.

No test release may be published. Remote PR CI results must not be inferred from
local checks. Preserve existing releases/assets/tags and verification records.

T1–T5 are implemented. T6 offline tooling, syntax/documentation and focused
source/Arch container checks passed; see verification.md. The final hosted PR
matrix remains the delivery check and is not preclaimed here.
