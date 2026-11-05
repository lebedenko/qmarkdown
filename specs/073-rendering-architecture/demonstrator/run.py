#!/usr/bin/env python3
"""Serial native supervisor with stage deadlines and retained failure evidence."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import selectors
import signal
import subprocess
import time

HERE = Path(__file__).resolve().parent


def hashes(paths):
    return {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths if p.is_file()}


def supervise(command, folder, env, expected, inputs, deadlines=(20, 10, 10)):
    folder.mkdir(parents=True, exist_ok=False)
    before = hashes(inputs)
    (folder/"inputs-before.json").write_text(json.dumps(before, indent=2)+"\n")
    started = time.monotonic()
    deadline = started + deadlines[0]
    records, errors, partial = [], [], b""
    stage, timed_out = "startup", False
    with (folder/"stdout.jsonl").open("wb") as log, (folder/"stderr.log").open("wb") as err:
        process = subprocess.Popen(command, env=env, stdout=subprocess.PIPE, stderr=err, start_new_session=True)
        selector = selectors.DefaultSelector()
        selector.register(process.stdout, selectors.EVENT_READ)
        while selector.get_map():
            if time.monotonic() > deadline:
                timed_out = True
                os.killpg(process.pid, signal.SIGKILL)
                break
            for key, _ in selector.select(min(.1, max(0, deadline-time.monotonic()))):
                chunk = os.read(key.fileobj.fileno(), 65536)
                if not chunk:
                    selector.unregister(key.fileobj)
                    continue
                log.write(chunk); log.flush()
                partial += chunk
                while b"\n" in partial:
                    line, partial = partial.split(b"\n", 1)
                    try:
                        record = json.loads(line)
                        if not isinstance(record, dict) or not isinstance(record.get("stage"), str):
                            raise ValueError("missing stage")
                        records.append(record)
                        stage = record["stage"]
                        # Startup progress cannot indefinitely extend readiness.
                        if stage == "ready":
                            deadline = time.monotonic()+deadlines[1]
                        elif stage == "shutdown":
                            deadline = time.monotonic()+deadlines[2]
                        elif stage in ("capture", "replay"):
                            deadline = time.monotonic()+deadlines[1]
                    except (ValueError, UnicodeDecodeError) as exc:
                        errors.append(str(exc))
        selector.close()
        process.stdout.close()
        if partial:
            errors.append("truncated JSON record")
        try:
            code = process.wait(timeout=max(.01, deadline-time.monotonic()))
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            code = process.wait()
            timed_out = True
    after = hashes(inputs)
    ready = next((r for r in records if r["stage"] == "ready"), {})
    for key, value in expected.items():
        if ready.get(key) != value:
            errors.append(f"identity {key}: expected {value!r}, observed {ready.get(key)!r}")
    if before != after:
        errors.append("inputs changed during execution")
    if not records or records[-1].get("stage") != "exit":
        errors.append("missing exit record")
    if any(r.get("exact") is False for r in records):
        errors.append("replay mismatch")
    result = dict(command=command, exit=code, timeout=timed_out, lastStage=stage,
                  seconds=time.monotonic()-started, observed=ready, errors=errors,
                  passed=code == 0 and not timed_out and not errors, inputsAfter=after)
    (folder/"result.json").write_text(json.dumps(result, indent=2)+"\n")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--qt", required=True)
    parser.add_argument("--compositor-scale", type=float, required=True)
    parser.add_argument("--fixtures", default="padded,objects,document")
    parser.add_argument("--cases", default="combined")
    parser.add_argument("--backends", default="opengl,vulkan")
    parser.add_argument("--dprs", default="1,1.25,1.5,2")
    parser.add_argument("--close", action="store_true")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    manifest = HERE/"fixtures.json"
    data = json.loads(manifest.read_text())
    inputs = [args.executable.resolve(), *[HERE/name for name in ("fixtures.json","qt.cpp","CMakeLists.txt","stage.py","run.py")], *args.executable.resolve().parent.rglob("libQMarkdown.so*"), *map(Path, data["fonts"].values())]
    results = []
    for backend in args.backends.split(","):
        for renderer in ("distance", "curve"):
            for dpr in map(float, args.dprs.split(",")):
                for fixture in args.fixtures.split(","):
                    for case in args.cases.split(","):
                        folder = args.output/f"{backend}-{renderer}-{dpr}-{fixture}-{case}"
                        env = dict(os.environ, QT_QPA_PLATFORM="wayland", QSG_RHI_BACKEND=backend,
                                   QT_SCALE_FACTOR=str(dpr/args.compositor_scale), QSG_INFO="1")
                        env.pop("QT_QUICK_BACKEND", None)
                        command = [str(args.executable.resolve()),str(manifest),fixture,renderer,str(folder.resolve()),"--case="+case]
                        if args.close:
                            command.append("--close")
                        result = supervise(command,folder,env,
                                           dict(qt=args.qt, api={"opengl":3,"vulkan":5}[backend],platform="wayland",dpr=dpr),inputs)
                        results.append(result)
                        print(json.dumps({k:v for k,v in result.items() if k!="inputsAfter"}), flush=True)
    (args.output/"summary.json").write_text(json.dumps(results,indent=2)+"\n")
    return 0 if all(r["passed"] for r in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
