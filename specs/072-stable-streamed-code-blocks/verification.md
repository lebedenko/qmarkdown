# Actual verification

> Storage cleanup: generated artifacts linked below were removed from Git history.
> See the [retention policy](../../docs/artifact-storage.md); historical results remain, raw-data revalidation requires regenerated inputs.

## Scope and approval

The supplied plan was approved for implementation and verification on 2026-11-04 by the explicit request to implement it. Version 1.18.0 / QML 1.18 is unchanged. Consumer viewport-height configuration, merge, publication and system-package replacement are excluded.

## Focused results (2026-11-04)

Host Qt 6.12.0, shared build with KSyntaxHighlighting enabled:

- `cmake --build build-highlighted --parallel 2`: passed.
- `QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH=$PWD/build-highlighted/qml ctest --test-dir build-highlighted --output-on-failure -R 'qmarkdown-(syntax|syntax-view|codeblocks|view)$'`: 4/4 suites passed (45.74 seconds).
- The final asynchronous color assertion and delayed-provider identity case were also run directly: `qmarkdown-syntax-view-test colorsThemeAndSource streamedItemIdentity`, 4 test phases passed.
- `cmake --build build-highlighted --target all_qmllint --parallel 2`: passed, including updated private metadata. An existing unused import informational diagnostic remains in the import-only example.
- `python3 scripts/check.py`: passed, including 87 Python tests. `python3 scripts/check-docs.py` and `git diff --check`: passed.

The new regressions exercise both append-owned source and an actual growing QML binding, plain/richer viewports, overflow onset, following, scrolling up, resumption, horizontal retention, wrapping, line numbers, selection, Find, height changes, closure, outside appends, explicit flush, pending flush on disabling streaming, replacement, initial long-block position and absence of append-induced outer navigation. Syntax tests cover retained accepted spans, repeated delayed updates, stale completion/cancellation, empty completion, replacement, disable and stable text-item identity through failure fallback. Existing provider, backend-limit, selection and streaming tests remain part of the runtime suite.

Initial focused runs found a queued-follow/fence-closure race and transient zero-height layout during a plain-to-selectable transition. Both were corrected and the expanded cases passed. Initial broader runs also exposed an old expectation that unavailable highlighting used a plain text item; it now correctly expects the configured richer item with empty ranges. The color assertion now waits for rendered colors because initial layout can precede asynchronous ranges.

## Exact-Qt Core verification

Final Core runs with syntax-enabled coverage passed on 2026-11-04:

```sh
python3 scripts/run-ci.py --qt-role minimum --profile core --syntax
python3 scripts/run-ci.py --qt-role development --profile core --syntax
```

Both exact Qt targets passed shared syntax OFF and ON: 33/33 runtime checks and 32/32 installed-consumer checks per configuration, QML lint, cmark symbol privacy, relocated package checks and payload checks. The runner returned exit 0 and `PASS: core verification` on each target.

| Qt | Result | Local evidence directory | Verification time |
| --- | --- | --- | --- |
| 6.8.0 | Passed, Core + syntax | `build-ci/6.8.0/20261104T151539Z-9kb6f1pe` | 491.573 s |
| 6.12.0 | Passed, Core + syntax | `build-ci/6.12.0/20261104T151557Z-0z7szcs_` | 496.955 s |

Each evidence directory retains `result.json`, `environment.json`, `ci.log`, captured source and packaging artifacts. The final runtime/test source snapshots were compared byte-for-byte with the working tree and matched. Subsequent edits are verification/specification prose only; current documentation links and whitespace were checked separately. Build-only compiler caching was reused; these are correctness results, not performance measurements.

Initial sandbox attempts could not access `/var/run/docker.sock`; authorized unrestricted runs used the pinned Qt images. Earlier runs that exposed the closure race and asynchronous color assertion were superseded by these passing final runs.

## Rendered sequence and visual acceptance

The offscreen Qt 6.12.0 sequence was captured with `QMARKDOWN_STREAM_CAPTURE` while running `qmarkdown-codeblocks-test streamedViewport`:

- Before overflow (removed artifact: `rendered/01-before-overflow.png`)
- Following appended code (removed artifact: `rendered/02-following.png`)
- Paused above the bottom (removed artifact: `rendered/03-paused.png`)

The images were opened and inspected: the short block begins at line 1, the followed block shows its last lines, and the paused block keeps the earlier source rows and horizontal offset. The numerical tests verify the positions; these images are offscreen rendering evidence only. Native interactive visual acceptance has not been obtained and is not implied by these results.

## Delivery

This change groups implementation, tests, documentation and verification on `fix/stable-streamed-code-blocks` for one reviewable PR. No merge, publication or system package replacement has been performed.
