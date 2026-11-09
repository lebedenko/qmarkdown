# Retained candidate-scan implementation

Feature 073 R1–R5 / R-T1–R-T5, explicitly approved by the owner's request to
implement the supplied plan on 2026-11-06. This isolated Skia change excludes
runtime measurements, shipping Qt integration, ownership changes, merge and
publication. The completed proof experiment and its archives remain unchanged.

`candidate.patch` is the clean review artifact against the hash-validated ownership
baseline. It changes only TextWrapper.cpp and adds ScanBound.h. It contains no
oracle/shadow selector, counters, shadow evaluation or proof-only fatal diagnostics.
`diagnostic.patch` is a separate overlay from those clean sources to the diagnostic
copy. `implementation.py` reproduces both from checked replacements of the proven
prototype and audits diagnostic removal. No ownership source changes enter the
clean patch. The two builds use the inherited pinned dependencies and compiler
settings; diagnostic state is confined to the overlay.

```sh
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-implementation/test_harness.py
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-implementation/run.py build-ci/073-candidate-implementation-fresh build-ci/073-candidate-implementation-fresh-evidence
python3 specs/073-rendering-architecture/demonstrator/candidate-scan-implementation/report.py build-ci/073-candidate-implementation-fresh-evidence
```

Fresh directories are mandatory. The schedule is persisted before building: twelve
fixtures, zero then padded, oracle/shadow/instrumented prototype/clean retained,
96 fresh serial processes. Each covers initial layout, width +17, unchanged repeat
and metadata mutation/layout. Exact geometry, glyph positions, visual slices,
mappings and shaping deltas compare against the hash-validated archived runtime
correctness traces and the scheduled oracle. Ownership lifecycle checks remain in
the diagnostic arm. The clean arm emits geometry and shaping only.

The bound and arithmetic envelope are unchanged from the
[proof](../candidate-scan-experiment/README.md): complete ordered enumeration and
whitespace arithmetic through the first hard break inclusive, first indivisible
endpoint collected during enumeration, reverse suffix minima and strictly greater
than available width to stop expensive evaluation. Legal preference, emergency,
visual slices, bidi and shaping remain unchanged. Nonfinite/negative padding,
nonfinite inputs/intermediates, unsupported representation, fast math, C/MXCSR
rounding and FTZ/DAZ reject the bound and use the original exhaustive loop. Partial
endpoint choices are cleared before fallback, including late required-addition
failure. The original loop's arithmetic order remains intact.

Environment guards, padding validation and reserve occur only when the existing
padded scanner applies. Zero horizontal and vertical padding has zero guard visits,
preprocessing, trimming and scratch allocation. Vertical-only padding still enters
the padded scanner. One layout-local vector preserves the prototype Candidate
representation and reserves paragraph cluster count, reuses capacity between lines
and releases it on return. No persistent cache or public API is added.

Inherited exclusive locking and child lock inheritance, 60-second process deadlines,
4 GiB sampled RSS ceilings and terminal failures without automatic retries apply.
Build, execution and gate journals are separate. A completed-run resume requires a
new destination and validates identities, checkpoints, raw hashes and comparisons
without launching any processes. Archives include both source copies, patches,
configuration, reproduction dependencies, commands, environment, raw outputs and
reports; every file and archive hash is verified.

Numeric coverage retains exhaustive short sequences and 10,000 seed-73074 cases.
Focused coverage includes hard breaks, trimming, graphemes, bidi, adjacent identities,
objects, zero widths, oversized first units, absent legal breaks, vertical-only
padding and fractional widths. Actual scanner fallback exercises C rounding,
mismatched MXCSR rounding, FTZ, DAZ, negative/nonfinite padding and envelope overflow;
fast-math rejection uses its own build target. Historical boundary/extent tolerances
are unchanged. Guard/preprocessing/trimming/storage counts describe work only and
do not establish a runtime speedup.

Actual results: [evidence](../../evidence/candidate-scan-implementation/README.md).
