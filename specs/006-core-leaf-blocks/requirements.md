# 006: Core leaf blocks

**Approval:** The user explicitly instructed implementation of the supplied Feature 006 plan on 2026-10-08. Approval covers R1–R5, the design below, tasks T1–T6, and delivery as package 0.4.0 / QML module 0.4.

Target [CommonMark 0.31.2](https://spec.commonmark.org/0.31.2/) with bounded top-level syntax, preserving the private parser/model and native Qt Quick rendering.

- **R1:** Nonempty pending paragraph plus a contiguous `=` or `-` underline, indented at most three columns and followed only by spaces/tabs, becomes H1/H2. Join multiline content with existing paragraph normalization; retain inline formatting. Setext takes precedence over thematic breaks.
- **R2:** At least three identical `*`, `-`, or `_` markers with optional spaces/tabs and at most three leading columns form a thematic break, including paragraph interruption. Add private ThematicBreak and full-width native Rectangle.
- **R3:** Nonblank input indented at least four columns starts code only without a pending paragraph. Remove four columns with tab-aware deindentation; retain literal content, residual indentation, trailing whitespace and internal blank lines. Exclude leading/trailing blanks. Reprocess terminating nonblank input below four columns normally.
- **R4:** Reuse code block style/delegate. Add notifying thematicBreakColor (#202020) and thematicBreakThickness (1). Render finite thickness as max(0,value), nonfinite as 1. Preserve spacing, width suppression/recovery and style lifecycle.
- **R5:** Preserve fences, runs beyond 255 markers, private info, inline handling, replacement, and zero resource requests/activation. No new dependencies or cmark changes.

Nested lists/quotes are deferred to Feature 007. Links/images, resource policies and full conformance remain future work. Unsupported containers retain bounded fallback; full container precedence is outside scope.

**Status — 2026-10-08:** Implemented and verified for the bounded approved scope; see [actual results](verification.md).
