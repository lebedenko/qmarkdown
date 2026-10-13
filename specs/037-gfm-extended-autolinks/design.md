# Design

Approved on 2026-10-13 by the user response "approved"; see [requirements](requirements.md) and [mapped tasks](tasks.md).

Extend bundled cmark with a private default-off option and locally authored recognition routines. Recognize HTTP(S)/`www.` candidates during inline parsing from source text; recognize email/`mailto:`/`xmpp:` candidates in consolidated decoded text nodes after inline parsing. Both paths produce ordinary link nodes. Use lowercase prefix matching and explicit ASCII character predicates, following the pinned specification rather than current GitHub implementation behavior.

Integrate web recognition with existing bracket handling so successful explicit/reference links keep precedence. Within pending labels, yield at bracket, code, escape and image-opening syntax so normal bracket resolution remains intact. If a successful explicit link consumed a generated web candidate, reparse its original label with web recognition disabled to restore ordinary inline escapes/code/emphasis. Traverse eligible text for the post-inline pass, skipping link subtrees and code/HTML nodes while allowing image descriptions. Keep recognition within inline text boundaries and preserve surrounding literal text when splitting nodes. Validate domains and address suffixes before committing a candidate, then apply the pinned punctuation, balancing and entity-tail rules. No source preprocessing or regular-expression substitution across Markdown constructs.

The document adapter already owns link nodes and derives displayed UTF-16 offsets. Reuse that path without adding semantic node types. Existing image projection retains semantics while removing active presentation. Existing native link formatting, wrapping, pointer interaction and activation destinations apply to generated links.

Add the ViewState property, notification, private QML metadata and public MarkdownView alias. Extend `QMarkdownPrivate::parse` with a default-false fourth argument. All source parsing paths pass all three independent switches. Mirror existing extension setters: no-op on equality; reparse stored source; rebuild resources/model; emit the extension notification without `markdownChanged`.

Update package/module versions and every compatibility matrix together. Add a QML 1.5 installed consumer asserting default-off, enabled destinations and toggle behavior in shared linked/plugin-only/static modes. Extend the playground with an independent toggle and representative examples. Update README, changelog, extension guides, parser provenance and current specification summaries, preserving historical evidence.

Trade-off: a bounded private cmark patch preserves established syntax precedence and avoids a parser replacement or new dependency. Separate source-web and decoded-text passes follow the requested recognition semantics; focused escape/entity and link-precedence tests will guard their interaction.

## Authorized 1.5.0 release delivery

The user requested implementation of the release plan on 2026-10-13, authorizing preparation, protected PR/squash merge, exact-main release gates, annotated tagging, stable/latest publication and downloaded-package acceptance. See [delivery scope and gates](requirements.md#authorized-150-release-delivery). Earlier verification and authorization records remain historical.
