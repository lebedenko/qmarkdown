# Scale diagnosis and performance qualification

This measurement-only continuation implements Feature 073 P1–P6. It does not
optimize the boundary patch, select a renderer, alter Qt controls or authorize
publication. The owner's visual acceptance covers only previously inspected cases.
Historical boundary artifacts and correctness runners are preserved.

## Reproduce

Use the pinned prerequisites and staging procedure in
[the boundary experiment](../boundary-experiment/README.md). The build below
checks every inherited library/configuration identity and patched source hash
against the corrected boundary build record. All output/stage directories must
be new. Commands assume the repository root; replace local build paths as needed.

```sh
python3 specs/073-rendering-architecture/demonstrator/performance-experiment/build.py /tmp/qmarkdown-073-skia /tmp/073-boundary-reproduction-skia build-ci/073-boundary-reproduction build-ci/073-performance-new
python3 specs/073-rendering-architecture/demonstrator/performance-experiment/test_performance.py
python3 specs/073-rendering-architecture/demonstrator/performance-experiment/run.py build-ci/073-performance-new build-ci/073-performance-new-scale --mode scale
python3 specs/073-rendering-architecture/demonstrator/performance-experiment/scale.py build-ci/073-performance-new-scale
python3 specs/073-rendering-architecture/demonstrator/performance-experiment/run.py build-ci/073-performance-new build-ci/073-performance-new-cpu --mode cpu
python3 specs/073-rendering-architecture/demonstrator/performance-experiment/run.py build-ci/073-performance-new build-ci/073-performance-new-native --mode native
python3 specs/073-rendering-architecture/demonstrator/performance-experiment/run.py build-ci/073-performance-new build-ci/073-performance-new-captures --mode captures
python3 specs/073-rendering-architecture/demonstrator/performance-experiment/diagnostics.py /tmp/073-boundary-reproduction-skia build-ci/073-performance-new /tmp/073-performance-new-counter-skia build-ci/073-performance-new-diagnostics
python3 specs/073-rendering-architecture/demonstrator/performance-experiment/report.py build-ci/073-performance-new-cpu build-ci/073-performance-new-native build-ci/073-performance-new-report --charts
```

Run phases serially on an otherwise quiet desktop. Native measurements alone
have 324 × (2+5) seconds of warmup/measurement, plus initialization/drain/teardown.
`--case ID` reproduces one input; full runs cover every variant/API/repetition.
The chart command additionally needs Matplotlib in a separate tooling environment;
no shipping Python dependency is introduced. Wayland presentation protocol headers
are generated from the installed stable XML using `wayland-scanner`.

After interruption, use `--resume-from PRIOR` with a **new** output directory.
The runner validates source/executable identities, completed comparison geometry,
raw/fixture hashes when recorded, and durable phase checkpoints. Use
`--identity-map FILE` to map historical source paths to hash-matched restored files;
`--control ROOT --recovery-manifest FILE` identifies rebuilt upstream prerequisites
without changing the historical build record. Unacknowledged raw output and
truncated index tails remain preserved as interrupted evidence. Completed pairs
and timeout/skip decisions are imported. A shared exclusive execution lock is
inherited by measurement children, so an orphaned live child blocks another runner.
Later-phase resumes reuse validated upstream build and comparison checkpoints.

`retry.py BUILD ORIGINAL_NATIVE NEW_OUTPUT` repeats only incomplete native
processes once. Completed runs with unfavorable pacing or missing feedback are
never selected. Pass its output to `report.py --native-supplement NEW_OUTPUT` to
retain both attempts. `retain.py EVIDENCE INPUT...` archives raw files in bounded
parts and records every file/archive hash; it refuses existing archive paths.

## Measurement semantics

- Variants: clean upstream (no metadata), patched zero padding, patched `[6, 3]`
  padding. They share identical input text, style ranges, fonts and real objects.
  Boundaries intentionally affect joining/ligatures; padding affects wrapping.
- Fixtures use complete grapheme atoms and UTF-16 intervals, then pass the same
  ICU validation as the boundary correctness harness. Prose and long code reach
  exactly the requested lengths; all exact sizes and constant span coverage are
  also recorded in manifests. CPU document inputs have 10/100/1000 256-unit blocks.
  Native typical input reuses the existing nine-block illustrative `document`
  fixture; its table rows remain text illustrations, not general table support.
  Span-dense uses ten 1 Ki paragraphs; large-document uses 1,000 object-bearing
  256-unit paragraphs. These stress inputs are not claimed to represent every document.
- CPU construction includes source validation/mapping and paragraph building,
  excluding font loading. Initial layout includes shaping. Width reflow changes
  width by 17, then unchanged layout repeats that width. Metadata timing changes
  the first identity and performs the resulting shape/layout; paragraphs without
  code metadata report null. No fresh filesystem-cache claim is made.
- Each cold sample is a fresh process. The warm process performs five discarded
  iterations then 30 recorded iterations, constructing fresh paragraphs and
  resetting only the paragraph cache each time; font/typeface caches remain warm.
  Document fixtures repeat the same 256-unit block: within-document cache reuse
  is available upstream, while boundary metadata retains its existing cache bypass.
  Width/repeat/metadata operations reuse that iteration's paragraph. The 60-second
  deadline covers the full CPU case's cold/warm batch, so incomplete batches are
  explicitly partial. Larger sizes in that variant/series are skipped.
- The supervisor samples process RSS every 20 ms and kills its process group at
  4 GiB or the deadline. It retains complete JSONL records from interrupted runs.
  RSS is process resident memory, not allocation counts or GPU memory. Both the
  sampled process peak and `getrusage` high-water mark are retained. Native RSS
  includes the buffered measurement records as well as fonts, driver state and
  document data; it is not an isolated renderer-allocation measurement.
- Diagnostic counter builds copy the checked patched tree and rebuild only the
  diagnostic library. Candidate/fragment counters are absent from headline
  binaries. Counters aggregate initial/reflow/repeat/invalidation operations;
  inherited shaping-generation guards verify width reuse and metadata invalidation.
  Full historical geometry/source mapping and per-workload geometry are compared
  against uninstrumented binaries. A separate upstream diagnostic counts shaping
  for 1 Ki inputs in all five families and widths, plus 10-paragraph documents.
  Diagnostic durations are not headline timings.
- The native document adapter copies the validated full-document traversal. It
  disables console logging and parameterizes metadata. Upstream uses the original
  viewer's range-rectangle background painter with zero padding; patched variants
  use authoritative visual fragments. Color and corner styling match. Paint-cost
  comparisons therefore include these different rectangle/fragment query paths. No culling/cache/layout optimization is added.
  Startup layout completes before the two-second scenario warmup starts.
- Native phase durations are wall time on the render thread, not CPU cycles or
  utilization. Frame thread wall time includes blocking and excludes event polling.
  Frame thread wall time also includes measurement bookkeeping.
  CPU paint includes selection geometry and traversal. Acquisition, layout,
  submission and swap blocking are separate. The generated Vulkan helper keeps
  upstream semaphores/present transitions and times submission inside swap.
  Vulkan's reported swap duration is the remaining swap-path wall time after
  subtracting submission; it includes helper overhead as well as blocking.
  Supported Ganesh elapsed queries complete asynchronously. Unsupported, incomplete
  or failed-submission GPU statistics remain null. Cache bytes are Skia resource
  cache usage, never total device memory.
- Presentation requests attach to SDL's existing Wayland surface, without creating
  competing viewport/fractional objects. Each frame records presented/discarded/
  missing feedback, timestamp, refresh period, sequence, flags and output proxy.
  Pacing uses actual presentation intervals and the advertised clock resolution;
  swap-return times are never presentation timestamps. Each scenario/repetition
  remains separate. The 500 ms feedback drain is outside the timing window.
- Native samples are buffered until measurement and drain complete. Protocol
  traces and PNG captures are separate processes. A process killed before flushing
  its native buffer has unavailable frame metrics; its supervisor evidence remains.

## Interpretation

Read [retained performance evidence](../../evidence/performance-experiment/README.md)
for actual results, scale mapping, gaps, static curves and optimization proposals.
Summary statistics use median, nearest-rank p95 and maximum; partial sample counts
are explicit. Warm charts include only complete batches; fresh-process charts mark
partial batches with crosses and sample counts. Adjacent-size growth
ratios describe these workloads and are not an asymptotic complexity proof.
Missing scale information, GPU timing or presentation feedback never becomes zero
or a qualified pass. Initial layout, memory and stress outcomes are descriptive;
only the existing presentation pacing target is assessed. The Qt 6.8 Vulkan
shutdown blocker remains independent.
