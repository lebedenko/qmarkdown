# Bundled cmark

Version: 0.31.2. Acquisition URL: https://github.com/commonmark/cmark/archive/refs/tags/0.31.2.tar.gz

Archive supplied by the user from ~/Downloads/cmark-0.31.2.tar.gz.
SHA-256: `f9bc5ca38bcb0b727f0056100fac4d743e768872e3bacec7746de28f5700d697`.

Library sources and generated upstream lookup tables originate from upstream src/. The local patches below describe changes to those upstream sources. COPYING retains all upstream notices. Locally authored cmark_export.h/cmark_version.h replace upstream configured headers for this private static embedding. symbols.h prefixes every externally defined library symbol (derived by compiling the pinned C99 library and running nm -g --defined-only). All library objects have hidden visibility and PIC. No public header or target is installed. No dependency downloads occur at configure/build time. The adapter uses only cmark.h's public document/node API, never HTML rendering or internal parser APIs.

## Local long-fence correction (iteration 021)

- src/node.h: change private cmark_code.fence_length from uint8_t to bufsize_t.
- src/blocks.c: store the complete scanned opener length rather than clamping it to 255.

CommonMark 0.31.2 requires a closing fence of the same character and at least the opener length. The upstream clamp incorrectly lets a shorter fence close an opener longer than 255 characters. The existing comparison now uses the full length. Supplementary parser/probe regressions cover both markers, boundary/long lengths, line endings and containers. Upstream version, acquisition checksum, COPYING, symbol prefixing and official corpus/ledger remain unchanged. This private patch adds no installed API.

## Local opt-in task-list recognition (iteration 035)

- src/cmark.h: add a private default-off task-list option and task-state getter.
- src/node.h and src/node.c: retain ordinary/unchecked/checked item state and expose it through the getter.
- src/blocks.c: recognize task markers in the raw first paragraph of a list item before inline parsing; strip the prefix only with the option enabled.
- symbols.h: prefix the added getter; hidden visibility and no installed headers remain unchanged.

The recognition routine is locally authored from [GFM 0.29-gfm section 5.3](https://github.github.com/gfm/#task-list-items-extension-), with the CommonMark 0.31.2 block parser retained. Default-off strict corpus checks and separate extension fixtures test both paths. This is selected task-list syntax, not full GFM conformance. No cmark-gfm code is copied or new library added. The upstream version, archive checksum and notices are unchanged.

## Local opt-in strikethrough (iteration 036)

`src/cmark.h`, `src/inlines.c` and `src/node.c` add a private default-off option and inline container, raw tilde recognition, equal-length one/two delimiter pairing with existing flanking rules, and reuse existing emphasis splicing. The routine is locally authored from [GFM §6.5](https://github.github.com/gfm/#strikethrough-extension-); no cmark-gfm implementation is copied. Separate extension tests and unchanged strict CommonMark checks cover both paths. No new external symbols, installed headers or dependencies are added; upstream version/checksum/notices remain unchanged.

## Local opt-in extended-autolink recognition (iteration 037)

Locally authored routines in src/inlines.c implement selected [GFM 0.29-gfm section 6.9](https://github.github.com/gfm/#autolinks-extension-) rules. A private default-off option recognizes web candidates from source during inline parsing; explicit links restore normal label parsing when a candidate was consumed. After text consolidation in src/blocks.c, a decoded-text pass recognizes email/mailto/xmpp outside link subtrees, including semantic image descriptions. Ordinary link nodes reuse the existing adapter and presentation. src/node.h marks temporary generated web nodes privately; src/inlines.h and symbols.h declare/prefix the added private pass. No cmark-gfm code, public headers or dependencies are added. Upstream version, archive checksum, COPYING and CommonMark fixtures remain unchanged.
