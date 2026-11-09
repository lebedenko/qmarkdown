# Public Qt placement gate

This is an actual-source interactive **placement control**, not a complete
Markdown architecture demonstrator. It uses neither the shipping renderer nor a
proxy. The object is a shaping placeholder, not an implemented object renderer.

Build with the exact SDK prefix:

```sh
cmake -S specs/073-rendering-architecture/probe -B build-ci/073-development -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-ci/073-development -j2
python3 specs/073-rendering-architecture/probe/run.py build-ci/073-development/placement-probe build-ci/073-new-evidence --qt 6.12.0 --compositor-scale 1.25
```

The output directory must be new. The compositor scale argument is the observed
native scale with `QT_SCALE_FACTOR=1`; each run independently checks its resulting
window DPR. All subprocesses have a 20-second deadline and preserve the last
flushed stage. This runner bounds the entire short process; it does not diagnose
inside a driver call. Renderer identity is observed from Qt, not inferred from
environment variables. QSG/RHI logs retain available device/driver details.

For interactive inspection (close the window to exit):

```sh
QT_QPA_PLATFORM=wayland QSG_RHI_BACKEND=opengl QT_SCALE_FACTOR=1 build-ci/073-development/placement-probe curve /tmp/073-review /usr/share/fonts/TTF/DejaVuSans.ttf --interactive
```

Use `distance` instead of `curve` for the independent distance-field process.
The window moves actual text slowly with reversals. PNGs retain original and
requested positioned runs plus the before/after text-node captures. Raster
controls intentionally demonstrate geometry, not cross-renderer pixel equality.
Swap notifications exclude captures but are not compositor presentation or GPU
work. Neither these notifications nor exact resting replay establish acceptance.
