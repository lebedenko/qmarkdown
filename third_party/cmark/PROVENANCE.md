# Bundled cmark

Version: 0.31.2. Acquisition URL: https://github.com/commonmark/cmark/archive/refs/tags/0.31.2.tar.gz

Archive supplied by the user from ~/Downloads/cmark-0.31.2.tar.gz.
SHA-256: `f9bc5ca38bcb0b727f0056100fac4d743e768872e3bacec7746de28f5700d697`.

Library sources and generated upstream lookup tables are copied unchanged from src/. COPYING retains all upstream notices. Locally authored cmark_export.h/cmark_version.h replace upstream configured headers for this private static embedding. symbols.h prefixes every externally defined library symbol (derived by compiling the pinned C99 library and running nm -g --defined-only). All library objects have hidden visibility and PIC. No public header or target is installed. No dependency downloads occur at configure/build time. The adapter uses only cmark.h's public document/node API, never HTML rendering or internal parser APIs.
