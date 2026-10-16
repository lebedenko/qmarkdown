# Requirements — Bounded code-block scrolling

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Publish QMarkdown 1.11, then implement 1.12 code-block usability” plan. Scope includes Features 050–053 and one package 1.12.0 / QML 1.12 release PR. Merge, tagging and publication of 1.12 require separate authorization.

Expose height limiting (default false), viewport height (0), ratio (0.5), minimum lines (8), maximum lines (30), and host scrollbar component (null). Limit only source area to clamp(viewport*ratio,min*lineHeight,max*lineHeight), falling back to maximum for unavailable viewport. Normalize invalid inputs. Keep full source for copy, selection and Find.
