# 009: Requirements

**Approval:** On 2026-10-08 the user explicitly requested implementation of the supplied Feature 009 plan, approving these requirements, design and tasks for package 0.6.0 / module 0.6 before implementation.

- R1: Inline/resolved-reference/URI/email links in paragraphs, headings and recursive containers emit parser-decoded strings through `MarkdownView.linkActivated(string destination)`. Empty, relative, fragment and arbitrary schemes are valid. Hosts own all navigation policy.
- R2: Notifying `MarkdownStyle.linkColor` (#0066cc) and `linkUnderline` (true) compose with fonts, emphasis, strong and code.
- R3: One activation per primary click/touch tap; pointing-hand hover; scrolling and canceled/replaced gestures never activate.
- R4: Image descriptions preserve formatting without resource loads. Descendant links are inert; enclosing links include image descriptions. Code and literal HTML are inert.
- R5: Playground reports plain-text destinations and exposes palette-bound styling/presets/reset. Deliver 0.6.0/0.6 without dependencies or parser-source edits.
Keyboard traversal, accessibility, selection, tooltips, visited states, resource rendering and automatic navigation are deferred. Full CommonMark conformance is unclaimed.
