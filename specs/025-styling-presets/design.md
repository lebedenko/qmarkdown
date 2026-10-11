# Design

Approved on 2026-10-11 through the explicit instruction to implement the supplied v1.1.0 plan.

Sequence: style state, heading/code surfaces, inline decoration, playground, fixtures/documentation/packaging. Extend native Qt Quick rendering and existing whole-font resolution. Group objects retain lifetime with the style; flat and grouped paths share override-aware state. Preset application changes untouched fields only, and reset methods update state without overwriting QML bindings. Render decorations independently of text layout; separators belong to complete heading containers. Reuse visual line/run geometry for inline backgrounds; no parser or resource changes.

CI preparation (024) precedes final styling verification. Exact SDK provisioning failure blocks completion; no SDK substitution. Implementation detail and test mappings will be recorded as each approved slice is implemented.

## Implemented state design

Typed permanent QObject groups forward to shared style-owned values. A single override set records all explicit setters, before equality checks, including whole QFont values and resolve masks. Flat/grouped aliases use the same keys and emit both notifications when effective values change. Preset application writes native values without removing QML bindings; individual RESET removes one override and reapplies selected-preset defaults. Native preset writes are distinguished from explicit writes so signal-driven host edits still become overrides.

Construction font snapshots seed immutable Legacy defaults. Non-Legacy palettes include separator/border colors even when decoration visibility/width is disabled. Visual separators remain attached to a complete block; image segment children suppress their own separators. Code uses a Rectangle containing literal Text with clamped padding. Inline backgrounds merge adjacent visual character cells per line, following the same cursor geometry as link hit testing, and render before the existing QTextLayout draw. A monotonic interval cursor keeps code-cell preparation linear before per-line visual sorting. No text padding, parser or resource changes.

The expanded playground uses a bounded scroll area for style controls, preserving editor/preview space at the existing minimum window size. Installed consumers retain the 1.0 source and add a separately selectable 1.1 styling source for shared/static, relocated and plugin-only checks. Presentation artifacts are generated in build directories at scaling 1 and 1.25; checks assert palette/geometry rather than identical glyph pixels across SDKs.
