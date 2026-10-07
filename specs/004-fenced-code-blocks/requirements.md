# 004: native fenced code block requirements

**Status:** Implemented and verified for the bounded Feature 004 slice. Approved by the user on 2026-10-07. Approval covers requirements R1–R6, design, and tasks T1–T7 for this bounded slice; implementation and verification are authorized.

## Scope

Extend version 0.2.0 with native top-level fenced code blocks; deliver package 0.3.0 and QML module 0.3. Paragraphs and ATX headings retain existing behavior. This deliberately supersedes Feature 001's protected-fence literal fallback and Feature 003's retained fence behavior. Historical verification records remain unchanged.

| ID | Requirement and acceptance criterion |
| --- | --- |
| R1 | Recognize backtick and tilde fences under bounded top-level CommonMark 0.31.2 rules: at least three identical markers, at most three indentation columns, same-marker closer at least as long as opener, and only spaces/tabs after closer. Backtick opener info cannot contain backticks. Unclosed fences consume to document end. Full-length matching must work beyond 255 markers; invalid openers retain existing paragraph fallback. |
| R2 | Hide delimiter lines and preserve literal content without inline ranges. Remove up to opening indentation from each content line with four-column tab stops, retaining residual spaces from partially consumed tabs. Preserve other spaces, tabs, blank lines, Unicode, and Markdown-looking text. Existing CRLF/CR and NUL normalization applies. Semantic content stores one LF per physical content line, including an unterminated final line; a trailing split sentinel is not a content line. |
| R3 | Retain the full decoded info string privately, using bundled cmark public parser/accessors and validated synthetic tilde-fence adaptation; on failure preserve trimmed source info. Do not display/expose language metadata or execute it. Resource-looking code causes no requests, navigation, or activation. |
| R4 | Add notifying MarkdownStyle.codeBlockFont and codeBlockColor. The independent default font starts from the application font, uses system fixed-font family, 16 pixels and normal weight; default color is #202020. Whole-font assignment, QML subproperty notifications, shared styles, replacement/destruction/reset follow existing conventions. Body and inline-code styles remain independent. |
| R5 | Render with native Text.PlainText, Text.Wrap and left alignment. Remove exactly one terminal LF only for display. Empty blocks occupy one font line; intentional blank content lines remain visible. Preserve block gaps, nonpositive-width suppression/recovery, content height, resizing and polish-based layout updates. No background or padding; hosts own scrolling/clipping. |
| R6 | Update editable viewer and installed consumers with both markers, whitespace, long lines and independent code styling. Update package/module versions, imports, private registration metadata and current documentation consistently. Add no dependencies. Run focused tests then shared/static CTest, QML lint, symbol and relocated packaging checks, and inspect viewer capture. Record actual outcomes separately from planned checks. |

Lists, block quotes, indented code, highlighting, copy actions, custom fence renderers and full CommonMark conformance are outside scope. Full conformance remains unverified.

See [design](design.md), [tasks](tasks.md), and [verification](verification.md).
