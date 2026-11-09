# Four-scale study evidence

> Storage cleanup: generated artifacts linked below were removed from Git history.
> See the [retention policy](../../../../docs/artifact-storage.md); historical results remain, raw-data revalidation requires regenerated inputs.

**Terminal harness failure after one graphical process; 689 scheduled cells remain
untested.** DP-1's original **1.25** scale was restored by the owner and verified
against the complete original output configuration. No retry or replacement ran.
See the [approved protocol](../../scale-study-protocol.md).

The attempted cell was Qt 6.8.0 / distance-field / OpenGL / document at desktop
scale 1. The frozen validator requires `wl_output.name`; this Qt session supplied
`zxdg_output_v1.name` instead. The trace binds xdg-output 22 to wl_output 21,
names it DP-1, then reports surface 33 entering output 21. The viewer also reports
DP-1, DPR 1, zoom 1, an 800 × 600 window and matching capture buffer.
This is a **validator defect**, not evidence of a renderer or output-placement
failure. The original terminal journal and validator are preserved unchanged.

| Gate | Actual outcome |
| --- | --- |
| Configuration | Desktop 1 verified before launch; frozen protocol validator failed. Retrospective trace read-back independently establishes DP-1. |
| Correctness | The one viewer records exit 0, 60 motion frames, three exact replay assertions, passing inverse/containment checks and exact copied heading in both selection directions. The cell remains failed overall. |
| Comparability | Untested: no second configuration or desktop scale launched. |
| Transitions | Untested: no persistent windows launched. |
| Visual review | Agent inspected the retained contact sheet; owner acceptance untested. |
| Restoration | Passed: full output configuration matches the original read-back. |

[Complete 690-cell summary](results/summary.json), raw evidence archive (removed artifact: `results/study.tar.gz`),
[archive read-back](results/archive.json), [member hashes](results/members.json),
[forensic checks](forensic-validation.json), contact sheet (removed artifact: `contact-sheet.png`),
[restoration](restoration.json), [delivery validation](delivery-validation.json),
and [frozen source/binary/library/font archive identities](frozen-inputs.json)
retain the failed attempt without relabelling it a pass. The raw archive includes
all ten captures, the 60-frame motion records, stdout, protocol trace, immutable
attempt journal and the verified first-scale handoff. Frozen inputs are retained
in `frozen-inputs.tar.gz` and verified by member read-back.

The process exited before validator failure; no process-wall duration or sampled
RSS result survived the exception. These quantities remain unknown; viewer
monotonic exit time is 1698.747924 ms. Resume conservatively reserves 120 seconds
for the uncompleted journal attempt and rejects this terminal study before launch.
The post-stop result is explicitly labelled forensic, not a supervisor completion.

- [Frozen schedule](schedule.json): 680 fresh processes plus ten persistent windows.
- [Freeze](freeze.json) and [identities](identities.json): commands, inputs and limits.
- [Original output](original-output.json): Samsung U32J59x on DP-1, 3840 × 2160.
- [Preparation](preparation.json): exact retained Skia libraries and five patched
  sources match the boundary experiment; retained metadata records the later
  original-control rebuild and is not claimed byte-identical to the older metadata.
- [Offline provenance](preflight.json): archived fixture read-back, original fonts,
  source identities and retained layout/preparation bodies pass.
- [Qt library provenance](qt-library-provenance.json): both retained libraries match
  historical geometry-final identities.
- [Layout checks](layout-check.json): 15 fixtures × three exact builds, 45 passes;
  observer calls preserve checked geometry/shaping, with inverse mapping,
  containment and bidirectional copied-source checks passing.
- Fourteen supervisor tests pass before execution, including wrong-scale/output
  rejection, missing confirmation, deadlines/RSS boundaries, malformed records,
  interruption/resume, duplicate launches, immutable writes and scale-only events.

The first isolated build failed because Qt 6.8 lacks the newer DPR signal. The
overlay now observes its DPR-change event; all final viewers build successfully
with two workers. No graphical process was used for this build correction.
Historical evidence is unchanged. Final check results are recorded in the
[verification record](../../verification.md#coordinated-scale-study-terminal-delivery--2026-11-09).

Final Light checks pass: documentation/local links, evidence/configuration syntax
and 87 repository Python tests. Log (removed artifact: `light-check.log`); [whitespace](whitespace-check.json).
[Artifact hashes](artifacts.json) cover the delivered evidence files.
