# Requirements

Approved on 2026-10-13 by the user instruction to implement the supplied iteration 036 plan. Approval covers these requirements, design, mapped tasks, local implementation and verification. PR creation, merge and publication require separate authorization.

R1: Add writable, independent, default-off `MarkdownView.strikethroughEnabled`; equal assignments are inert, toggles preserve source and markdown notifications, and reparse through the resource lifecycle.

R2: Retain CommonMark 0.31.2 and recognize raw one/two-tilde runs only when enabled, with emphasis flanking, equal-length pairing and existing block/link/code precedence. Longer, escaped, entity-generated, unmatched and mismatched runs remain literal.

R3: Retain private strike nodes and additive flags through projection, images and intervals. Render native strikeout composed with existing font, color, emphasis, code and link behavior.

R4: Advance source candidate to 1.4.0 / QML 1.4, preserve historical imports, add playground controls, documentation and installed consumers.

R5: Require separate GFM evidence and unchanged 652-example default CommonMark gate, host shared/static checks and configured Qt 6.8.0/6.12.0 container gates.

Tables, autolinks, streaming, custom strike styling and full GFM conformance are excluded. Exact-commit Arch lifecycle remains a release delivery gate; 1.4.0 is not published.
