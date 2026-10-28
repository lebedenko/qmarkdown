# Actual verification

## Reproduction and payload investigation

Baseline: QMarkdown 1.18.0, `b22d519`. Host Qt 6.12.0, offscreen backend.
The new real-mouse nested scrolling cases failed before the fix: natural-height
code emitted 5 outer requests and exhausted bounded code emitted 6 while the
pointer was away from outer edges. Both pass after removing boundary forwarding.
This reproduces a library defect, not the entire reported native workflow.

Initial partial-copy gesture cases passed before the fix (8 data rows): forward
and reverse single/multiline selections, mid-line/newline endpoints, spaces, tabs,
blank lines, CJK, emoji and combining marks; wrap and line-number combinations.
Literal expected substrings, public UTF-16 offsets, selectedText and Ctrl+C copyRequested agree. Expanded final cases
also select across wrapped long lines, replace the initial `alpha` identifier with
the same-length C++ keyword `while`, explicitly assert highlighted ranges when
available, and assert nonzero internal Y/unwrapped X offsets. A setup refinement
was required: initial queued source-layout restoration could reset offsets applied
before polish. The final fixture waits for layout and sets/asserts offsets before
each gesture. This was a test-setup issue, not a demonstrated payload defect.
No independent payload defect was found.
No projection, cursor geometry or clipboard sanitation changes were made.

The [manual fixture](fixture.md) mirrors the programmatic fixture, except that the
test also retains a trailing space on its long line. Mouse positions in the tests
come from layout cursor geometry; expected text and the prose projection prefix
length are specified independently. The code display drops its terminal newline;
projection registration retains the full source length by design.

## Executed checks

- Local focused build: `cmake --build build-shared --target
  qmarkdown-selection-test qmarkdown-codeblocks-test --parallel 2` (the final
  selection target was rebuilt after test refinements).
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build-shared -R
  'qmarkdown-(selection|codeblocks)$' --output-on-failure`: 2/2 passed, 28.12 s (final strengthened tests).
  Includes existing paragraph/cross-block selection, Copy shortcut, whole-code
  action, cancellation/release, viewport/source lifecycle and wheel/touch controls.
  New scrolling cases cover internal X/Y range, stationary endpoint updates,
  exhausted top/bottom and non-overflowing code away from outer edges, internal
  vertical priority at an outer edge, and outer movement once it is exhausted.
- `python3 scripts/check.py`: passed (documentation, configuration/evidence syntax,
  Qt-independent tooling tests).
- `python3 scripts/check-docs.py`: passed for current 1.18.0 and local links.
- `git diff --check` and final `git diff --cached --check`: passed.

Final exact-target Core plus syntax-enabled verification passed:

| Qt | Syntax backend | Runtime CTests | Installed consumers | QML lint / symbol privacy | Elapsed |
| --- | --- | --- | --- | --- | --- |
| 6.8.0 | OFF and ON | 29/29 each | 32/32 each | Passed | 448.5 s |
| 6.12.0 | OFF and ON | 29/29 each | 32/32 each | Passed | 453.4 s |

Commands:

- `python3 scripts/run-ci.py --qt-role minimum --profile core --syntax --artifact-dir build-ci/070-final-minimum`
- `python3 scripts/run-ci.py --qt-role development --profile core --syntax --artifact-dir build-ci/070-final-development`

Both retained `result.json` records report `qmarkdown-ci-v2`, profile `core`,
status `passed`, exit 0. Shipping source and tests match both captured snapshots.
Initial runs also passed in `build-ci/070-minimum` and `build-ci/070-development`;
final runs supersede them after the fixture/assertion refinements described above.
These Core checks do not constitute Full release-publication evidence.

## Native acceptance — pending

The affected md-viewer document/session, exact gestures, clipboard captures and
remote SSH/tmux session have not been supplied. The available automated harness
runs offscreen and does not own the clipboard. No native emitted/clipboard/local
paste/SSH-tmux comparison was performed; the reported unwanted local characters
and empty remote paste remain unresolved observations. Passing library tests
cannot establish a terminal, tmux, SSH or application cause.

On the affected environment, use the fixture above and preserve both partial and
whole-block controls. Record exact drag endpoints/direction, wrapping, line numbers,
internal offsets, QMarkdown revision, Qt version/backend, terminal, SSH and tmux
versions. Capture escaped payloads and UTF-8 bytes at copyRequested (partial),
codeBlockCopyRequested (whole block), host clipboard, local paste and remote paste.
Compare every stage against independently specified expected source substrings.
Native acceptance requires those comparisons and actual local and SSH/tmux paste;
it is not satisfied by this PR's automated checks.

## Delivery

Requirements, design and tasks were approved through the implementation request on
2026-10-29 before implementation. One PR contains the fix, tests, report and records.
Public APIs, clipboard ownership, dependencies and package versions are unchanged.
Merge and publication remain separately authorized actions.
