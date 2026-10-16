# Requirements — Language-aware headers

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Publish QMarkdown 1.11, then implement 1.12 code-block usability” plan. Scope includes Features 050–053 and one package 1.12.0 / QML 1.12 release PR. Merge, tagging and publication of 1.12 require separate authorization.

Preserve the case-sensitive first info token. Resolve existing aliases to canonical identifiers and friendly names with either backend configuration. Unknown tokens remain unchanged; empty and indented blocks have empty metadata. Extend headers with optional language, languageId, languageName, infoString and fenced properties while preserving complete-source copy and old contracts.
