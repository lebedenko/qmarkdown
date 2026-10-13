# Design

Approved on 2026-10-13 by the user instruction to implement the supplied iteration 036 plan. Approval covers these requirements, design, mapped tasks, local implementation and verification. PR creation, merge and publication require separate authorization.

Extend bundled cmark delimiter processing with a private option and inline node. Scan raw tilde runs using existing Unicode flanking rules. Only runs of length one or two enter the delimiter stack; matching requires equal lengths without the emphasis modulo-three rule. Maintain separate opener bounds for each length and opener capability; reuse inline splicing to consume the complete run. Option-aware text scanning leaves default CommonMark untouched. Code, HTML tokens and decoded escapes/entities bypass delimiter recognition.

Adapt the node into owned inline semantics and a new additive flag. Extend interval counters and apply QFont strikeout after resolving the existing body/code font. Links retain their destinations, colors and hit testing. ViewState owns the public alias and reparses original source using both independent switches and the existing resource lifecycle. No public style properties or dependency changes.

Trade-off: maintain a small authored private cmark patch instead of replacing the parser or preprocessing source, preserving syntax precedence and the established corpus. Base: origin/main `d4b0afd` (merged task-list PR #14).
