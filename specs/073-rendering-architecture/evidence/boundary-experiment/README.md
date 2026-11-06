# Explicit-boundary experiment evidence — 2026-11-05

**Feasible for the tested isolated slice; owner visual acceptance pending.**
See [support](support.json), [geometry summary](geometry-summary.json),
[extent audit](extent-audit.json), [native summary](native-summary.json) and
[reproduction/viewer instructions](../../demonstrator/boundary-experiment/README.md).
Historical evidence under `fragment-experiment` is unchanged.

- [Build record](build.json) and [build log](build.log): clean pinned source,
  checked patch, dependencies/configuration, two-worker build and identities.
- [Geometry archive](geometry.tar.gz): manifests, full compressed traces, invalid
  input diagnostics, transition probes and actual results. Metadata-free control
  comparison is exact for 751 cases / 775 blocks. Boundary-enabled zero padding
  is the separate shaping baseline. The padded scan and 14 added regressions pass.
- [Transition brackets](transitions.json): 41 detected line-signature changes,
  refined to 1/64 pixel, with 147 block probes immediately below/at/above them.
- [Extent audit](extent-audit.json): 14,272 code cells and 2,901 fragments pass;
  95 differences between 0.01-pixel near-edge probes and stable interior probes
  remain recorded as upstream snapping behavior. The 0.05 containment tolerance
  is unchanged.
- [Native archive](native.tar.gz): 120 serial OpenGL/Vulkan processes with
  stationary/selected, resize/reset, motion/replay and three normal recreations
  per fixture/API. Deadlines and pre/post input hashes are retained. DPR 1 was
  observed; DPR 1.25/1.5/2 were unavailable.
- [Supervisor log](supervisor.log): nine tests pass. [Repository checks](repository-check.log):
  documentation/configuration checks and 87 Python tests pass.
- [Artifact hashes](artifacts.json): SHA-256 of retained evidence files.

The native run used the [earlier build record](native-build.json). Provenance
review found that the inherited copy routine dereferenced a tracked relative
`Cargo.toml` symlink. Corrected staging preserves the symlink and accepts only the
five patched runtime files. The [rebuild equivalence record](rebuild-equivalence.json)
proves that all libraries, executables and patched runtime sources are identical.
CPU gates were rerun on the corrected fresh stage; native checks were not repeated
for byte-identical inputs. The earlier build script and pin are retained in
`native-inputs/` with the exact hashes referenced by the native process records.
No production Cargo dependency or source file changed.

The final stored patch also removes whitespace-only context prefixes for repository
whitespace validation. [Patch normalization](patch-normalization.json) records a
successful fresh application and byte-identical hashes for all five compiled
sources. The exact previously tested patch is retained as `tested-patch.gz` and
its five-file pin as [tested pin](tested-patch-pin.json). No runtime inputs changed.

`trials/` retains failed exploratory checks: synthetic-width fitting overflow,
metadata invalidation with accumulated bidi regions, and 0.01-pixel caret probes
that entered upstream's snapping zone. These are not final acceptance evidence.
They remain separate from the unchanged failures in commit `d3b1f05`.

`objects@679` now has baseline 31, rounded height 42 and zero metric rounding delta.
The logical object top/bottom are 3/39; its isolated antialiased DPR-1 raster bounds
are also vertically 3/39. Both pass containment. Original raw ascent/descent and
all logical/painted bounds are retained in the geometry archive.

Representative matched captures: [GL adjacent](opengl-adjacent.png),
[Vulkan adjacent](vulkan-adjacent.png), [GL objects](opengl-objects.png),
[Vulkan objects](vulkan-objects.png), [GL adjacent lam–alef](opengl-adjacent-lam-alef.png),
[Vulkan adjacent lam–alef](vulkan-adjacent-lam-alef.png),
[GL whole lam–alef](opengl-whole-lam-alef.png),
[Vulkan whole lam–alef](vulkan-whole-lam-alef.png).

No performance qualification, backend selection, Qt-control change, production
integration, merge or publication is claimed. Qt 6.8 Vulkan shutdown remains an
independent historical blocker.
