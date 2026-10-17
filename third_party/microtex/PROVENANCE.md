# MicroTeX provenance

Upstream: https://github.com/NanoMichael/MicroTeX
Commit: 0e3707f6dafebb121d98b53c64364d16fefe481d
License: MIT (LICENSE). Bundled font notices: res/fonts/licences.
Only res/fonts is embedded or shipped in the source distribution; unused Greek
and GPL-licensed Cyrillic alphabet payloads remain upstream audit material only.
The complete upstream src/res payload is retained for auditability; demos are not built.

Private modifications: fixed embedded resource root and Qt-only XML reads; preloaded
GUI-thread font registry with no lazy loading; operation/allocation/depth guards in
parser, atom layout and box loops. Repository CMake wrapper replaces upstream policy.
Input commands are restricted by the library before engine execution. This snapshot
is private and does not establish a public C++ API.

Additional Qt adapter changes: record glyph outlines at normalized 1000 px rather
than tiny-font drawText operations (avoids QPicture replay glyph loss and keeps
font/DPI-independent logical geometry); bundled roman text font; strict parsing
including missing required arguments; private namespace renaming for static linking.

Depth guards also cover recursive command/argument parsing and inline atom layout
methods, including nested unbraced commands. The library caps painted logical area
at 4 megapixels before creating an item texture, in addition to per-axis limits.
