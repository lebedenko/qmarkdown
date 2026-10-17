# Verification — Text rendering sharpness

Actual results on 2026-10-16. Planned checks are in [tasks](tasks.md) and [design](design.md).

## Rendered regression

`selectableTextRetainsEdgeContrast` captures the same MarkdownView with selection disabled/enabled using `QQuickWindow::grabWindow()`. The resolved fonts match. Both captures have identical colors, width and placement. Summed squared horizontal/vertical luminance differences must retain at least 80% of the ordinary-text reference.

| SDK / offscreen software / 125% | Before correction | After correction |
| --- | --- | --- |
| Qt 6.8.0 | 0.551 — fails | 0.967 — passes |
| Qt 6.12.0 | 0.549 — fails | 0.953 — passes |

The actual fixture differs from the earlier exploratory text, so its measured ratios differ from the illustrative 44%/94% in the supplied plan. It independently reproduces the regression and passes the unchanged 80% criterion after the fix. Qt 6.12.0 also passes at 100%, 150% and 200% (ratios 1.000, 1.008 and 1.000).

## Captures and behavior

- `sharpnessPresentationFixtures` captures headings, bold/italic text, inline code, links, tables and C++ fenced code. It waits for syntax foreground ranges when the backend is available. All four scales (100%, 125%, 150%, 200%) passed, with item placement `(16.3, 16.3)` and scroll offsets `0`, `80.3`, `160.6`.
- Captures: `build-shared/tests/presentation/sharpness/offscreen/{1,1.25,1.5,2}/`. Inspection found sharp, antialiased glyph edges, continuous decorations and expected viewport clipping.
- Focused host CTest: styling and selection at normal/fractional scale plus code blocks **5/5 passed**. Reading and keyboard navigation **2/2 passed**. Existing geometry, hit-testing and selection checks are unchanged.

## Native Wayland acceptance

- Installed the candidate into `build-054-candidate`, then built the sibling Markdown viewer into this repository's `build-054-native-viewer` against that prefix. No sibling sources or system packages were modified. `/proc/1107257/maps` confirmed `build-054-candidate/lib/libQMarkdown.so.1.12.0` was loaded.
- Opened this repository's `README.md`, as explicitly permitted by the user's clarification. Hyprland reported native Wayland (`xwayland: false`) on eDP-1 at scale 1.25. The user inspected scrolling and replied **“Looks acceptable”** to the sharpness/jaggedness/shimmer acceptance question.
- A local diagnostic harness (`build-054-native-check-src`, executable in `build-054-native-check`) loaded the existing playground QML against the same installed candidate. At settled DPR 1.25, selection via Ctrl+A, Ctrl+F, query entry, next-result navigation, Escape and actual wheel delivery all passed. It captured README, selected text, Find highlights and eight fractional scroll positions. The backend was native Wayland/OpenGL, Mesa Intel Graphics (RPL-S), Mesa 26.2.4.
- Initial native invocations of the offscreen regression were invalidated by startup window/DPR negotiation: the two grab sizes differed. These are not counted as rendering failures or successful acceptance. A diagnostic copy with a 600 ms startup settling interval and a one-time floating-window launch passed at native 125% (contrast ratio 1.159); settled formatted-content captures were also inspected. The committed regression continues to target the deterministic offscreen CTest environment.
- Local logs and native captures are retained in `build-054-evidence`; diagnostic harnesses are ignored build artifacts. The candidate viewer is available for further inspection.

## Packaging and final review

- `python3 scripts/ci-toolchain/qt_versions.py --sync-host qmake6` confirmed development Qt 6.12.0; the pinned targets were unchanged.
- Minimum matrix: `python3 scripts/run-ci.py --qt-role minimum --artifact-dir build-ci-054-minimum` **passed (exit 0)**. Shared/static, syntax backend OFF/ON, full CTest, QML lint, installed/relocated consumers and benchmark smoke passed.
- Development matrix: `python3 scripts/run-ci.py --qt-role development --artifact-dir build-ci-054-development` **passed (exit 0)** with the same complete coverage. Each SDK passed 31/31 CTest entries in all four shared/static × backend OFF/ON configurations (248 passing CTest entries across both SDKs), plus consumer and packaging checks.
- Both captured matrix source trees match the final renderer and styling-test files byte for byte. Subsequent edits only record documentation evidence.
- `python3 scripts/check-docs.py` and `git diff --check` passed. The production diff is limited to disabling smoothing beside the preserved antialiasing setting; there are no layout, interface, dependency or minimum-Qt changes.
- No system-package replacement, merge, tag or release publication was performed.
