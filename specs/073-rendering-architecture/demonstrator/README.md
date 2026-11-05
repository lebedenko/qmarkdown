# Native rendering comparison demonstrator

Standalone feature-073 experiment. Shipping source, dependencies and APIs are
unchanged. The original [placement probe](../probe/README.md) remains separate.

## Build

Run from the repository root. Python 3, CMake, Ninja and the exact Qt SDK are
required. Staging refuses an existing destination; it extracts the recorded Git
commit and modifies only that copy. No working-tree source is copied implicitly.
Staging checks and applies [prepared-cursor.patch](prepared-cursor.patch), retaining
its SHA-256 and output hashes in demo-source.json. Existing stages predate this
correction: create a fresh destination and point PREPARED_STAGE at it.

~~~sh
python3 specs/073-rendering-architecture/demonstrator/stage.py build-ci/073-prepared
cmake -S specs/073-rendering-architecture/demonstrator -B build-ci/073-demo-development -G Ninja -DCMAKE_BUILD_TYPE=Release -DPREPARED_STAGE="$PWD/build-ci/073-prepared"
cmake --build build-ci/073-demo-development -j2
cmake -S specs/073-rendering-architecture/demonstrator -B build-ci/073-demo-minimum -G Ninja -DCMAKE_BUILD_TYPE=Release -DPREPARED_STAGE="$PWD/build-ci/073-prepared" -DCMAKE_PREFIX_PATH="$PWD/build-ci/065-scenegraph/native-sdk"
cmake --build build-ci/073-demo-minimum -j2
~~~

The development prefix above uses the installed Qt 6.12.0. The minimum path is
the existing local Qt 6.8.0 SDK; substitute its location on another machine.

Skia is independently built outside shipping dependencies. Required system
development packages: SDL2, Vulkan, OpenGL, FreeType, HarfBuzz, ICU, libpng,
Fontconfig, zlib and nlohmann JSON headers. Use the exact revision in
[skia-pin.json](skia-pin.json), not a moving branch:

~~~sh
git init /tmp/qmarkdown-073-skia
git -C /tmp/qmarkdown-073-skia remote add origin https://skia.googlesource.com/skia.git
git -C /tmp/qmarkdown-073-skia fetch --depth 1 origin 00227028b0c3cb3052432a456af2027a49e598b2
git -C /tmp/qmarkdown-073-skia checkout --detach 00227028b0c3cb3052432a456af2027a49e598b2
python3 specs/073-rendering-architecture/demonstrator/build_skia.py /tmp/qmarkdown-073-skia
cmake -S specs/073-rendering-architecture/demonstrator/skia -B build-ci/073-skia-demo -G Ninja -DCMAKE_BUILD_TYPE=Release -DSKIA=/tmp/qmarkdown-073-skia
cmake --build build-ci/073-skia-demo -j2
~~~

The build script checks the pin, fetches three exact external revisions and the
pinned GN tool, and records system dependency versions and archive hashes.
Upstream's pinned window helper owns Vulkan resources and swapchain; SDL creates
the Wayland surface. Generated helper copies adapt the Wayland extension allowlist
and initial extent/failed-initialization queue handling, plus the sRGB surface color type. GL functions are resolved
through SDL. Helper compilation imports the library's actual ABI definitions.

## Inspect

~~~sh
python3 specs/073-rendering-architecture/demonstrator/launch.py --renderer distance --backend opengl --fixture document
python3 specs/073-rendering-architecture/demonstrator/launch.py --renderer curve --backend vulkan --fixture padded
python3 specs/073-rendering-architecture/demonstrator/launch.py --renderer skia --backend opengl --fixture objects
python3 specs/073-rendering-architecture/demonstrator/launch.py --renderer browser
~~~

The browser option writes a self-contained HTML reference with embedded fixture
fonts into /tmp/073-review/reference.html. Open it in a browser. CSS flow is a
visual reference, not a pixel or selection oracle.

Space pauses/resumes; S selects slow movement; F selects flick-like movement;
R reverses; 0 resets; +/- changes document zoom. Resize with the window manager.
Drag to select within a paragraph and Ctrl+C copies the corresponding manifest
source (including object source labels). Cross-paragraph drag selection is not
implemented. Titles identify Qt native/raster blocks and Skia's unqualified
reservation composition. Titles are outside the captured document.

Qt's --scale is a window DPR multiplier, separate from document zoom. SDL's
window DPR is compositor-controlled; the launcher rejects using this option to
pretend SDL zoom is a DPR change. Close windows to exit.

The manifest uses UTF-16 source offsets. Qt preserves the staged preparer's
source mapping; Skia records a separate mapping for inserted padding reservations
and collapsed objects. Skia reservations remain experimental until wrapping/bidi
and selection geometry are accepted; replay alone cannot qualify composition.
Table rows currently provide text and explicit thin separators, not a general
table layout engine. The historical overlapping scene is retained under feature
065 and the original placement probe, not used for normal document positioning.

## Reproduce and verify

All output directories must be new. GPU invocations must run sequentially.

~~~sh
python3 specs/073-rendering-architecture/demonstrator/test_runner.py
python3 specs/073-rendering-architecture/demonstrator/run.py build-ci/073-demo-development/qt-compare build-ci/073-new-development --qt 6.12.0 --compositor-scale 1.25
python3 specs/073-rendering-architecture/demonstrator/run.py build-ci/073-demo-minimum/qt-compare build-ci/073-new-reduction --qt 6.8.0 --compositor-scale 1.25 --fixtures ligatures --dprs 1 --backends vulkan --cases empty,rectangle,static,capture,motion,combined
python3 specs/073-rendering-architecture/demonstrator/run_skia.py build-ci/073-skia-demo/skia-compare build-ci/073-new-skia --dpr 1 --cycles 3
~~~

Add --close to the Qt runner to exercise the window-close path, separately from
application quit. This is programmatic window close; an owner-operated close still
needs visual review. Repeating a run in three new output directories recreates all
Qt windows in independent processes. Skia's --cycles 3 does the same.

The supervisor bounds startup at 20 seconds, captures/replays at 10 seconds and
shutdown at 10 seconds. It retains partial logs, rejects malformed/truncated JSON,
wrong observed API/DPR/SDK and changed inputs, and does not stop later cases after
a failure. A successful process requires a final exit record. Qt combines
stationary, translated-pose/replay, clipping, settling, resize and zoom/reset.
Skia also combines clipping, settling, resize and zoom/reset replay. The separate motion runner samples 60 rendered steps through slow movement,
flick-like movement and reversal, then verifies captured-pose replay.

Native Qt layouts/nodes survive translation. Raster fallback paints at the
transformed physical phase, rounds only texture bounds, uses nearest sampling and
reuses identical phase/extent textures. Selection uses an explicitly labelled
raster region. Logs count raster work/uploads separately from native nodes.
Preparation times and capture stages are not presentation pacing or GPU timings.

See [verification](../verification.md) for actual results and remaining gates.


## Geometry checks and known unsupported cases

~~~sh
QT_QPA_PLATFORM=offscreen build-ci/073-demo-development/qt-selection specs/073-rendering-architecture/demonstrator/fixtures.json
QT_QPA_PLATFORM=offscreen build-ci/073-demo-minimum/qt-selection specs/073-rendering-architecture/demonstrator/fixtures.json
build-ci/073-skia-demo/skia-selection specs/073-rendering-architecture/demonstrator/fixtures.json
~~~

With the corrected stage, Qt selection checks pass on both exact SDKs, including
`bidi-objects` copying the full `code` range. Reverse drags use the same normalized
logical interval for painting and copy. Skia geometry checks deliberately exit 1:
source copying passes, but per-fragment padding and some placeholder vertical
bounds fail. These are composition failures, even when GPU replay passes.
See the [current support matrix](../evidence/correctness/support.json); the
[earlier support matrix](../evidence/continuation/support.json) remains historical.

Run the launcher with --fixture bidi-objects and drag across `code` in both
directions; check the highlight and copied text, then resize across its wrap.
Use --fixture unicode-code for marks, surrogate pairs, mixed directions and an
object in one paragraph. Skia --fixture wrapping remains an unsupported control.
Owner still-image and temporal acceptance are separate from automated results.

For review, inspect both slow motion and flick/reversal on actual windows.
Automated exact replay cannot provide owner visual acceptance.


~~~sh
python3 specs/073-rendering-architecture/demonstrator/motion.py build-ci/073-new-motion
~~~

The motion runner covers padded code, objects and the document on both exact Qt
SDKs, both Qt renderers, both APIs and four observed Qt DPRs; independent Skia
controls use the available SDL DPR 1. It runs all GPU cases serially. Sixty
rendered steps do not establish sixty compositor presentations or pacing quality.


## Bounded correctness reproduction

All output directories below must be new. Run native commands serially; the
native matrix takes approximately 25 minutes on the recorded host. It runs the
complete fixture matrix, three recreations of affected fixtures and separate
slow/flick/reversal replay. Selected captures must differ from unselected captures
and report exact source. Skia runs use the original reservation layout; the
rejected per-fragment trial is restricted to geometry diagnostics.

~~~sh
python3 specs/073-rendering-architecture/demonstrator/correctness.py geometry build-ci/073-new-geometry
build-ci/073-skia-demo/skia-selection specs/073-rendering-architecture/evidence/correctness/skia-minimal.json --fragment-trial
python3 specs/073-rendering-architecture/demonstrator/lifecycle.py build-ci/073-new-lifecycle
python3 specs/073-rendering-architecture/demonstrator/correctness.py native build-ci/073-new-native
~~~

Geometry emits the exact generated width-scan manifest, compressed traces,
failures and identities. The trial bounds convergence at eight passes and rejects
cycles explicitly. Stable boundaries still require passing padding, object and
source checks. The geometry command returns failure for the retained Skia gaps.

The standalone qt-lifecycle target has no dependency on staged text preparation
or the fixture manifest. The lifecycle runner compares default/basic render loops,
ordinary shutdown and --release-live, with three recreations and bounded debugger
stacks on each exact SDK. Qt 6.8.0 Vulkan still fails all variants; no workaround
is adopted. Both lifecycle and native runners return failure for this known gap.
GPU access and gdb are required for the native lifecycle diagnostics.

See [verification](../verification.md) for actual outcomes, retained failures and
the capabilities required next. No shipping changes or backend selection follow
from these experiment results.
