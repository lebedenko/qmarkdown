# Mapped tasks

Approved on 2026-10-13 through the explicit user reply "approved". Scope covers local implementation and verification of requirements, design and tasks; Commit, push and PR creation were subsequently authorized on 2026-10-13 through "commit, make a PR". Merge and publication remain separately authorized delivery actions.

| Task | Requirements | Checks |
| --- | --- | --- |
| T1: Confirm integration baseline and record approved scope | R5 | Current workflow, user edits preserved, approval recorded |
| T2: Add opt-in raw-prefix recognition, private task metadata and provenance | R2, R4 | Independent valid/invalid marker cases, strict default CommonMark corpus, hidden symbols |
| T3: Add ViewState option, reparsing and model role; preserve image projection | R1, R2, R4 | Toggle/replacement/notification tests, resource lifecycle regressions |
| T4: Add native read-only indicators to recursive list rows | R3, R4 | Mixed/nested/ordered layout, narrow/zero widths, live style, accessibility state and inert interaction |
| T5: Add viewer toggle/sample, 1.3 candidate metadata, installed consumer and user guide | R1–R5 | Viewer QML checks, new and historical import fixtures, shared/static consumers |
| T6: Verify candidate and record actual results in this feature | R5 | Focused tests, full relevant host suite, minimum/development CI and packaging |
| T7: Prepare one reviewable feature PR when delivery is authorized | R5 | Required checks and review resolution; squash by default; no separate evidence initiative |

T2 precedes T3 and T4; T5 follows the settled API; T6 covers the final inputs. Later selected extensions are future work, not unchecked tasks in this iteration.

## Actual status

T1–T6 complete for the approved local scope on 2026-10-13. Host shared/static and both configured Qt container gates pass; see [actual verification](verification.md) for results and bounded evidence. T7 is authorized and in progress: prepare the feature commits and PR, then observe required hosted checks. Merge, tag and release publication have not been requested. A fresh committed Arch candidate lifecycle remains a delivery check as explained in the verification record.
