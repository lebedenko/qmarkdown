# Investigation — initial source review

Date: 2026-10-13. Static inspection only; no prototype or performance result is claimed.

## Existing implementation

`src/QMarkdown/private/formattedtext.cpp` uses cached QTextLayout, consolidated format intervals, WrapAtWordBoundaryOrAnywhere and native line heights. Decoded LF becomes a same-length line separator. Code backgrounds are unions of valid visual cursor cells, sorted across bidi runs. No layout padding exists. Link hit testing scans the same cells and requires layout/source lengths to agree. Painting permits ink overhang through m_paintOffset while logical width and height drive parent layout.

`document.h/.cpp` owns recursive blocks and inline semantics; parse currently takes three extension booleans. ViewState extension setters repeat parsing and call the established resource reload path. MarkdownView.qml recursively renders list/quote children and formatted/image content. MarkdownStyle groups use central values, explicit overrides, resets and precise notifications; effective metrics are normalized in presentation. Project/module versions are currently 1.5.0/1.5. Initial git status was clean.

## Candidate approaches (provisional findings)

| Approach | Feasibility and limits | Complexity/performance to measure |
| --- | --- | --- |
| Expand painted rectangles only | Cannot reserve width or line height; fails required spacing and transparent-background behavior | Cheap but rejected by requirements |
| Apply font letter spacing to code | Changes internal glyph advances and does not directly represent padding at each wrapped fragment boundary | Small implementation; unsuitable as sole strategy |
| Layout-only spacer positions plus explicit mapping | Qt can shape an augmented layout while semantic text stays unchanged; fragment boundaries after wrapping and bidi effects require a prototype | Extra mappings and potentially repeated layout; measure convergence and break stability |
| Place visual fragments from Qt-shaped runs | Allows explicit padded geometry, but preserving paragraph bidi, ligatures, graphemes and breaks is difficult | Highest complexity; measure layout/preparation cost and ensure whole-paragraph shaping is preserved |

These are engineering hypotheses from code/API review, not measured feasibility findings. QTextLayout exposes line creation, cursor validity/geometry and glyph runs, but no general inline-object sizing hook comparable to a QTextDocument object handler. Do not adopt QTextDocument to obtain that hook.

## Prototype acceptance and evidence

After approval, build a disposable bounded prototype and record reproducible commands, Qt version, inputs, geometry and elapsed/preparation costs. Use the existing renderer as the zero-padding reference. Check LTR adjacent text/code, multiple adjacent code spans, code wrapping over several lines, mixed RTL/LTR code and linked code, combining sequences, surrogate pairs/emoji, whitespace boundaries and extremely narrow widths.

For each visual fragment assert horizontal space on both sides, nonoverlapping neighbor bounds, vertical padding within row advance, and consistent background geometry even when transparent. Test hits inside glyphs and both padding edges against the enclosing link; blank tails and unrelated neighbors remain inactive. Preserve original semantic text and UTF-16 ranges. Compare font/width/DPI changes and zero padding with the baseline. Record finite layout work and scaling for long paragraphs and many code spans; choose a performance budget informed by measured baseline before approving production design.

Gate remains open. A successful prototype is required before the detailed production renderer design can be approved.

## Sources consulted

- [GFM 0.29-gfm specification](https://github.github.com/gfm/): table examples 198–205 and tag-filter example 657; acquisition/pinning and fixture attribution remain tasks.
- [Qt QTextLayout API](https://doc.qt.io/qt-6/qtextlayout.html): shaping, line, cursor and glyph-run interfaces. Runtime behavior must be checked on configured Qt versions.

## Actual prototype results — 2026-10-13

The bounded visual-run prototype is retained at [prototype/padding.cpp](prototype/padding.cpp), outside production build targets. It uses full-paragraph QTextLayout shaping, monotone padding reservation before wrapping, translated glyph runs carrying string indexes, and source-indexed visual cells/fragment hit regions. No synthetic characters, parser changes or document renderer are involved.

Reproduction on host Qt 6.12.0:

```sh
c++ -std=c++17 -fPIC -O2 specs/038-gfm-completion-inline-code-padding/prototype/padding.cpp -o /tmp/qmarkdown-padding-prototype $(pkg-config --cflags --libs Qt6Gui)
QT_QPA_PLATFORM=offscreen /tmp/qmarkdown-padding-prototype /tmp/qmarkdown-padding-prototype.png
```

Passed 144 cases: six LTR/adjacent/wrapped/mixed-direction/combining/emoji strings, three pixel font sizes (12/16/28), four widths (1/32/80/220), and zero/default padding. Checks assert immutable text, source mapping, no overlap between different visual identities, both horizontal edges and vertical padding, padding link identities, line advance, and zero-padding line-count/height compatibility. Ligature cursor cells can overlap within one visual identity; union geometry preserves their shared glyph. An initial overly strict cell-overlap assertion exposed this expected Qt behavior and was corrected to check distinct identities. An initial compile required `-fPIC` for the installed Qt build. The generated mixed-direction preview was visually inspected and showed reserved padding and intact visual ordering.

[Recorded timings](prototype/results-qt6.12.json) average five preparations: 150 characters 0.124/0.165 ms, 1,500 characters 1.349/1.899 ms, and 15,000 characters 26.658/38.680 ms for zero/default padding respectively. Code-bearing lines used two passes; zero-padding lines used one. Source-map lookup replaced avoidable scans over all spans. Remaining scaling includes Qt line/cursor/glyph retrieval and is not claimed linear. These are advisory host measurements, not benchmark release gates.

Visual-run translation is feasible for the required wrapping/bidi/hit-testing prototype gates. Layout-only spacers would require additional source mapping and bidi controls plus wrap-boundary reinsertion, without an established convergence result; do not select that approach. The proposed detailed design in design.md covers production color/decorations, semantic code identities, alignment, caching and edge hit ownership beyond this bounded prototype.

Remaining verification: production font/decorations, screen/DPI invalidation, pointer interaction, and configured Qt 6.8.0 runtime coverage. No production padding integration is authorized until A2 approval. Prototype results do not claim full renderer or release verification.

The same 144 cases also passed with `QT_SCALE_FACTOR=1.25`; [fractional-scale measurements](prototype/results-qt6.12-fractional.json) are retained. This checks the prototype under fractional application scaling, not production screen-change handling.

## Production follow-through — 2026-10-13

After explicit A2 approval, the visual-run algorithm was integrated into FormattedText with separate semantic code-span identities, complete source/link maps, native format/fallback/decoration preservation, padded alignment and raster bounds, and half-open hit regions. Production tests cover wrapped linked fragments, partial links within one code fragment, adjacent spans, whitespace-only/trailing-space spans, Hebrew/Latin text, combining marks and emoji, one-pixel widths, font changes, transparent padding, repaint stability, defaults/reset and pointer/drag interaction. Fractional-scale styling runs exercise the same production tests at 1.25 scale.

Production testing exposed a Qt edge not exercised by the bounded prototype's fonts/formats: cursor cells clamp to the assigned width when an indivisible glyph overflows. The renderer uses QTextLine's natural text width (the minimum preserving its chosen break position) to measure that overflowing line, keeping Qt's grapheme/word decisions. The original zero-padding geometry/draw path remains unchanged. See [Qt QTextLine](https://doc.qt.io/qt-6/qtextline.html#naturalTextWidth).

The initial production 15,000-character/1,000-code-span test measured 17.2544 ms with zero padding and 34.4428 ms with 4/2 padding over five preparations on host Qt 6.12.0. A later Qt 6.8 run measured approximately 2.18×, above the advisory budget. Investigation compared glyph retrieval strategies and glyph-ink caching; additional ink caching did not improve preparation and was removed. The final renderer retrieves each shaped line once, partitions glyphs by resolved color while retaining Qt decoration/fallback flags, and reuses cursor cells when only moving the baseline. Final focused host measurements were 18.1457/32.0357 ms (1.77×), within the advisory 2× budget. This is a bounded host observation, not a portable timing guarantee; the test records measurements without a machine-dependent timing assertion. Qt shaping, glyph extraction and per-line visual sorting still govern costs. Background/radius repaint tests retain geometry and hits; the existing window/screen/DPI invalidation path remains in use. Static fractional-scale coverage does not claim a physical screen-switch experiment.

The configured Qt 6.8 API test exposed a separate anonymous grouped-style revision issue: per-property `REVISION(1, 6)` prevented QML 1.6 assignments, while Qt 6.12 allowed them. An explicit extra type version did not fix the restriction. The final metrics follow the existing unrevisioned anonymous grouped-field convention, preserving historical group access and allowing QML 1.6 assignments on both supported versions. The successful focused minimum-version test is retained in the local revision log; final complete configured checks supersede these investigation attempts.
