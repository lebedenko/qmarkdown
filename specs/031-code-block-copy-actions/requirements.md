# Requirements

Approved on 2026-10-12 through the explicit instruction to implement the supplied iteration 030 plan. Scope is local implementation and verification; integration, commits, publication, tags and uploads are excluded.

- R1: Optional MarkdownView.codeBlockActions Component (null by default) and codeBlockCopyRequested(string text), for all fenced/indented code including nested and empty blocks.
- R2: Item delegates declare required string codeText and required var requestCopy; each request emits the exact parser text without fences/info strings or further normalization.
- R3: Header sits above code inside surface padding, with available content width and finite nonnegative implicitHeight. Null preserves dimensions. Hosts own spacing, styling, accessibility, clipboard and feedback; library adds no Controls dependency.
- R4: Replacement/removal destroys instances and invalidates old callbacks; failure diagnoses without preventing ordinary code rendering.
- R5: Playground host Copy control writes plain text with brief Copied feedback and mouse/touch/keyboard activation.
- R6: Prepare package 1.2.0/QML 1.2, retaining 1.0/1.1 consumers, compatible earlier CMake requests and EXACT 1.2.0. Extend installed/release/Arch consumer checks.
- R7: Verify exact text, routing, lifecycle, geometry, playground clipboard/keyboard, shared/static/lint/relocation and both pinned Qt suites/CommonMark. Record actual results separately.

Document-wide selection, inline-code actions, labels, highlighting and generic extensions remain deferred.
