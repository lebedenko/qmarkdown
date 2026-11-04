# Actual verification

## Reproduction (baseline)

QMarkdown 1.18.0 sources with the new `tests/tst_lines.cpp`, host Qt 6.12.0,
`QT_QPA_PLATFORM=offscreen`, `QT_QUICK_BACKEND=software`. At `QT_SCALE_FACTOR` 1.25
and 1.5 the thickness cases failed on the unmodified `Rectangle` lines (for example
"table line 5 is 2 px, expected 1" at 1.25, contentY 0.25), confirming the report.
Scale factor 1 and 2 were not recorded separately on the baseline.

## Initial implementation checks

`PixelLineItem` replaces the line `Rectangle`s in `TableBlock.qml`,
`HeadingSeparator.qml` and the thematic-break delegate in `BlockSequence.qml`.
`qmarkdown-lines` (34 cases: contentY 0, 0.25, 0.5, 0.75, 1, 37.3, 101.6 × `pixelAligned`
on/off × integer/fractional origin, a 0.37-step scroll sweep, non-default thicknesses 2/3/1,
disabled lines, a 0.2 logical px line, and unchanged layout geometry) passes at scale
factors 1, 1.25, 1.5 and 2. Three further repeats at 1.5 also passed. The full local
CTest suite (33 tests, shared build, math/highlighting as configured) passed.

Measurements are exact: one column of pure-red runs through heading separator,
thematic break and every table border, and one row through the vertical borders;
any partially covered pixel fails the case.

## Findings during implementation

A first version rounded `1 * 0.99999994 * 1.5` down to one device pixel on some
scroll offsets; thickness rounding now carries a 1e-4 pixel tolerance. An earlier
0.2 logical px run showed black rows adjacent to lines at 1.5 on the software backend
(also visible around the unmodified table's outer border); it did not recur after
that fix and is not explained. This initial result was superseded by the review fixes below.

## Not run

- Native GPU and compositor review at real fractional scale (owner-run, pending).
- Continuous-scroll cost measurement (R6) with benchmark tooling.
- Installed-consumer and syntax-enabled configurations, minimum Qt 6.8.0 target,
  and hosted CI.
- Selection and navigation geometry suites were covered only through the full local
  CTest run above; no dedicated assertions were added beyond line-item layout checks.

## Review fixes (2026-11-04)

The owner's request to fix the review findings authorizes these routine corrections
within R1–R5:

- Normalize disabled/transparent painted rectangles to empty during both tracking
  and painting, so ordinary grids and disabled decorations settle without idle frames.
- Coordinate vertical starts and horizontal ends with their perpendicular border's
  snapped edge. Paint all table borders above all cell backgrounds; otherwise a
  following row can cover an edge crossing a logical row boundary.
- Include both new private source files in the release archive allowlist.
- The review reproduced the subpixel software failure at DPR 1.5. A unit-rectangle
  transform did not reliably fix it. The final software-only bounded render node
  paints through Qt's public painter resource without claiming opaque logical-pixel
  coverage. GPU backends retain the ordinary rectangle node.

Regression coverage adds idle-frame checks for zero thickness, transparency and an
ordinary grid, plus full-height scans through every column border at fractional
scroll offsets with translucent borders of thickness 0.2, 1 and 2. These scans detect
both gaps and alpha buildup at row intersections and outer corners.

Actual local results after review fixes, host Qt 6.12.0:

- Shared build, math ON, syntax highlighting OFF: built successfully.
- `ctest --test-dir build-shared -R '^qmarkdown-lines-' --output-on-failure`:
  all four scales pass, 40 QtTest results per scale including setup/cleanup.
- Remaining runtime suite (`-E '^qmarkdown-lines-'`): all 29 tests pass, including
  styling, fractional styling, selection, fractional selection and keyboard checks.
- `all_qmllint`, `scripts/verify-cmark-symbols.py build-shared`,
  `scripts/check.py` and `git diff --check`: pass.
- Archive generated from the working tree using `build_archive(..., commit=None)`:
  both `pixellineitem` source files are present; the extracted library configures
  and builds successfully as shared, math OFF. This is local archive coverage,
  not publication or a committed-release verification claim.

Native GPU/compositor review, minimum Qt, static/syntax-enabled builds,
installed consumers and continuous-scroll performance measurements remain pending.
