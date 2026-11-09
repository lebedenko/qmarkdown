# Isolated scale contract

Approved scope and limits: [protocol](../../scale-contract-protocol.md).
Historical runners and sources remain unchanged. The candidate links the original
explicit-shaping-boundary Skia libraries and changes only its SDL integration and
coordinate mapping; Qt changes are observation only.

`mapping.h` separates document, logical content, toolkit window/Wayland surface
and buffer coordinates. On Wayland window coordinates are surface coordinates.
Window content scale is SDL window display scale / pixel density; each buffer axis
uses its actual integer extent ratio. Padding, font size and document zoom retain
their original units. Layout uses logical viewport width; pointer and synthetic
selection use the inverse drawing mapping. Frame mapping is sampled **after**
motion advances. Buffer changes invalidate GPU surfaces; logical width/zoom changes
invalidate layout. Captures reference the generation that produced their geometry.

SDL API meanings follow [upstream high-DPI guidance](https://wiki.libsdl.org/SDL3/README-highdpi).
No desktop-scale parameter enters the renderer, and no Wayland viewport is owned
by the adapter. Desktop read-back is an independent assertion only.

`prepare.py` checks frozen source hashes before generating an isolated overlay.
`build.py OUTPUT` builds exact Qt 6.8.0, Qt 6.12.0 and SDL3 3.4.18 with two workers,
then verifies all fifteen fixtures without graphical windows. `probe.cpp` builds
minimal Qt and SDL2/SDL3 probes; `--live` retains the window until stdin `quit`.
Raw toolkit timestamps and Wayland object/timestamp records are retained together.

`diagnose.py` reserves diagnostic slots before launch and rejects duplicates.
`live_probe.py` preserves a single mapped probe while operator handoffs are read
back; waits are excluded from automated time. `run.py` loads the unchanged v2
supervisor with atomic publication and generation assertions. It freezes a fresh
690-cell schedule, reserves diagnostics against the four-hour limit, and uses its
original lifecycle and dependent-recreation rules. No automatic retries.

```sh
python3 -m unittest discover -s specs/073-rendering-architecture/demonstrator/scale-contract -p 'test_*.py'
cmake -S specs/073-rendering-architecture/demonstrator/scale-contract -B build-ci/073-scale-contract/probes -G Ninja
cmake --build build-ci/073-scale-contract/probes -j 2
ctest --test-dir build-ci/073-scale-contract/probes --output-on-failure
```

Evidence and actual coverage: [record](../../evidence/scale-contract/README.md).

## Source retention after cleanup

The five files in `baseline/` are the exact source inputs previously copied from
an ignored scale-study build tree. `frozen-inputs.json` verifies their unchanged
bytes at repository-owned paths. Source generation and mapping tests work without
old builds or raw archives. Full graphical builds still require rebuilding the
pinned Qt/Skia dependencies described by the historical preparation scripts.
Removed raw traces/captures are not available for historical revalidation.
