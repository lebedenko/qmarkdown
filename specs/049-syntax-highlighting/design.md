# Design — syntax highlighting

Approval: approved on 2026-10-16 through the user instruction to implement the supplied “QMarkdown 1.11: syntax highlighting” plan. Approval covers requirements, design, mapped tasks, verification and one package 1.11.0 / QML 1.11 release PR. Merge, tagging and publication require separate authorization.

SyntaxTheme is a public QML-only QObject palette. Unset operator color is invalid and resolves to existing code foreground. ViewState owns fallback/theme lifetime plumbing. CodeSurface owns a private request subscription and binds semantic spans to palette colors; palette edits never retokenize. Custom visuals suppress the subscription until source is shown.

A private adapter returns role spans against exact parser source. AbstractHighlighter uses Format::textStyle(), carries State between lines and normalizes ranges before delivery. Roles map control flow to keyword, character/string variants to string, numeric variants to number, documentation/annotations/comment variants to comment, imports to preprocessor, attributes to type, extensions to built-in and diagnostics to error; normal/other retain foreground.

An application-owned scheduler serializes requests on one worker, creates its repository on that thread, deduplicates identical active/pending jobs, prunes invalidated subscriptions and caches completed semantic results with an LRU byte/entry cap. Delegates cancel on source/language/enablement/lifetime changes; queued callbacks validate generation. Only application shutdown waits for the worker. Processing limits are private and whole-block; cancellation/time checks cannot forcibly interrupt an individual backend call.

FormattedText consumes separate foreground ranges in layout and glyph painting; overlays remain independent. One terminal newline is clipped only at display time. CMake links KDE privately, discovers it for installed static consumers only, and never discovers it in disabled builds. CI builds pinned dependency archives for each exact SDK rather than mixing host Qt ABIs.
