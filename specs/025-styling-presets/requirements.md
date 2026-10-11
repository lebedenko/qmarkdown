# Requirements

Approved on 2026-10-11 through the explicit instruction to implement the supplied v1.1.0 plan.

- R1: Package 1.1.0/module 1.1, retaining 1.0 imports and revisioning additions to 1.1; no dependencies/parser changes.
- R2: MarkdownStyle.preset: Legacy (default), PolishedLight/Dark, MinimalLight/Dark. Preserve v1.0 defaults, fonts, host ownership and resources.
- R3: Permanent style-owned headings.h1–h6 objects (font, color, separatorVisible/Color/Thickness/Spacing), inlineCode (font, foreground, backgroundColor, cornerRadius), codeBlock (font, color, backgroundColor, borderColor/Width, cornerRadius, horizontalPadding/verticalPadding). Flat aliases share state and notify both paths.
- R4: Track explicit whole-property overrides including default assignments, false, zero and transparency regardless of declaration order. Preserve font resolve masks. Individual RESET clears overrides; resetOverrides restores preset values without detaching bindings. Internal restoreDefaults restores construction snapshots/Legacy. Document binding via one spelling.
- R5: Fixed light/dark palettes: text #202020/#e6e6e6, links #0066cc/#8db9f2, separators/borders/breaks #d0d7de/#454b54, quote rules #808080/#a0a0a0, polished code backgrounds #eff1f3/#30343b. No document background/theme subscription.
- R6: Legacy new decorations disabled/transparent with zero metrics. Minimal preserves construction typography, spacing/container metrics; decorations disabled. Polished preserves construction typography/container metrics, block spacing 12, H1/H2 separators thickness 1/spacing 8, inline radius 4, code padding 12/border 1/radius 6.
- R7: Inline foreground inherits containing text unless overridden; links take precedence. Heading separators once per entire heading including image row transitions. Inline backgrounds use existing visual run geometry without affecting wrapping, line height, offsets or hit areas; padding/borders deferred. Native literal code surfaces preserve wrapping/empty blocks.
- R8: New metrics clamp to finite nonnegative geometry; nonfinite renders zero. Positive widths retain at least one text pixel.
- R9: Playground controls, fixtures, documentation, 1.0 and 1.1 consumers and packaging checks. Verification covers state/aliases/fonts/resets/bindings/lifetimes, heading/code/inline edge cases, all presets at normal/fractional scaling without exact cross-version glyph comparison, shared/static suites, strict 652-example corpus, lint, installed/relocated consumers and both configured CI targets.

Selection/copy, GFM, streaming, highlighting, renderer extensions and arbitrary style inheritance remain deferred. Publication is separate.
