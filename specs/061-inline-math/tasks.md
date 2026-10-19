Approval: explicitly approved by the user on 2026-10-19 for implementation and
verification of requirements, design and T1–T7. Merge/tag/publication excluded.

# 061 — Mapped tasks

Status: approved for implementation and verification.

| Task | Requirements | Work and acceptance checks |
| --- | --- | --- |
| T1 | R1–R3 | Add default-off raw parser node/options, source spans, image-description reparsing and table-aware recognition; delimiter/protection/parser fixtures and disabled CommonMark baseline. |
| T2 | R2, R5–R6 | Adapt owned nodes and UTF-16 formula projections; propagate through presentation/text-image slices; exact model/copy/search/outline and multibyte offset assertions. |
| T3 | R4, R7 | Extract cancellable math request service, text mode, baseline metrics and cache mode identity; block regression, limits, cancellation and backend OFF tests. |
| T4 | R4–R7 | Add measured atoms, width reservations, line composition and bidirectional mappings to FormattedText; baseline, wrapping, bidi, narrow fallback, links, atomic selection and asynchronous offset assertions. |
| T5 | R1, R6, R8 | Expose QML 1.15 opt-in; update package metadata, deterministic playground and guides; verify independent toggles, imports and existing interactions. |
| T6 | R8 | Run focused then affected suites, lint/docs, installed shared/static consumers, backend ON/OFF and exact Qt 6.8.0/6.12.0 gates after toolchain sync; record actual evidence and explicit unavailable native-input acceptance. |
| T7 | R8 | Review final diff, keep implementation/tests/docs together in one PR and require final-head packaging checks. Record PR/check links. Do not merge, tag or publish. |

Approval scope: requirements.md, design.md and T1–T7, including implementation
and verification only. A scope change requires renewed approval. Later features
062 and 063 require their own approval before implementation.

## Implementation status

T1–T5 are implemented. T6 host suites and relocated consumers pass; local exact
container preparation is blocked as recorded in verification.md. T7 PR delivery
and required final-head hosted packaging gates remain. Record final delivery
results in the implementation PR without an evidence-only follow-up commit.
