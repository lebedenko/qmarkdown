# 008: Font units and application typography

**Approval:** On 2026-10-08 the user explicitly requested implementation of the supplied “Support both font units and application-based defaults” plan. This approves the complete requirements, design, and mapped tasks below before implementation. Package 0.5.0 and QML module 0.5 remain unchanged.

R1. Keep QFont roles and accept whole-font assignments and QML pixelSize/pointSize edits with Qt semantics. No public unit enum or separate size API.
R2. Snapshot QGuiApplication::font() at construction. Body retains its size/unit and normal weight. Bold H1–H6 scale by 2, 1.75, 1.5, 1.25, 1.125, 1; points retain fractions, pixels round to a positive integer. Code blocks use body size/unit, system fixed family, normal weight. Inline code remains a family-only overlay inheriting its containing block.
R3. Roles remain independent; shared styles, notifications, replacement, reset and destruction retain their lifecycle. Reset restores construction defaults. Later application-font changes require host updates or a new style.
R4. Playground offers active px/pt units for every role, fractional points and integer pixels. Convert using preview-screen logical DPI (px = pt × DPI / 72), rounding pixels, without DPR. Preserve unrelated font fields/resolve masks. Reading/selecting inherited inline code does not set size; size/unit editing creates an override. Neutral/Reset use application defaults; Alternate retains explicit pixels.
R5. Native Text and private inline text agree on point sizing; screen/DPI changes invalidate private layout. Spacing, indents and rule thickness remain logical pixels.
R6. Document defaults, both units, approximate conversion, independent roles, inheritance and snapshots. Default wrapping/heights may change; explicit legacy pixels reproduce old typography. Supersede historical fixed defaults without deleting approvals. Keep package/import versions.
