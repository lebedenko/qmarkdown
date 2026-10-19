# 059 — Design

Approved on 2026-10-19 by the user-supplied “Refresh the playground and README” implementation plan. Approval covers requirements, design, mapped tasks, implementation and verification on a local feature branch; no push, PR, merge or release.

Use one entries model of key/title/source objects. Tests select semantic keys. Sample changes load source only. Scale an unscaled-size wrapper around the Controls indicator from its top-left origin, and scale its advertised dimensions together. Style-owned anchors resolve inside the natural-size wrapper, avoiding offsets and clipping at 2×. Bind syntax control availability to the actual preview backend. Keep new controls in the bounded Style panel. Configure highlighted builds in build-highlighted with the matching QML import path.

All optional rendering toggles live in the bounded Style panel (180 logical pixels), leaving Find, selection and panel visibility in the toolbar. This preserves a usable splitter under Fusion, Basic and Material at 800×600.

Approved startup-default refinement (2026-10-19): set the shared playground checkbox initial values and host MarkdownStyle preset/overrides declaratively. Both demo tasks load this same UI. Keep Neutral/Reset actions and backend availability behavior intact. Verify startup options in both backend configurations; existing toggle/reset tests explicitly establish their baseline.
