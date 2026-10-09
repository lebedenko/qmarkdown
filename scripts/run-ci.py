#!/usr/bin/env python3
"""Run the GitHub Qt verification commands in their pinned Ubuntu container."""

import argparse
import hashlib
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import sys
import tempfile
import threading
import time
import uuid

PLATFORM = "linux/amd64"
QT_VERSIONS = ("6.8.0", "6.11.3")
TIMEOUT_SECONDS = 35 * 60
SOURCE = Path(__file__).resolve().parents[1]


def image_metadata(qt, context):
    digest = hashlib.sha256()
    for path in sorted(context.rglob("*")):
        if path.is_file():
            name = path.relative_to(context).as_posix().encode()
            content = path.read_bytes()
            digest.update(len(name).to_bytes(8, "big") + name)
            digest.update(len(content).to_bytes(8, "big") + content)
    digest.update(f"{qt}\0{PLATFORM}".encode())
    fingerprint = digest.hexdigest()
    architecture = PLATFORM.split("/")[-1]
    return {"tag": f"qmarkdown-ci:{qt}-{architecture}-{fingerprint}",
            "recipeFingerprint": fingerprint, "qt": qt, "platform": PLATFORM}


def logged_process(command, log_path, container=None):
    with log_path.open("w") as log:
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                   text=True, errors="replace", bufsize=1, start_new_session=True)

        def copy_output():
            for line in process.stdout:
                log.write(line)
                log.flush()
                sys.stdout.write(line)
                sys.stdout.flush()

        reader = threading.Thread(target=copy_output)
        reader.start()
        try:
            return process.wait(timeout=TIMEOUT_SECONDS)
        except (subprocess.TimeoutExpired, KeyboardInterrupt) as error:
            if container:
                stop_container(container, process)
            else:
                # Buildx may spawn a plugin inheriting the output pipe. Stop the
                # whole private process group so cancellation cannot hang on it.
                try:
                    try:
                        os.killpg(process.pid, signal.SIGTERM)
                    except ProcessLookupError:
                        pass
                    try:
                        process.wait(timeout=10)
                    except subprocess.TimeoutExpired:
                        pass
                finally:
                    try:
                        os.killpg(process.pid, signal.SIGKILL)
                    except ProcessLookupError:
                        pass
                    process.wait()
            return 124 if isinstance(error, subprocess.TimeoutExpired) else 130
        finally:
            reader.join()
            process.stdout.close()


def inspect_image(tag):
    result = subprocess.run(["docker", "image", "inspect", "--format", "{{.Id}}", tag],
                            capture_output=True, text=True, timeout=15)
    if result.returncode:
        return None
    image_id = result.stdout.strip()
    if not image_id.startswith("sha256:"):
        raise RuntimeError("Docker returned an invalid image ID")
    return image_id


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


def docker_command(source, artifacts, qt, name, image_id):
    return [
        "docker", "run", "--rm", "--init", "--name", name,
        "--platform", PLATFORM,
        "--mount", f"type=bind,src={source},dst=/source,readonly",
        "--mount", f"type=bind,src={artifacts},dst=/artifacts",
        # The retained snapshot is also visible below the artifact mount.
        "--mount", f"type=bind,src={source},dst=/artifacts/source,readonly",
        "--workdir", "/tmp",
        "--env", f"CI_QT_VERSION={qt}",
        "--env", f"CI_UID={os.getuid()}",
        "--env", f"CI_GID={os.getgid()}",
        image_id, "timeout", "--signal=TERM", "--kill-after=30s", "35m",
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
    parser.add_argument("--rebuild-image", action="store_true", help="Refresh provisioning without build cache")
    parser.add_argument("--image-metadata", action="store_true", help="Print image identity JSON without Docker or writes")
    args = parser.parse_args()
    context = SOURCE / "scripts" / "ci-toolchain"
    metadata = image_metadata(args.qt, context)
    if args.image_metadata:
        print(json.dumps(metadata))
        return 0
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
    print(f"CI image: {metadata['tag']}\nQt: {args.qt}\nArtifacts: {work}", flush=True)
    started = time.monotonic()
    preparation_seconds = 0
    verification_seconds = 0
    status = 1
    reason = "preparation failed"
    evidence = {**metadata, "startedAt": datetime.now(timezone.utc).isoformat()}
    try:
        # Hash and build the same captured inputs even if the working tree changes.
        with tempfile.TemporaryDirectory(prefix="qmarkdown-toolchain-") as temporary:
            captured = Path(temporary) / "context"
            shutil.copytree(context, captured)
            if image_metadata(args.qt, captured) != metadata:
                raise RuntimeError("Provisioning inputs changed during capture; retry")
            image_id = None if args.rebuild_image else inspect_image(metadata["tag"])
            evidence["imageReused"] = image_id is not None
            if image_id is None:
                build = ["docker", "build", "--platform", PLATFORM, "--progress=plain",
                         "--tag", metadata["tag"], "--build-arg", f"CI_QT_VERSION={args.qt}"]
                if args.rebuild_image:
                    build += ["--no-cache", "--pull"]
                build.append(str(captured))
                evidence["preparationCommand"] = build
                status = logged_process(build, work / "image-preparation.log")
                if status != 0:
                    raise RuntimeError(f"Image preparation exited {status}")
                image_id = inspect_image(metadata["tag"])
                if image_id is None:
                    raise RuntimeError("Prepared image is missing")
            else:
                (work / "image-preparation.log").write_text(f"Reusing {image_id}\n")
        evidence["imageId"] = image_id
        preparation_seconds = time.monotonic() - started
        snapshot(SOURCE, work / "source")
        name = "qmarkdown-ci-" + uuid.uuid4().hex
        command = docker_command(work / "source", work, args.qt, name, image_id)
        evidence["command"] = command
        (work / "environment.json").write_text(json.dumps(evidence, indent=2) + "\n")
        verification_started = time.monotonic()
        status = logged_process(command, work / "ci.log", name)
        verification_seconds = time.monotonic() - verification_started
        reason = {0: "passed", 124: "timed out", 130: "interrupted"}.get(status, "failed")
    except (OSError, subprocess.SubprocessError, RuntimeError, KeyboardInterrupt) as error:
        if isinstance(error, KeyboardInterrupt):
            status = 130
        elif status == 0:
            status = 1
        message = f"CI runner error: {error}\n"
        with (work / "image-preparation.log").open("a") as log:
            log.write(message)
        print(message, file=sys.stderr)
    if not preparation_seconds:
        preparation_seconds = time.monotonic() - started
    (work / "environment.json").write_text(json.dumps(evidence, indent=2) + "\n")
    (work / "result.json").write_text(json.dumps({
        "status": reason, "exitCode": status, "imageId": evidence.get("imageId"),
        "recipeFingerprint": metadata["recipeFingerprint"],
        "preparationSeconds": round(preparation_seconds, 3),
        "verificationSeconds": round(verification_seconds, 3),
        "elapsedSeconds": round(time.monotonic() - started, 3),
    }, indent=2) + "\n")
    print(f"CI {reason} (exit {status}); evidence: {work}", flush=True)
    return status if status >= 0 else 128 - status


if __name__ == "__main__":
    sys.exit(main())
