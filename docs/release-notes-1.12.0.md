# QMarkdown 1.12.0

<!-- current-release:start -->
QMarkdown 1.12.0 / QML 1.12 adds language-aware headers, view-wide code wrapping,
bounded source viewports, internal scrolling and optional source-line numbers.
See the [code-block guide](code-blocks.md) for the full API and host contracts.

Library defaults retain wrapped natural-height source with line numbers off.
Hosts can enable nowrap, line numbers and a height limit based on their actual
reader viewport. Headers remain stationary above the source viewport. Numbers
use native text baselines and remain outside copy, Find and selection payloads.
The resettable line-number color follows the effective code foreground at 60%
alpha. Code layout settings work with highlighting OFF or ON.

Host action components may declare language, canonical language ID, friendly
language name, full info string and fence origin. Legacy headers retain their
existing property contract. Copy requests include the complete parser-produced
source. Custom fenced visuals retain precedence and their own layout; Find or
selection reveals ordinary source. The playground demonstrates original language
badges, an accessible copy icon with feedback, and live code layout controls.

Internal source scrollbars have a replaceable host component and a minimal Qt
Quick fallback. QMarkdown has no Qt Quick Controls dependency. Hosts continue to
own clipboard writes, outer reader navigation, header composition and visual
identity. Optional KSyntaxHighlighting >=6.10.0 remains a private dependency:
source builds default it OFF; Arch enables it. See the [syntax guide](syntax-highlighting.md).

Historical QML 1.0–1.11 and unversioned imports remain supported. Compatible
CMake 1.0–1.12 requests and EXACT 1.12.0 select this package; exact earlier versions
require historical packages. Requests newer than 1.12.0 and 2.x are rejected.
Linux amd64 requires Qt >=6.8, C++17 and CMake >=3.21. Configured verification
targets are Qt 6.8.0 and 6.12.0. Native implementation classes and QMarkdown.Private
remain private, with no public C++ ABI promise. Streaming and bundled Mermaid/math
engines remain deferred.

Download [1.12.0 release assets](https://github.com/lebedenko/qmarkdown/releases/tag/v1.12.0):
`qmarkdown-1.12.0.tar.gz` and `qmarkdown-1.12.0-1-x86_64.pkg.tar.zst`, each with a
`.sha256` companion. See [installation commands](release-install.md) and
[Arch installation](arch-install.md). Historical [1.11.0 notes](release-notes-1.11.0.md)
retain that release's behavior. Actual verification is recorded separately in
[Feature 053 verification](../specs/053-code-block-line-numbers/verification.md).
<!-- current-release:end -->
