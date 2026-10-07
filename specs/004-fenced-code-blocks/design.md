# 004: native fenced code block design

**Status:** Implemented and verified for the bounded Feature 004 slice. Approved by the user on 2026-10-07. Approval covers requirements R1–R6, design, and tasks T1–T7 for this bounded slice; implementation and verification are authorized.

## Parser and private model

Keep private/document.cpp's document splitter, normalization, paragraph joining and heading extraction. Add BlockKind::CodeBlock and a QString infoString to private Block; code blocks bypass inline adaptation and have empty ranges. Keep marker-run lengths in qsizetype so the splitter never inherits cmark's 255-marker cap.

On a valid opener, flush the paragraph and capture marker, full run length, opening indentation and trimmed info. While inside a fence, test each physical line for a valid closer before extracting content. Closing indentation is independent of opening indentation. Append deindented content plus LF for every actual content line. Exclude only the trailing empty split element caused by a final LF. Emit a code block on closing or at EOF, even when empty.

Deindent content using a small column-aware helper: consume leading spaces/tabs up to the opener's indentation, stop at non-whitespace, and replace only the consumed tab's unremoved columns with spaces. Leave subsequent tabs unchanged. Fence indentation is limited to three columns, so a tab reaching column four cannot introduce an opener/closer.

Decode info with a small private helper in document.cpp, using public cmark_parse_document and cmark_node_get_fence_info. Construct a synthetic tilde-fenced block with the original trimmed info after an explicit separating space, empty content, and a closing fence. Validate a document containing exactly one code-block child, no unexpected descendants/siblings and an available info accessor; free the root deterministically. Fall back to trimmed source info on any failure. Keep metadata out of public API and model roles. No cmark internal API or new dependency is needed.

## Style and rendering

Add codeBlockFont/codeBlockColor using existing MarkdownStyle setters, resolve-mask-aware font equality, signals and restoreDefaults. Construct the default code font independently from QGuiApplication::font and the system fixed-family. Existing ViewState ownership/reset/destruction machinery applies.

Add a private code-block discriminator role. Extend delegate selection to distinguish code, formatted inline blocks, and plain paragraph/heading blocks without changing their behavior. Use a native Text delegate for code with style font/color, PlainText, Wrap, left alignment, no eliding, padding or background. Its display text removes one terminal LF; its semantic block text remains untouched. Use FontMetrics to reserve one line when display text is empty, including a single empty content line. Longer sequences of blank lines retain native line geometry. Keep Column spacing and width-dependent model suppression; live layout assertions wait for completed polish.

Retaining the splitter avoids broader block-parsing changes; native Text avoids another layout implementation. Info decoding reuses the bundled dependency while full fence recognition remains independent of its marker cap.

## Delivery

Update CMake package/module versions to 0.3.0/0.3, QML imports, private runtime registration and tooling metadata, viewer and relocated installed-consumer fixtures. Update current README/spec overview/roadmap claims after verification; preserve historical feature records. No selection, resource loading, custom renderers or styling beyond font/color is introduced.

The [task mapping](tasks.md) and [planned checks](verification.md) define the approval scope. The user explicitly approved these documents on 2026-10-07; the recorded scope is R1–R6 and T1–T7.
