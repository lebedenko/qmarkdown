# 007: Design

**Approval:** On 2026-10-08 the user explicitly requested implementation of the supplied Feature 007 plan. Approval covers these requirements, design, tasks, and package 0.5.0 / module 0.5.

Use cmark public nodes only and free the parse tree after adaptation. Containers own value children; BlockModel owns a matching QObject child-model tree disposed on replacement. Recursive private QML sequences instantiate through Loaders to avoid eager component recursion. Leaf rendering remains native Text/FormattedText; HTML blocks use body-styled plain multiline Text. Newlines in QTextLayout use line separators with unchanged UTF-16 offsets.

Lists measure every marker with bodyFont; gutter=max(listIndent, widest+8), clamped to width-1. Markers align at y=0; rows reserve at least one body line. Tight item/interior gaps are zero, loose gaps use blockSpacing. Quotes inset=max(quoteIndent, thickness+8), clamped to width-1; rule spans content height and is clamped to available inset. Empty quotes reserve one body line. Nonpositive root widths suppress delegates. Host owns scrolling and identity; sizing settles through polish.

Intentional migration: resource source spellings become labels, reference definitions disappear, hard breaks become newlines, HTML blocks remain literal without inline decoding, and cmark supersedes handwritten precedence/whitespace fallbacks. No dependency or bundled-source change.

The container semantics target is [CommonMark 0.31.2](https://spec.commonmark.org/0.31.2/#container-blocks). Bundled cmark stores opening fence lengths capped at 255; this migration deliberately follows that upstream behavior for the retained >255-marker regression. No bundled-source workaround is introduced. The existing FormattedText newline-to-line-separator conversion already preserves UTF-16 offsets and is reused unchanged.
