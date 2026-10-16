# Design — Code-block wrapping

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Publish QMarkdown 1.11, then implement 1.12 code-block usability” plan. Scope includes Features 050–053 and one package 1.12.0 / QML 1.12 release PR. Merge, tagging and publication of 1.12 require separate authorization.

Carry view settings through ViewState. Add wrapEnabled and logicalWidth to native FormattedText; use native QTextLayout NoWrap with explicit separators. Plain Text uses matching wrap modes. Source viewport owns horizontal offsets.
