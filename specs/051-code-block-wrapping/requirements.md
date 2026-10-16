# Requirements — Code-block wrapping

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Publish QMarkdown 1.11, then implement 1.12 code-block usability” plan. Scope includes Features 050–053 and one package 1.12.0 / QML 1.12 release PR. Merge, tagging and publication of 1.12 require separate authorization.

Expose view-wide codeBlockWrapEnabled, default true, independently of syntax highlighting. Preserve tabs, Unicode, alignment and explicit newlines; omit one terminal display newline. Nowrap exposes measured horizontal overflow. Inline code and custom visuals retain their existing behavior.
