# Completed live-only continuation

> Storage cleanup: generated artifacts linked below were removed from Git history.
> See the [retention policy](../../../../docs/artifact-storage.md); historical results remain, raw-data revalidation requires regenerated inputs.

**All seven checkpoints completed in all ten windows.** Seventy source/coordinate/
containment checks and thirty exact repeated-scale comparisons pass. Eight windows
shut down normally; the two Qt 6.8 Vulkan windows exited with signal 11. Original
125% configuration is restored and verified. [Consolidated report](../../scale-study-results.md).

[Live results](results/summary.json), raw archive (removed artifact: `results/study.tar.gz`),
[archive read-back](results/archive.json), [member hashes](results/members.json),
[checkpoint/mapping review](live-checkpoints.json), [review source](review_live.py),
raw-buffer contact sheet (removed artifact: `live-buffer-contact-sheet.png`),
[delivery validation](delivery-validation.json), and [consolidated cells](consolidated.json).
The contact sheet uses a common raw-buffer pixel scale; it is not a compositor
screen capture or proof of equal physical sizing. Agent inspection is not owner acceptance.

The live windows retained their initial fractional preference and surface sizing.
Ten lower-scale SDL checkpoints differ in inferred nominal physical font size from
fresh windows at the same desktop settings. Qt retains DPR 2 but its physical
nominal sizes follow the desktop scale through an unchanged 800 × 600 surface.
Repeatability and semantic checks pass independently of this mapping gap.

The [approval](approval.json) binds ten replacement live windows to the frozen
[schedule](schedule.json); no fresh-window rerun occurred. Actual total launches
are **685**, within 691. [Preflight](preflight.json), [freeze](freeze.json) and
[original output](original-output.json) retain preparation/approval history.
Preparation-time pending labels remain historical; approval preceded execution.

The atomic publisher passes eleven tests: deterministic old-race reproduction,
paused writer invisibility, concurrent publishers, no overwrite, serialization and
sync/link failures, Unicode round-trip, ten-window bounds and approval rejection.
[Tool archive validation](tool-archive.json), preparation Light log (removed artifact: `light-check.log`)
and [preparation whitespace](whitespace.json) preserve the repair evidence.
Final verification is linked from the consolidated report. No renderer, layout,
font, tolerance, dependency or public API changes were made.

Final Light checks pass: documentation/local links, evidence/configuration syntax
and all 87 repository Python tests. Final log (removed artifact: `final-light-check.log`),
[final whitespace](final-whitespace.json), [evidence hashes](artifacts.json).
