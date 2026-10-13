# Requirements

Approved on 2026-10-13 by the user response "approved". Approval covers requirements, design, mapped tasks, local implementation and verification. Commit, PR creation, merge and publication remain separately authorized.

R1: Add writable, independent, default-false `MarkdownView.extendedAutolinksEnabled`. Equal assignments are inert. Both toggle directions reparse original Markdown through the existing resource cancellation/rebuild lifecycle, emit only the extension property notification, and preserve source and `markdownChanged` behavior.

R2: Implement complete selected extended-autolink rules from [GFM 0.29-gfm section 6.9](https://github.github.com/gfm/#autolinks-extension-): lowercase HTTP(S), `www.`, bare email, `mailto:` and `xmpp:` including resources. Follow its ASCII grammar, domain validity, boundaries, punctuation trimming, parenthesis balancing, entity tails and email/protocol suffix rules. Preserve CommonMark 0.31.2 core behavior and unchanged behavior when disabled.

R3: Preserve explicit/reference-link precedence and prevent nested links. Leave code and HTML nodes untouched. Retain semantic links inside image descriptions while existing image projection keeps them inert. Reuse owned link nodes, UTF-16 spans, native styling/hit testing and host-owned `linkActivated(destination)`.

R4: Advance the unpublished source candidate to package 1.5.0 / QML 1.5. Preserve historical imports and compatible historical 1.x package requests; accept EXACT 1.5.0 and reject earlier exact and future versions. Add installed consumers in shared linked, plugin-only and static configurations, playground control/sample, extension documentation and accurate current status/provenance.

R5: Pin all 14 official examples 622–635 with offline fixtures and independently authored text, semantic link/destination and UTF-16-span expectations. Record attribution, CC BY-SA license, acquisition date and checksum. Require focused syntax/state/native checks, eight extension combinations, unchanged strict 652-example CommonMark parser/model gate, shared/static CTest, QML lint, symbol privacy and configured Qt 6.8.0/6.12.0 container packaging gates.

No new dependencies, renderer, navigation, network requests or public style properties. Tables, combined GFM mode, HTML filtering, streaming and full GFM conformance remain deferred. Commit, PR creation, merge and publication require separate authorization. Exact-commit Arch lifecycle and publication checks belong to later release delivery.
