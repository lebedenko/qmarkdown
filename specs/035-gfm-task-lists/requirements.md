# Requirements

Approved on 2026-10-13 through the explicit user reply "approved". Scope covers local implementation and verification of requirements, design and tasks; Commit, push and PR creation were subsequently authorized on 2026-10-13 through "commit, make a PR". Merge and publication remain separately authorized delivery actions.

## Scope

Begin selected GFM support with task lists. Tables, strikethrough and extended autolinks follow in separately approved iterations. CommonMark 0.31.2 remains the core; this is not full GFM conformance. Syntax reference: [GFM 0.29-gfm section 5.3](https://github.github.com/gfm/#task-list-items-extension-).

R1: Add writable `MarkdownView.taskListsEnabled`, default false. Enabling/disabling reparses the current source; `markdown` retains the original input. Existing imports and default rendering remain compatible.

R2: When enabled, recognize task markers only at the beginning of the first paragraph of a list item, following section 5.3's whitespace and marker rules. Checked markers accept x/X. Support ordered/unordered, mixed ordinary/task, nested, quoted, tight and loose lists. Escaped/entity-produced brackets, code, HTML, standalone paragraphs and later paragraphs are not task markers. Preserve remaining inline formatting, links and images.

R3: Render a native read-only checkbox indicator alongside the existing list marker. Remove the recognized task prefix from displayed paragraph text. Preserve ordered numbering, nesting and spacing. Empty task content reserves a body line. Indicators follow body color/font metrics, fit the view's existing narrow-width policy, and expose checked/unchecked state to accessibility. They do not accept focus, pointer activation or edit the source.

R4: Keep parsing/model semantics, native presentation and host resource permissions separate. Retain literal HTML, host-controlled navigation, deny-by-default images, styling defaults and code-copy behavior.

R5: Verify extension semantics independently and keep the strict 652-example CommonMark gate unchanged with extensions disabled. Deliver implementation, tests, documentation and actual evidence together in one meaningful change under the current development workflow.
