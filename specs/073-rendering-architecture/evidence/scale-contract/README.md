# Scale-contract evidence — complete bounded execution

> Storage cleanup: generated artifacts linked below were removed from Git history.
> See the [retention policy](../../../../docs/artifact-storage.md); historical results remain, raw-data revalidation requires regenerated inputs.

**Fresh physical sizing is corrected; live raster-density acceptance is blocked
by a minimal SDL3 reproduction of missing fractional-scale notifications.**
[Consolidated findings and prerequisite](../../scale-contract-results.md).
DP-1's original 125% configuration is restored and verified; all windows are closed.

- Final cell/gate results (removed artifact: `final-results.json`): 690 accounted cells, 674 final
  launches, sixteen dependent skips. Runner assertions/exit: 551 passed, 123 failed.
  These statuses do not override fifty failed live density/fresh-equivalence checks.
- [Restoration](restoration.json): exact original output configuration read-back.
- Archives and SHA-256 manifest (removed artifact: `archives.json`): fourteen sequential 32 MiB parts
  (last shorter); all 9,599 members and concatenated parts verified by read-back.
  Concatenate `scale-contract-part-001.tar.gz` through `scale-contract-part-014.tar.gz`
  in numeric order to recover the gzip tar archive. The archive contains raw traces,
  captures, append-only journals, failed attempts, frozen schedule/identities,
  candidate/probe binaries, source variants and runtime/build records.
- [Runtime inventory](runtime-inventory.json) and
  [compositor/library identities](compositor-identities.json).
- [Layout checks](layout-check.json), Light log (removed artifact: `light.log`),
  atomic-handoff tests (removed artifact: `handoff-tests.log`), runner tests (removed artifact: `runner-tests.log`).
- Fresh 200% contact sheet (removed artifact: `contact-sheets/fresh-2.png`) and
  live 150% contact sheet (removed artifact: `contact-sheets/live-1.png`). The directory contains all
  four fresh scales and seven live checkpoints; these are raw-buffer thumbnails,
  not compositor screenshots or owner visual acceptance.

Eleven diagnostic launches plus 674 final launches total 685, below 750. Recorded
automated time is 1,278.612 seconds; builds and human waits are excluded. d011 is
an explicitly recorded post-matrix minimal reproduction, within the separate
60-launch diagnostic allowance; it does not replace any final window or result.
Its 200 → 150 → 125% trace reproduces the stale preference without Qt or Skia.

No commit, push, publication, system package replacement or shipping change.

## Preparation and incremental execution records


Approved [protocol](../../scale-contract-protocol.md); isolated
[implementation](../../demonstrator/scale-contract/README.md).

Implementation and offline checks are complete; final graphical verification is
pending. Ten diagnostic launches are retained in `build-ci/073-scale-contract/diagnostics`.
The first adapter attempt d005 failed exact motion replay because mapping preceded
scroll advancement; d006 verified the correction. d008–d010 validate final candidate
OpenGL, Vulkan and Qt instrumentation. d007 keeps a mapped SDL3 window alive through
125 → 150 → 125%, observing fractional preferences 150 → 180 → 150 on the same
object, with buffers 1000×750 → 1200×900 → 1000×750. It exits cleanly.

SDL2-compatible creation reports an 800×600 buffer and a 640×480 viewport at 125%;
direct SDL3 reports an 800×600 surface and 1000×750 buffer. This demonstrates the
integration difference; it does not establish a defect in all SDL2 usage.

No final-matrix gates are declared passed from diagnostic evidence. Historical
sources and commit 8a392ae remain unchanged. No commit, push or publication.

## First final set

100% completed: 135 passed, 31 failed, four dependent skips (166 launches).
Thirty failures are known Qt 6.8 Vulkan exits; cell 014 is a preserved OpenGL
motion deadline with incomplete coverage. Both Skia backends and all Qt 6.12
configurations pass. All 1,635 obtained capture mappings pass physical-size and
raster-density assertions. Frozen identities and capture hashes verify.
[Full accounting](fresh-100-summary.json). Remaining scales and live pass pending;
no retries, commits or publication. Latest verified output is 100%; requested next
handoff is 125%, the recorded original setting.

## Second final set

125% completed: 136 passed, 30 known Qt 6.8 Vulkan shutdown failures, four dependent
skips (166 launches). All 1,644 obtained capture mappings pass physical-size and
raster-density assertions. Qt and SDL3 agree on the 800×600 logical surface and
1000×750 baseline buffer. Frozen identities and capture hashes verify.
[Full accounting](fresh-125-summary.json). Latest verified output is 125%; next
requested handoff is 150%. Remaining scales and live pass are pending.

## Third final set

150% completed: 136 passed, 30 known Qt 6.8 Vulkan shutdown failures, four dependent
skips (166 launches). All 1,644 captured mappings pass physical-size and density
assertions. Qt and SDL3 agree on an 800×600 logical surface and 1200×900 baseline
buffer. Frozen inputs and capture hashes verify.
[Full accounting](fresh-150-summary.json). Latest verified output is 150%; next
requested handoff is 200%, followed by the ten persistent-window pass.

## Fourth fresh set and persistent windows

[200% accounting](fresh-200-summary.json): 136 passed, thirty Qt 6.8 Vulkan failures,
four skips. Two Vulkan cells also have incomplete correctness coverage: capture
timeout 562 and keyboard-contaminated replay 563. All 1,636 obtained scale mappings
pass. The four fresh sets total 543 passes, 121 failures, sixteen skips.

Ten live windows pass the [initial 200% audit](live-0-summary.json). At the
[150% transition](live-1-summary.json), all retain density 2 and 1600×1200 buffers,
despite updated output dimensions. No new fractional preference appears in their
traces. Physical/source checks pass, but raster density and fresh equivalence fail.
The unchanged windows remain active, awaiting 125%; no gate is relabeled passed.
