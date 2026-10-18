# Design — Atomic math selection

Approval: the user explicitly requested implementation of the supplied atomic math selection plan on 2026-10-18. This approves the requirements, design and tasks below. Version changes, merge, tagging and publication are excluded.

Append private MathSelection without changing existing enum values. Include it in selection and navigation geometry. Reuse non-text boundary positioning and return its entire registration range for both repeated-click granularities.

CodeSurface registers only visible ready formulas; its existing source loader owns fallback registration. A noninteractive child rectangle uses projection overlap and selectionBackgroundColor, inheriting viewport clipping. Separate registered renderer membership from selection/Find-gated activation; native fenced math requires absence of a registry entry. No selection mutation occurs during presentation changes.

Supersedes only selection-enabled source-mode clauses in features 055/056; historical approvals and results remain intact.
