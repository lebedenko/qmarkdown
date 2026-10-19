# 060 — Design

Approved with the supplied implementation plan on 2026-10-19; see requirements.md.

Extend existing TableStyle value/override machinery. Invalid stored alternate color represents inheritance; valid transparent colors remain explicit. Forward body background changes while inherited. Revision new properties and signals at 1.14.

Keep equal-width Rectangle cell backgrounds and existing content loaders. Paint horizontal strips once across the row; paint vertical Grid segments only between horizontal strips, so intersections have one translucent contribution. Decorations are input-free Items inside row bounds. None removes border insets; other modes retain the existing inset metric. Layout.metric sanitizes widths before clamping to available column width.

Keep host palette ownership in the existing Style panel and preserve pending playground history. Development notes sit outside published-release markers.

Follow-up approval on 2026-10-19: preserve the grouped API and document Qt’s lack of runtime revision enforcement for grouped members. Verify 1.14 metadata and retain an explicit expected-failure probe for older-import rejection. Strict runtime rejection is excluded from the approved compatibility contract.
