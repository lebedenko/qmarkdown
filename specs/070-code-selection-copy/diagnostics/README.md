# Feature 070 native acceptance

This harness changes only fresh `/tmp` copies. Run from the QMarkdown checkout:

```sh
python3 specs/070-code-selection-copy/diagnostics/prepare.py \
  --viewer /home/andrii/Projects/pet/md-viewer --root /tmp/qmarkdown-070-native
python3 -m unittest discover -s specs/070-code-selection-copy/diagnostics -p 'test_*.py'
```

The root must not exist. Both revisions use host Qt, Release/shared, math ON,
highlighting ON and installed packages. The viewer snapshot includes current
uncommitted source edits; `viewer-snapshot.json` hashes its pre-instrumentation
files. `manifest.json` records original revision/status and Qt. No dependency
acquisition, package installation, pin updates or production edits occur.
`ldd.txt` records the actual resolved library and startup records list QML imports.
For native import resolution evidence, launch with `QML_IMPORT_TRACE=1` and save
stderr; confirm QMarkdown loads from that revision's prefix. Do not substitute a
system provider. The startup `qml-resource` record hashes the embedded MarkdownView.qml source;
compare it with that revision’s source file. The installed qmldir prefers
`:/qt/qml/QMarkdown/`; the linked library can supply registration without loading
the optional plugin. Each app has its own seeded config file; record all settings.

## Owner session

Keep the launched app running during every paste (clipboard ownership matters).
Use one app at a time and retain stderr:

```sh
QML_IMPORT_TRACE=1 /tmp/qmarkdown-070-native/released/launch.sh \
  "$PWD/specs/070-code-selection-copy/fixture.md" 2>/tmp/070-released-native.log
QML_IMPORT_TRACE=1 /tmp/qmarkdown-070-native/fixed/launch.sh \
  "$PWD/specs/070-code-selection-copy/fixture.md" 2>/tmp/070-fixed-native.log
```

Before gestures record OS/session, terminal name/version, `ssh -V`, local and
remote `tmux -V` (including whether tmux is absent), `$TERM`, Qt/backend (startup
record), viewer revision/status, library revision, viewport pixels, zoom/font,
wrap, line numbers, highlighting and code height limit. Record internal X/Y
positions through visible scrollbar position and screenshot/owner observation;
no private renderer tree inspection is used. Record exact numeric positions only
if available through the host's public controls; otherwise label them estimates.

Before each copy, record a case ID and the intended literal expected substring.
The checked-in `expected/P1.txt` through `P5.txt`, `C1.txt` and `C2.txt` contain
predefined exact UTF-8 payloads (no added newline except where explicitly selected).
For different endpoints, write a new expectation file before the gesture. Do not derive expectations from emitted or clipboard text.
`trace.jsonl` appends actions numbered per process (PID + action uniquely identify
an action). Signal records (`partial`/`whole-code`), selectedText, writer and
immediate readback share the action ID. Offsets are public UTF-16 selection offsets.
JSON escapes control characters; hex/byte length/SHA-256 describe UTF-8 bytes.
A signal record precedes host forwarding; selectedText is sampled at that point.
Whole-code selectedText/offsets describe any existing selection, not its payload.

Paste into a local receiver, then the affected SSH/tmux receiver, without copying
anything else. Transfer `capture.py` to the remote machine by your usual method;
it needs only Python 3 and a POSIX terminal. Use a new output filename each time:

```sh
python3 specs/070-code-selection-copy/diagnostics/capture.py /tmp/local-case01.json --action PID:1
# Remote, including inside the affected tmux pane:
python3 /tmp/capture.py /tmp/remote-case01.json --action PID:1
```

Paste once, then press Ctrl-]. Ctrl-C cancels. Timeout is 120 seconds without input.
Input is never evaluated. The receiver restores termios in `finally` on normal
exit, exceptions and Ctrl-C; SIGKILL/power loss cannot run cleanup. Bracketed paste
is requested during capture and disabled at exit. Wire bytes include bracketed
paste markers; payload removes one exact enclosing pair only. Raw mode disables
terminal CR/LF conversion; CR, LF, extra framing and invalid UTF-8 remain in hex.
If multiple pastes/markers occur, repeat into a fresh file; never trim results.
Keep terminal framing distinct from clipboard payload. Record transport changes
seen between local and remote captures instead of attributing them to QMarkdown.

Compare expected bytes → signal → writer → readback → local payload → remote
payload using hex (text previews alone cannot show all differences):

```sh
python3 specs/070-code-selection-copy/diagnostics/compare.py expected.txt \
  /tmp/qmarkdown-070-native/fixed/trace.jsonl --pid PID --action 1 \
  --local /tmp/local-case01.json --remote /tmp/remote-case01.json
```

The comparator requires one complete action and both paste captures. It reports
the earliest mismatching payload and rejects missing or misidentified captures. Use the first
divergent stage to target investigation. Include expected file, trace lines,
local/remote JSON, gestures and settings with each result.

## Matrix (run on both revisions)

Start with [fixture](../fixture.md). Define endpoints literally before dragging:

| Case | Literal expected text (JSON notation) | Gesture |
| --- | --- | --- |
| P1 | `"while"` | forward then reverse, mid-line |
| P2 | `"    while\tbeta\n\n  中文 😀 é tail\nnext"` | forward then reverse, indentation/blank line/Unicode |
| P3 | `"beta\n"` | newline endpoint |
| P4 | `"中文 😀 é"` | grapheme boundaries, both directions |
| P5 | first eight `long_identifier` words with intervening spaces (`expected/P5.txt`) | wrapped drag |
| C1 | `"Before prose"` | paragraph control |
| C2 | complete fenced source including terminal newline (`expected/C2.txt`) | header Copy control |

Repeat partial cases with wrap OFF/ON × line numbers OFF/ON, highlighting enabled.
Include nonzero internal vertical offsets, and horizontal offsets with wrap OFF;
record exact endpoints if those gestures select different substrings. Keep the
same text expectations across direction/configuration pairs where possible.
Retain tabs and the combining acute accent as present in the fixture.

| Scroll case | Pointer / range | Fixed acceptance |
| --- | --- | --- |
| S1 | internal range available, away from outer edge | internal scroll first |
| S2 | internal range available, at outer top/bottom edge | internal Y takes priority |
| S3 | exhausted internal top/bottom, away from outer edge | no outer movement |
| S4 | exhausted internal top/bottom, in outer edge zone | outer movement |
| S5 | natural-height code, away from outer edge | no outer movement |
| S6 | natural-height code, in outer edge zone | outer movement |
| S7 | hold pointer stationary during scrolling | endpoint follows newly exposed text |

Exercise both top/bottom and internal horizontal range. Record observations or
screen recording and selection endpoint change. Baseline S3/S5 may fail by design;
record actual outcomes. Native acceptance cannot be inferred from offscreen tests.

Every case/configuration starts **not run**. Mark passed only with required
captures/observations, failed with earliest divergence, or not run with reason.
If this fixture does not reproduce unwanted characters/empty remote paste, obtain
the minimized original document, exact gesture and affected environment. Do not
close the original report based solely on this fixture. No sanitization, trimming,
terminal setting workaround, merge or publication is authorized by this runbook.
