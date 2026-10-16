# Design — Optional source-line gutter

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Publish QMarkdown 1.11, then implement 1.12 code-block usability” plan. Scope includes Features 050–053 and one package 1.12.0 / QML 1.12 release PR. Merge, tagging and publication of 1.12 require separate authorization.

Expose first-row source-line baselines from native QTextLayout, including paint offsets. Use native renderer whenever numbers, selection or highlighting require it; keep plain path otherwise. Paint visible numbers only, subtract gutter width before layout, synchronize vertical offset and exclude custom visuals. Palette/gutter toggles do not retokenize syntax.
