# Bundled cmark

Version: 0.31.2. Acquisition URL: https://github.com/commonmark/cmark/archive/refs/tags/0.31.2.tar.gz

Archive supplied by the user from ~/Downloads/cmark-0.31.2.tar.gz.
SHA-256: `f9bc5ca38bcb0b727f0056100fac4d743e768872e3bacec7746de28f5700d697`.

Library sources and generated upstream lookup tables originate from upstream src/. The two-file local patch below is the only change to those upstream sources. COPYING retains all upstream notices. Locally authored cmark_export.h/cmark_version.h replace upstream configured headers for this private static embedding. symbols.h prefixes every externally defined library symbol (derived by compiling the pinned C99 library and running nm -g --defined-only). All library objects have hidden visibility and PIC. No public header or target is installed. No dependency downloads occur at configure/build time. The adapter uses only cmark.h's public document/node API, never HTML rendering or internal parser APIs.

## Local long-fence correction (iteration 021)

- src/node.h: change private cmark_code.fence_length from uint8_t to bufsize_t.
- src/blocks.c: store the complete scanned opener length rather than clamping it to 255.

CommonMark 0.31.2 requires a closing fence of the same character and at least the opener length. The upstream clamp incorrectly lets a shorter fence close an opener longer than 255 characters. The existing comparison now uses the full length. Supplementary parser/probe regressions cover both markers, boundary/long lengths, line endings and containers. Upstream version, acquisition checksum, COPYING, symbol prefixing and official corpus/ledger remain unchanged. This private patch adds no installed API.
