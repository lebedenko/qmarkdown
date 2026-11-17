# Coordinated four-scale correctness study

Approved 2026-11-09 by the owner's supplied-plan implementation request. This
replaces the proposed design-only milestone. No shipping renderer/API changes,
scanner optimization, performance qualification, backend selection, commits or
publication are authorized. Preserve historical evidence through ba843e0 and the
uncommitted architecture review. Scale confirmations are coordination handoffs.

## Requirements

- SS1: Exact Qt 6.8.0 and 6.12.0, distance-field/curve × OpenGL/Vulkan, followed
  by pinned explicit-boundary Skia OpenGL/Vulkan. Fifteen fixtures: document first,
  remaining shared fixtures, adjacent-lam-alef and whole-lam-alef. Preserve fonts,
  shaping, padding and geometry tolerances. Reuse verified libraries and preparers.
- SS2: Actual DP-1 desktop scales 1, 1.25, 1.5, 2; remove Qt scale overrides.
  Record output identity, window/surface/buffer extents, Qt DPR or SDL drawable
  ratios, zoom, API and source/binary/font hashes. Scale comparisons use zoom 1.
- SS3: Each fresh fixture has stationary, 60 slow/flick/reversal frames, captured
  pose replay, clipping/settling, resize/reset, separately labelled zoom/reset,
  selection, exact copied source and coordinate assertions. Two extra normal
  document processes supply three independent lifecycle observations.
- SS4: Ten persistent bidi-objects windows at 2, then transitions through
  1.5, 1.25, 1, 1.25, 1.5, 2. Pause other scripted activity during checkpoints;
  preserve windows/contexts, check both selection directions, object containment,
  inverse input mappings and repeated states. Observe Wayland; no viewport owner.
- SS5: Append-only attempts, immutable captures, validated resumable checkpoints;
  no retries or replacement samples. Missing coverage is untested. Unavailable
  requires an attempted prerequisite and exact reason.

## Design and fixed limits

Fresh schedule: 4 × 10 × 17 = 680 processes, plus ten persistent processes:
**690 maximum**. Serial configuration order: Qt 6.8.0, Qt 6.12.0; each distance
then curve, each OpenGL then Vulkan; Skia OpenGL then Vulkan. Within each group,
15 combined fixture processes precede two normal document recreations. A failed
initial document skips dependent recreations; independent cells continue.

Startup 20 seconds, operation 10, shutdown 10, fresh absolute deadline 120;
sampled RSS 4 GiB per process, live aggregate 8 GiB; automated total four hours.
Explicit human waits and build time do not count. Build with two workers.
Integrity failure, unreliable output identification or whole-study resource limit
stops execution. Wrong confirmed scale leaves the set pending without launches.
Interrupted attempts stay failed; resume advances only to untouched cells.
Record original output configuration before execution, request restoration after
completion/termination and verify read-back.

Qt mapping is document → item/scene using existing item transforms → buffer using
window DPR. SDL mapping is document → window using existing margin/offset/zoom →
drawable using measured ratios. Wayland surface/buffer mapping is observed from
protocol traces, including output enter/name, viewport and fractional-scale events.
Desktop scale and drawable ratio are distinct. Never alter nominal font sizes to
force equality. Equal nominal dimensions must map to equal physical sizes with
only recorded integer-buffer rounding; otherwise comparability fails independently.

## Gates and tasks

- SS-T1 (SS1–SS2): Verify provenance including archived sources, graphical access,
  DP-1 identity and original configuration; preserve existing work.
- SS-T2 (SS2–SS5): Implement isolated instrumented viewers, coordination and bounded
  supervisor. Test wrong-scale/output gating, deadlines, malformed records, RSS,
  interruption/resume, duplicate prevention and scale-only changes without buffer
  changes. Verify unchanged layout/preparation behavior before graphical execution.
- SS-T3 (SS1–SS3): Freeze identities, commands, schedule and expected assertions;
  execute each confirmed fresh scale set once and validate before advancing.
- SS-T4 (SS4–SS5): Execute persistent transitions without recreations; retain all
  attempts and restore original scale.
- SS-T5: Validate all cells as passed/failed/skipped/untested, capture and archive
  hashes/read-back; retain contact sheets/motion evidence. Run final Light,
  document/link and tracked/new whitespace checks. Update decision and verification.

Configuration, correctness, comparability, transitions and visual-review gates are
separate. Agent inspection never implies owner visual acceptance. No result relaxes
support requirements or declares production qualification. Actual results belong in
[verification](verification.md) and [study evidence](evidence/scale-study/README.md).

## Corrected complete scale study — approved 2026-11-09

The owner's “Well, let's proceed” approves correcting the harness and running a
new complete study, following the explicit proposal of 691 total graphical launches:
one preserved failed attempt plus a fresh 690-process schedule. This supersedes the
suggested untouched-cell continuation. The original terminal run and frozen sources
remain immutable. Renderer/platform failures retain the existing independent-cell
and dependent-recreation rules; no additional adaptive samples are authorized.

Requirements: SS1–SS5 remain unchanged except the explicit replacement study budget.
Design: use a sibling `scale-study-v2` supervisor with both wl_output names and
xdg-output object bindings; reject conflicts, missing bindings and wrong outputs.
Persist exit, elapsed time, sampled peak RSS and capture hashes before validation;
record a failed result and journal completion even if validation raises. Reuse the
byte-identical verified viewers/libraries; no renderer or instrumentation rebuild.
Reserve the original failed attempt's 120 seconds conservatively against the same
four-hour automated budget, excluding build time and human waits.

Tasks: test the actual retained trace and negative identity cases; test failure
recording and terminal recovery; verify original evidence and reused binary hashes;
freeze a separately identified full schedule; coordinate all four scales and live
transitions, validate/archive results and restore the original scale. No commits,
publication, performance qualification or backend selection are authorized.

## Atomic live handoff repair and bounded continuation proposal — 2026-11-09

The v2 live supervisor stopped at live-5 when it observed a confirmation file
between creation and JSON write. All fresh sets and live-0 through live-4 remain
immutable. The owner restored 125%; read-back matched the original output.
Fixing the harness within the approved study is authorized; replacing terminated
windows is a separate, concrete execution amendment pending owner approval.

Repair requirements/design: write a sibling temporary file, serialize/flush/fsync
it, atomically publish using a no-overwrite hard link on the same filesystem, sync
the directory, and remove the temporary name. Final filenames are never exposed
with partial content; duplicates must fail without changing existing evidence.
Test the original race, a deliberately paused writer, concurrent publishers,
serialization/fsync/link failures and duplicate rejection without graphical runs.
Reuse the frozen v2 supervisor and byte-identical viewers via an isolated wrapper;
change only publication and the recovery schedule/budget enforcement.

Proposed execution: ten new persistent bidi-objects windows, initial desktop 2,
then 1.5 → 1.25 → 1 → 1.25 → 1.5 → 2, with the original checks/limits. No fresh
matrix rerun. Prior actual launches are 675 (one original, 664 v2 fresh, ten v2
live); the proposal totals 685, within the ceiling of 691. This explicitly replaces
ten live attempts; unused fresh recreation slots do not silently grant approval.
Conservatively reserve 120 seconds for each historical harness failure alongside
completed fresh automated time within the unchanged four-hour budget.

Tasks: preserve/validate terminal evidence and restoration; implement/test atomic
publication; prepare/hash a ten-cell continuation with no-launch preflight; obtain
explicit approval of that replacement scope; execute once, restore 125%, archive
and reconcile complete versus failed/untested coverage. No publication or commit.

Owner approval (2026-11-09): **“Proceed with ten replacement windows”** explicitly
approves the prepared live-only continuation above, its atomic handoff repair and
ten-cell frozen schedule. No fresh-window rerun; total actual launches after this
continuation are bounded to 685 within the existing ceiling of 691. All other
checks, exclusions and restoration requirements remain unchanged.
