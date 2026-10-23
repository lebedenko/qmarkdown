# Retained glyph-image diagnosis — actual verification

2026-10-23: **do not integrate the retained glyph-image candidate**. The local
coordinate formula introduces rasterization differences. Keeping original glyph
coordinates and translating by integer physical pixels eliminates that component
on both tested SDKs. Whole-run versus individual-glyph painting still differs in
the minimum environment at overlapping ink. Both previous experiments and their
[selection](verification.md) and [retained](retained-verification.md) reports remain unchanged.

## Execution and identities

Fresh, separate build directories `/tmp/qmarkdown-065-diagnostic-612` and
`/tmp/qmarkdown-065-diagnostic-680` used the staged source export
`4baf0fb934ced7b732267e1ed83bbab529cf0ef9`, with only the existing prepared-run
header patch. Harness refinements were rebuilt and the complete final matrix
rerun. [Source SHA-256 identities](evidence/diagnostic/source-identities.json)
identify the final harness, historical harness, patch and summarizer.
[Commands and selection controls](retained-prototype/README.md) are reproducible.

- Exact Qt 6.12.0: host GCC 16.2.1, QPainter raster, offscreen platform;
  resolved font **DejaVu Sans Oblique**, 53 physical-independent pixel font size.
- Exact Qt 6.8.0: existing CI container identified in the README, GCC 13.3.0,
  SDK `/opt/Qt/6.8.0/gcc_64`, QPainter raster, offscreen platform;
  resolved font **Noto Sans Italic**, pixel size 53. The requested font in both
  environments is italic DejaVu Sans, 40 point. Font resolution differs; these
  results do not isolate Qt version from font/toolchain/environment effects.
- Both configured and built successfully, math disabled in staged source, shared
  linkage. Existing QTP0004 developer warnings were nonfatal. No shipping files,
  public APIs, dependencies, package versions or installed packages changed.

Each final run completed all **258 cases**: historical fX at `(24.25,24.25)` with
DPR 1/1.25, then f/X/fX/fXfX at DPR 1/1.25/1.5/2 and independent first-glyph physical
X/Y phases 0/.25/.5/.75. Matrix origins compensate for the prepared baseline's
fractional phase. Later glyph phases remain authoritative prepared positions.
Both default-all runs returned 1, as expected for mismatches, after all controls.
The summarizer checked case completeness, target phases and occurrence/emission
agreement. Each path emitted each occurrence once in original order (1/2/4 glyphs
according to fixture). Decorations were consistently disabled.

Complete per-case/per-glyph logs, including flags, font identity, indexes,
positions, origins, phases, bounds, transforms and DPR, are retained as
[6.8 JSONL gzip](evidence/diagnostic/qt-6.8.0/results.jsonl.gz) and
[6.12 JSONL gzip](evidence/diagnostic/qt-6.12.0/results.jsonl.gz).
Decompress to run `summarize.py`. The readable summaries retain every historical
metric and aggregate totals: [6.8](evidence/diagnostic/qt-6.8.0/summary.json),
[6.12](evidence/diagnostic/qt-6.12.0/summary.json). Max channel differences are
premultiplied BGRA; this black-ink fixture changes alpha only. Lost/extra count
alpha support, not inferred geometric truncation.

## Reproduced historical failures

The unchanged historical executable was also run in separate processes per DPR.
Both DPR-1 runs passed exactly. DPR 1.25 reproduced **379** changed pixels on
6.12.0 (lost/extra 1/0, max alpha 8) and **306** on 6.8.0 (1/1, max alpha 5),
including the historical 0 versus 3 direct-control changes. Results are retained
as `historical-dpr-1.txt` and `historical-dpr-1.25.txt` in each SDK evidence directory.

The new multi-control process's historical-origin case measured 380 versus 306
local changes, respectively. The extra 6.12 pixel is an unresolved control-order
or raster-cache-state sensitivity; it is not hidden by a tolerance. Its support
counts are 1 lost/2 extra, max alpha 8. It does not change the exact rejection.
The historical separate-process executable remains the original reproducer.

## Final matrix comparisons

Counts below include the two historical-origin cases. “Maximum” is changed pixels
in one case; “total” sums changed pixels across cases, including repeated fixtures.

| Comparison | 6.8 failed cases / total / maximum | 6.12 failed cases / total / maximum |
| --- | --- | --- |
| Reference → reconstructed direct | 97 / 295 / 6 | 0 / 0 / 0 |
| Reference → metadata-preserved direct | 97 / 295 / 6 | 0 / 0 / 0 |
| Reconstructed → preserved direct | 0 / 0 / 0 | 0 / 0 / 0 |
| Preserved direct → local tile | 122 / 45193 / 1144 | 120 / 40349 / 1058 |
| Preserved direct → translated tile | 0 / 0 / 0 | 0 / 0 / 0 |
| Preserved direct → full-canvas crop | 0 / 0 / 0 | 0 / 0 / 0 |
| Reference → local tile | 154 / 45333 / 1143 | 120 / 40349 / 1058 |
| Reference → translated tile | 97 / 295 / 6 | 0 / 0 / 0 |
| Reference → full-canvas crop | 97 / 295 / 6 | 0 / 0 / 0 |
| Local physical copy → legacy logical copy | 0 / 0 / 0 | 0 / 0 / 0 |

Reference → legacy equals reference → local in every case. In 6.8, direct/local
lost/extra totals are 341/163, maximum alpha delta 8; reference/local has the same
support totals, maximum delta 51. In 6.12, direct/local and reference/local have
lost/extra 212/211, maximum delta 11. All direct/reference, translated and crop
comparisons have zero lost/extra support on both SDKs; maximum alpha delta is 51
on the 6.8 reference/direct differences and zero on 6.12.

Border ink is zero throughout the 6.12 matrix. In 6.8, each tile method records
76 border-ink pixels summed over occurrences/cases; the two-physical-pixel margin
therefore does not guarantee a transparent border. Nevertheless, crop and
translated tiles match their full-canvas direct control exactly, with no measured
support loss. This is evidence against truncation in this matrix, not a general
bounds guarantee. Historical cases have zero border ink.

## Diagnosis and recommendation

- **Metadata:** copying the prepared run does not fix any direct differences;
  reconstruction and copies match exactly. Logged run flags are zero for these
  plain fixtures. Nonzero flags and shaping breadth remain unqualified.
- **Coordinate/rasterization:** local tiles differ from direct painting, while
  unchanged coordinates with integer physical translation match direct exactly
  in every case on both SDKs. This supports retaining original coordinates for
  any later experiment. No claim about Qt's internal rounding algorithm follows.
- **Composition:** integer crop/copy and the two compositor coordinate systems
  introduce no additional differences. All 295 remaining reference/direct changed
  pixels in 6.8 lie in overlapping glyph support; none lie outside it. Single f/X
  controls match. Whole-run batch emission versus individual SourceOver emission
  is the remaining overlap-composition boundary. The exact internal batching or
  coverage calculation is unresolved; a run copy cannot restore whole-run painting.
- **Unresolved:** the minimum environment's overlap behavior, font-versus-Qt
  attribution, nontransparent tile borders, and the one-pixel multi-control/history
  discrepancy require any subsequent design to retain exact comparison.

Representative reference/candidate/difference captures and enlarged contact sheets
are retained for both environments under `evidence/diagnostic/qt-*`.
The local-tile sheets on both SDKs and 6.8 metadata-preserved direct sheet were
visually inspected: both glyphs remain present; local differences trace antialiased
edges, and the direct difference is concentrated near their overlap. Inspection
supports localization only; exact pixel comparison determines acceptance.

Only the 6.12 environment's translated/crop variant achieves preparation-image
parity for this matrix. No corrected variant passes both environments. Recommend
stopping before scene-graph integration and investigating overlap batching in a
later approved plan. GPU rendering, scrolling/filtering, shaping coverage,
lifecycle, resources, performance and production readiness remain untested.

## Delivery checks

Qt target synchronization reported 6.12.0 with no configuration change.
`python3 scripts/check-docs.py` and `git diff --check` passed. Summarizer assertions
passed on both final logs. The existing CI classification remains full because
this PR includes executable/CMake/evidence inputs; it was not weakened.
Final-head hosted results are recorded in [PR 48](https://github.com/lebedenko/qmarkdown/pull/48)
after pushing this report. Earlier-head checks do not qualify the new head.
The PR remains open, unmerged, with auto-merge disabled.
