# Code-block partial selection moves the page and produces incorrect clipboard text

## Affected baseline

Reported in md-viewer using pinned QMarkdown **1.18.0**, revision
`b22d5196fa5ee94ce6f36dada45bc61c9fa6106d`.
The original source inspection used a QMarkdown checkout at this revision. Exact native runtime, Qt/platform backend,
local terminal, SSH client, remote terminal and tmux versions remain unconfirmed.

## User-reported reproduction and observations

1. Open a Markdown document containing a fenced code block in md-viewer.
2. Drag-select part of the code.
3. Press Ctrl+C.
4. Paste into a regular local application.
5. Repeat the copy and paste into a tmux session over SSH.

**Actual behavior:** During partial code selection, the page moves and selection becomes difficult
to control. Local paste includes unwanted text or characters. Pasting into remote tmux produces
nothing. Exact unwanted characters and clipboard bytes have not been captured.

**Expected behavior:** Partial selection follows the intended code characters precisely. Scrolling
occurs only when appropriate for the active selection and viewport boundaries. Clipboard text
matches the selected code, preserving its indentation, tabs, newlines and Unicode.

**Working controls:** Paragraph selection followed by Ctrl+C works. The whole-block “Copy code”
action works, including pasting into the same remote tmux workflow.

These are user-reported observations, not a native reproduction performed for this report.
The original document, exact drag coordinates and a minimized failing fixture remain to be captured.

## Source inspection and ownership

The application owns forwarding and the operating-system clipboard write. QMarkdown owns selection
geometry, hit testing and the text payloads. Source inspection shows:

- md-viewer's `qml/ReaderPane.qml` forwards both `onCopyRequested` and
  `onCodeBlockCopyRequested` through the same `root.copyRequested(text)` signal.
  `qml/Main.qml` forwards that signal to `Clipboard.copyText(text)`, and
  `src/clipboard.cpp` calls `QGuiApplication::clipboard()->setText(text)` without transforming it.
- QMarkdown's [MarkdownView.qml](../src/QMarkdown/MarkdownView.qml) exposes separate selection-copy
  and code-block-copy signals. In [viewstate.cpp](../src/QMarkdown/private/viewstate.cpp),
  `ViewState::requestCopy()` emits `selectedText()` for a nonempty enabled selection.
  [viewstate.h](../src/QMarkdown/private/viewstate.h) implements `selectedText()` as a slice of
  `m_projection` between the selection endpoints.
- The whole-block action in [CodeActionHost.qml](../src/QMarkdown/private/CodeActionHost.qml)
  captures `codeText` and emits that text through its copy request.

Thus both actions share identical app forwarding and `QClipboard::setText`; their different
payloads originate in QMarkdown. This establishes the ownership boundary, not the cause of every
reported paste symptom. Application paths above are relative to the md-viewer repository;
linked QMarkdown paths are relative to this report.

## Original investigation leads, not confirmed causes

### Page movement during code selection

`ViewState::scrollSelection()` in [viewstate.cpp](../src/QMarkdown/private/viewstate.cpp)
tries the nested `codeSourceViewport` first. Its `scrollAxis` helper sets `boundaryDelta` when
vertical edge scrolling requests movement but `contentY` cannot change. When internal vertical
scrolling does not consume movement, it emits `selectionScrollRequested`, preferring that
`boundaryDelta` over the outer viewport's own edge-derived delta.

Consequently, code-edge scrolling can be forwarded to the outer reader even when the pointer is
not near an outer viewport edge. md-viewer's `ReaderPane.qml` handles the public signal by
scrolling its viewport. This is a source-level lead for the reported page movement; it has not
been confirmed by native reproduction or instrumented gesture tracing.

### Incorrect partial-copy content

Investigate the rendered-text selection projection and its mapping to code coordinates:
`m_projection` construction, registered offsets/lengths, `selectionItemAt()`,
`selectionPosition()`, clipping, coordinate conversion, and `FormattedText::cursorAt()`.
The selected projection slice may diverge from the intended code range if endpoints or mappings
are wrong. No specific projection or hit-testing defect has been established.

### SSH/tmux paste

This symptom remains unresolved pending exact clipboard-payload comparison. Whole-block copy
working through the same application clipboard path is a useful control, but does not prove that
SSH, tmux, the terminal, or selection content is responsible. Capture both payloads before
attributing the failure or proposing a terminal-specific workaround.

## Originally proposed regression checks

These checks are proposed and have not been executed for this report:

- Select partial single-line and multiline code ranges in both drag directions; verify the
  highlighted endpoints and exact copied substring, including newline boundaries.
- Cover leading indentation, literal tabs, blank lines, Unicode (including CJK, emoji and combining
  characters), and selections starting or ending within a line.
- Exercise horizontally and vertically scrollable code blocks inside the outer document viewport.
  Check internal edge scrolling, exhausted internal ranges, outer edges, and a pointer away from
  outer edges. Confirm page movement follows the intended nested scrolling policy.
- Compare the public selection text, emitted selection-copy payload and resulting clipboard text
  for exact equality. Check whole-block copy against the complete code text as a separate control.
- On the affected native environment, record escaped text and UTF-8 bytes for both copy actions,
  then compare local paste and remote SSH/tmux paste. Record runtime/backend and terminal/tmux
  versions, relevant paste settings, document and gesture details alongside the captures.
- Retain paragraph-selection and whole-block-copy controls while testing partial selections.

## Original documentation-only verification and boundaries

This report was authorized as documentation only. Source references and the pinned baseline were
checked against the local repositories; Markdown links and whitespace were checked.
Native reproduction and clipboard-byte capture have **not** been performed. No runtime regression
checks or hosted checks were run for this report. No implementation changes, dependency changes,
workarounds or commits are part of this work.

## Implementation investigation — 2026-10-29

The user approved the supplied implementation plan; scope and actual verification
are recorded under [070](../specs/070-code-selection-copy/requirements.md).
The original report and its unconfirmed native observations above are preserved.

Real offscreen mouse gestures reproduce outer scroll requests at both exhausted
bounded code and non-overflowing code edges positioned away from outer viewport
edges. The new `nestedCodeSelectionScrolling` cases fail against the original
implementation (5 and 6 unexpected requests respectively). This confirms the
boundary-forwarding defect in the library, without establishing the cause of all
native paste symptoms.

The fix removes code-boundary delta forwarding. Internal scrolling remains clamped
on both axes; successful internal vertical movement takes priority. Otherwise,
outer requests derive exclusively from the outer viewport's existing edge zone.
The timer continues updating the endpoint under a stationary pointer. Wheel and
touch paths, host clipboard ownership and public interfaces are unchanged.

Payload tracing follows presentation text into `m_projection`, code projection
offset/length through `SelectableText` registration, clipping and `mapFromItem`
coordinates in `selectionPosition`, and grapheme cursor cells in
`FormattedText::cursorAt`. Code display intentionally omits its final newline,
while projection/registration retain it. The gesture tests compare independently
specified literal substrings and normalized UTF-16 offsets with selectedText and
Ctrl+C copyRequested payloads. Initial cases pass against the original code across
wrapping and line numbers. Final tests additionally assert nonzero internal offsets
and highlighted ranges when the backend is available. No independent payload defect
has been demonstrated; no projection, geometry or clipboard sanitization changes
are justified by this evidence.

See [actual verification](../specs/070-code-selection-copy/verification.md) for final
checks and limitations. Native emitted/clipboard/local-paste/SSH-tmux byte comparison
remains pending. Passing library tests does not attribute the reported remote
paste failure to the terminal, tmux, SSH or the application.
