# Coordinated desktop-scale study

Isolated implementation of the [approved protocol](../../scale-study-protocol.md).
Historical adapters and archives are read-only inputs. `instrument.py` generates
viewer overlays in a new build directory, retaining layout/preparation bodies and
linking existing verified libraries. No shipping source or public API changes.

Preparation (two compiler workers, no graphical samples):

```sh
python3 specs/073-rendering-architecture/demonstrator/scale-study/prepare.py build-ci/073-scale-study-final
python3 specs/073-rendering-architecture/demonstrator/scale-study/preflight.py build-ci/073-scale-study-final
python3 -m unittest discover -s specs/073-rendering-architecture/demonstrator/scale-study -p 'test_*.py'
python3 specs/073-rendering-architecture/demonstrator/scale-study/study.py freeze build-ci/073-scale-study-final build-ci/073-scale-study-run
```

After the owner sets each requested scale and confirms readiness, the coordinator
records a read-back and runs that frozen set. Example for the first scale:

```sh
python3 specs/073-rendering-architecture/demonstrator/scale-study/study.py confirm build-ci/073-scale-study-run 1 fresh-1
python3 specs/073-rendering-architecture/demonstrator/scale-study/study.py fresh build-ci/073-scale-study-run 1
```

Repeat at 1.25, 1.5 and 2, validating each set before advancing. `live` starts the
persistent pass after all four fresh sets. Its JSON waiting messages identify the
next `live-N` confirmation label. The owner changes DP-1; `confirm` verifies it and
writes an immutable handoff file. Windows keep processing compositor events while
scripted checks pause. Human waits do not consume automated deadlines. No process
is replaced, including after an interrupted invocation.

Results and all Wayland traces remain under the frozen run directory. `report.py`
validates captures, reports every cell and archives/read-backs collected evidence.
Physical comparability requires a separate review of recorded mappings; a successful
assertion is not evidence of equal physical font sizing or owner visual acceptance.
