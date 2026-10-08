#!/usr/bin/env python3
"""Run the GitHub Qt verification commands in their pinned Ubuntu container."""

import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import uuid

IMAGE = "ubuntu:24.04@sha256:f610ab94648195aa356059f5b41d6085c9d4d903c072430cdd1af7bdb646106b"
QT_VERSIONS = ("6.8.0", "6.11.3")
TIMEOUT_SECONDS = 35 * 60
SOURCE = Path(__file__).resolve().parents[1]


def snapshot(source, destination):
    """Copy current tracked and nonignored files, without machine-private files."""
    files = subprocess.check_output(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"],
        cwd=source, timeout=10,
    ).split(b"\0")
    excluded = {".git", ".aws", ".codex", ".agents", ".ssh"}
    for entry in files:
        if not entry:
            continue
        relative = Path(os.fsdecode(entry))
        if (excluded.intersection(relative.parts)
                or relative.name.startswith(".env")
                or relative.suffix in {".pem", ".key"}):
            continue
        original = source / relative
        if destination.parent in original.parents:
            continue  # Custom artifact directories inside the repository are not inputs.
        if not original.is_file() and not original.is_symlink():
            continue  # Respect tracked deletions in the working tree.
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(original, target, follow_symlinks=False)


def docker_command(source, artifacts, qt, name):
    return [
        "docker", "run", "--rm", "--init", "--name", name,
        "--platform", "linux/amd64",
        "--mount", f"type=bind,src={source},dst=/source,readonly",
        "--mount", f"type=bind,src={artifacts},dst=/artifacts",
        # The retained snapshot is also visible below the artifact mount.
        "--mount", f"type=bind,src={source},dst=/artifacts/source,readonly",
        "--workdir", "/tmp",
        "--env", f"CI_QT_VERSION={qt}",
        "--env", f"CI_UID={os.getuid()}",
        "--env", f"CI_GID={os.getgid()}",
        IMAGE, "timeout", "--signal=TERM", "--kill-after=30s", "35m",
        "bash", "/source/scripts/ci-container.sh",
    ]


def stop_container(name, process):
    # Only stop the container created by this invocation, including on Ctrl-C.
    try:
        subprocess.run(["docker", "stop", "--time", "10", name],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=15)
    except (OSError, subprocess.TimeoutExpired):
        pass
    if process.poll() is None:
        process.terminate()
        try:
            process.wait(timeout=10)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--qt", choices=QT_VERSIONS, required=True)
    parser.add_argument("--artifact-dir", type=Path,
                        help="New or empty directory; defaults to build-ci/<Qt>/<unique run>")
    args = parser.parse_args()
    if sys.platform != "linux":
        parser.error("local CI currently requires a Linux host with Docker")
    if not shutil.which("docker"):
        parser.error("Docker is required; install it and enable access to its daemon")
    if args.artifact_dir:
        work = args.artifact_dir.resolve()
        work.mkdir(parents=True, exist_ok=True)
        if any(work.iterdir()):
            parser.error("--artifact-dir must be empty; previous run evidence is never overwritten")
    else:
        parent = SOURCE / "build-ci" / args.qt
        parent.mkdir(parents=True, exist_ok=True)
        work = Path(tempfile.mkdtemp(prefix=datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ-"), dir=parent))
    print(f"CI image: {IMAGE}\nQt: {args.qt}\nArtifacts: {work}", flush=True)
    name = "qmarkdown-ci-" + uuid.uuid4().hex
    command = docker_command(work / "source", work, args.qt, name)
    (work / "environment.json").write_text(json.dumps({
        "image": IMAGE, "platform": "linux/amd64", "qt": args.qt,
        "startedAt": datetime.now(timezone.utc).isoformat(), "command": command,
    }, indent=2) + "\n")
    started = time.monotonic()
    status = 1
    reason = "failed"
    with (work / "ci.log").open("w") as log:
        try:
            snapshot(SOURCE, work / "source")
            process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                       text=True, errors="replace", bufsize=1)

            def copy_output():
                for line in process.stdout:
                    log.write(line)
                    log.flush()
                    sys.stdout.write(line)
                    sys.stdout.flush()

            reader = threading.Thread(target=copy_output)
            reader.start()
            try:
                status = process.wait(timeout=TIMEOUT_SECONDS)
                reason = "passed" if status == 0 else "failed"
            except (subprocess.TimeoutExpired, KeyboardInterrupt) as error:
                status = 124 if isinstance(error, subprocess.TimeoutExpired) else 130
                reason = "timed out" if status == 124 else "interrupted"
                stop_container(name, process)
            finally:
                reader.join()
                process.stdout.close()
        except (OSError, subprocess.SubprocessError) as error:
            log.write(f"CI runner error: {error}\n")
            print(f"CI runner error: {error}", file=sys.stderr)
    (work / "result.json").write_text(json.dumps({
        "status": reason, "exitCode": status, "elapsedSeconds": round(time.monotonic() - started, 3),
    }, indent=2) + "\n")
    print(f"CI {reason} (exit {status}); evidence: {work}", flush=True)
    return status if status >= 0 else 128 - status


if __name__ == "__main__":
    sys.exit(main())
