# QMarkdown 1.6.0

QMarkdown 1.6.0 / QML 1.6 supports all five GFM extensions on a CommonMark 0.31.2 core: task lists, strikethrough, extended autolinks, tables and tag filtering. Every individual switch and the aggregate `gfmEnabled` switch defaults off. Aggregate enablement preserves individual settings and disabling it restores them. Exact GFM 0.29-gfm conformance is not claimed.

Tables use native Qt Quick components with equal-width columns, wrapped inline content, alignment and bold headers. Grouped table styling controls foreground, surfaces, borders and padding. Links and images retain host navigation and resource policies; hosts own scrolling and visual identity. Tag filtering retains private filtered HTML semantics while native presentation preserves literal source spelling.

Inline code now reserves **4/2 logical pixels of horizontal/vertical padding in every preset**, including Legacy and Minimal. This intentionally changes wrapping and line height even with transparent backgrounds. Each wrapped fragment receives padding; Qt bidi/grapheme shaping and semantic source ranges are preserved, and linked padding activates its enclosing link. Set both `style.inlineCode.horizontalPadding` and `style.inlineCode.verticalPadding` to zero for previous spacing.

Historical QML 1.0–1.5 imports and compatible CMake 1.0–1.6 requests remain supported. EXACT 1.6.0 selects this package; exact historical versions require their historical packages. Future 1.7/2.0 requests are rejected. Supported scope remains Linux x86_64, Qt >=6.8, C++17 and CMake >=3.21.

Release assets are the Ubuntu 24.04 x86_64 archive built with Qt 6.8.0 and the Arch `1.6.0-1` package, each with a SHA-256 file. Follow the [installation instructions](release-install.md). Compact release evidence links the exact commit, workflow verification and independently downloaded-package acceptance. Local candidate archives are not final release artifacts.

Offscreen checks cover normal/fractional application scale and font changes. A physical live display/DPI switch remains untested in the headless verification environment. See [feature verification](../specs/038-gfm-completion-inline-code-padding/verification.md) for local results and limitations.
