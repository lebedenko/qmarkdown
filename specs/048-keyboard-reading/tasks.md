# Tasks — keyboard reading and link accessibility

Status: **approved on 2026-10-15** by the user response “approved”. Approval
covers requirements, design and T1–T7 implementation and verification; merge and
publication remain separately authorized. Unchecked tasks are not yet complete.

| Task | Requirements | Work and acceptance checks |
| --- | --- | --- |
| T1 | R4, R6 | Add logical catalog/generation and propagate IDs before image projection; test duplicate destinations, text/image split, nested/table/autolinks and inert image-description descendants. |
| T2 | R1, R2, R4, R6 | Implement private targets and registration lifecycle; test forward/reverse entry, boundary exit, no links, host actions, modifiers, single activation, invalidation and destruction. |
| T3 | R3, R6 | Add public color/opt-in plumbing, fragment outlines and navigation requests; test wrapped/bidirectional geometry, resize/style/decode stability and focus loss/disablement. |
| T4 | R5, R6 | Implement accessible Link interface and notifications; inspect actual tree membership, unique nodes, names, destinations, geometry, focus and stale activation guards with opt-in both on and off. |
| T5 | R1–R7 | Add playground examples and focused interaction/accessibility regression coverage, including pointer/selection/Find coexistence and renderer controls. |
| T6 | R7 | Update API/reader/playground documentation, release notes, compatibility and current summaries together; preserve historical records and run ordinary/strict documentation gates at their appropriate stages. |
| T7 | R1–R7 | Run focused tests then lint and configured Qt 6.8.0/6.12.0 shared/static, installed-consumer and packaging checks; record actual outcomes and limitations in verification.md; prepare one focused PR. |

- [x] T1
- [x] T2
- [x] T3
- [x] T4
- [x] T5
- [x] T6
- [x] T7

Merge, tagging and publication remain separately authorized.
