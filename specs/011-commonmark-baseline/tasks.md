# 011: CommonMark conformance baseline tasks

**Approval:** On 2026-10-08 the user explicitly approved these requirements, design and tasks. Scope: T1–T6, test infrastructure and documentation only; package/module 0.7.0/0.7 and production behavior remain unchanged. Production fixes require subsequent approval.

| Task | Requirements | Deliverable and check |
| --- | --- | --- |
| T1 | R1 | Acquire pinned official fixture, record checksum/provenance/license, validate schema and complete unique IDs; verify offline reuse. |
| T2 | R2 | Add private parser/model probe using prefixed bundled objects; exact official HTML comparison with raw HTML enabled only in test serialization; verify allocation cleanup and no installed/public helper. |
| T3 | R3, R4 | Serialize production model, implement independent expected-HTML projection and authored oracle/model fixtures; check Unicode offsets, nesting, list tightness, empty links/images, title loss, HTML and source-only fields. |
| T4 | R2–R5 | Run every example, manually review mismatches/uncheckable distinctions, create per-ID ledger, deterministic JSON/summary and baseline/strict exit policies. Exercise deliberately corrupted fixture, output, ledger and expectation cases. |
| T5 | R5, R6 | Integrate baseline in CTest and document one offline runner command plus strict mode; update standards/current status without claiming full conformance. Confirm tests-disabled builds require no Python or fixtures. |
| T6 | All | Run focused oracle/parser/model checks first, then full shared/static CTest, QML lint and symbol privacy. Run relocated packaging checks if test integration changes installed/exported targets; otherwise document unchanged packaging boundaries. Record actual results, environment limits and prioritized follow-up gaps. |

## Approval boundary

The recorded approval authorizes T1–T6 and test/documentation changes only. Production parser, model and rendering fixes discovered by the baseline are excluded. Obtain renewed approval of a concrete follow-up specification before implementing those changes.

T1–T6 completed on 2026-10-08. Actual counts, commands, review findings and environmental limits are recorded in [verification](verification.md).
