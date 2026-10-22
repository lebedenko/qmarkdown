# Actual verification

2026-10-22: **prototype gate failed; production integration stopped**.
No production renderer, public interface, dependency or version changes.

## Reproduction

```sh
cmake -S specs/065-padded-code-rendering/prototype -B /tmp/qmarkdown-065-prototype
cmake --build /tmp/qmarkdown-065-prototype --parallel 2
cd /tmp/qmarkdown-065-prototype
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software ./padded-code-gate
```

Host GCC 16.2.1, exact Qt 6.12.0, offscreen software, observed DPR 1.00.
Configure/build succeeded. Probe exited 1, reporting:

```text
Qt 6.12.0 DPR 1.00 different pixels 732 lost ink pixels 0
```

[Reference](evidence/reference.png) paints only italic `f`.
[Selection emission](evidence/selected.png) paints `fX`, including the transparent
unselected source interval. Both captures were visually inspected. This violates
R3's prohibition on duplicated hidden paragraph content: coalesced fragments
cannot safely compose the paragraph using this method on this required backend.
This records observed output, not an assertion about Qt internals or other backends.

The probe uses public Qt APIs only. It is standalone and excluded from production
CMake/tests. The initial exploratory probe changed formats without recreating
lines, invalidating the comparison; that probe was corrected before the recorded
run and is not qualification evidence.

Qt 6.8.0, native backend/DPR matrix, full shaping cases, scrolling identities,
interactions, texture retention, timing, memory and packaging were not run:
production integration is explicitly prohibited after this gate failure. No
speedup, retained-node or production acceptance claim is made. Historical Feature
064 evidence is unchanged. PR delivery and hosted checks remain pending.

## Later retained-texture experiment

The separately approved follow-up and its actual results are recorded in
[retained verification](retained-verification.md). Historical results above
remain unchanged.
