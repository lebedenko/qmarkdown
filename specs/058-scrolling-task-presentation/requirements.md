# 058 — Requirements

Approved on 2026-10-19 through the user-supplied implementation plan. Approval covers requirements, design, mapped tasks, implementation, verification and one PR; no merge or release.

R1: Exclusively consume internal wheel movement; hand off outward boundary and non-overflow input. Preserve native phased filtering, horizontal/Shift input, touch and selection.

R2: Public taskIndicatorsReplaceBullets defaults false; unordered tasks replace bullets recursively, ordinary and ordered markers remain.

R3: Measure custom artwork; preserve eight-pixel text gap, first-line centering and complete band reservation, two-pixel indicator clearance. Clip at insufficient widths, retaining one content pixel.

R4: Preserve source, state, selection/copy, Find/navigation, links and inert checkbox accessibility. Add docs and playground comparison.

R5: Preserve 0b22f44, versions, sibling viewer and system packages. Deliver one PR without merge or release.
