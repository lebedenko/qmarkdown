# 062 — Mapped tasks

Status: explicitly approved by the user on 2026-10-19.

| Task | Requirements | Work and checks |
| --- | --- | --- |
| T1 | R1–R3 | Add private parser option, container-aware definitions/references, source retention and shared label normalization; test indentation, protection, duplicates and default-off CommonMark. |
| T2 | R2–R4, R6 | Add owned nodes and document-wide resolution/numbering; append each referenced body once; assert forward/repeated/undefined/unused behavior and exact projection. |
| T3 | R4, R6 | Add native superscript reference atoms and endnote entries with existing block styles/resources/renderers; verify geometry, rich bodies, tables/images, math/highlighting and stable offsets. |
| T4 | R5 | Allocate generated IDs after headings, register note/reference targets and per-occurrence backlink links; test collisions, outline, navigation signals, keyboard focus and accessibility. |
| T5 | R1, R7 | Add QML 1.16 opt-in, compatibility metadata, playground toggle/sample and guides/roadmap; verify independent GFM and older imports. |
| T6 | R7 | Run focused then affected suites, installed shared/static consumers, backend interactions, lint/docs and exact Qt 6.8.0/6.12.0 gates after toolchain sync; record actual results and native-input limits. |
| T7 | R7 | Review and open one PR containing implementation/tests/docs/evidence; require final-head minimum/development packaging checks. Do not merge, tag or publish. |

Approved scope: requirements.md, design.md and T1–T7 for implementation and
verification. Any changed approved scope requires renewed approval. Feature 063
and all merge/publication actions remain outside this approval.

Approval record: the user selected “Approve Feature 062” for requirements, design
and T1–T7 on 2026-10-19. Implementation and verification are authorized after
Feature 061's required CI checks pass. Merge, tagging and publication remain
excluded. No footnote runtime implementation had started at approval time.

## Implementation status

T1–T5 are implemented. T6 local shared/static, parser, native/offscreen and
relocated-consumer checks pass; see verification.md for actual results and native
input/local-container limits. T7 PR delivery and the final-head hosted minimum/
development packaging gates remain. Record their results in the implementation
PR; merge, tagging and publication remain excluded.
