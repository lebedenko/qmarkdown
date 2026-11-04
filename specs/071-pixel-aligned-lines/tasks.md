# Mapped tasks

Approved on 2026-11-04 by the owner request to implement ("Approved, go with nearest
rounding"). Approval covers requirements, design, tasks, nearest-device-pixel
thickness rounding with a one-pixel minimum, implementation, verification and one
reviewable PR. Merge and publication require separate authorization.

- R1, R2: Add `PixelLineItem` and register it privately. Update `.qmltypes`, CMake
  and symbol-privacy expectations. Replace the line rectangles in `TableBlock.qml`,
  `HeadingSeparator.qml` and `BlockSequence.qml`.
  Check: new GUI test grabs a window at DPR 1, 1.25, 1.5, 2 and `contentY` 0, 0.25,
  0.5, 0.75, 1 with `pixelAligned` on/off, fractional origin, odd width, three-column
  table, wrapped cells and mixed row heights, plus heading separators and thematic
  breaks. Assert identical measured physical thickness for equal configured lines,
  continuity, and no doubled or missing intersections. Confirm the test fails on the
  1.18.0 baseline before the fix.
- R3: Tests for `borderWidth: 0`, `BorderMode` none/rows/header-only, disabled
  separators, and a deliberately thicker line.
- R4: Run the existing selection, navigation, table and geometry suites unchanged;
  add assertions that line items do not alter content height or cell coordinates.
- R5: Verify through an installed-consumer example that no host changes are needed
  and no public API, version or dependency changed.
- R6: Compare continuous-scroll frame cost before and after on a long table
  with the existing benchmark tooling; record cold and warm conditions separately.
- R7: Keep the report under `docs/` and the probe and representative captures under
  [evidence/](evidence/). Native acceptance is owner-run at native scale factors;
  record passed, failed or not-run separately. Update the specs index and
  `docs/styling.md` if line behavior is described there. Deliver as one PR with a
  development-heading release note.

Verification profile: Full (CMake source/test registration and the release source
allowlist are build/package inputs). Hosted verification follows the shared/static,
syntax OFF/ON, math-disabled, library-only, installed-consumer and benchmark smoke
coverage on both configured Qt targets in the development workflow. Local results
and unrun checks are recorded separately in verification.md.
