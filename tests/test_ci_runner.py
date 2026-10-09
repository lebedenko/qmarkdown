"""Exercise local CI source isolation, failure evidence and cancellation without Docker."""

import contextlib
import importlib.util
import io
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location("ci_runner", Path(__file__).resolve().parents[1] / "scripts/run-ci.py")
runner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)


class CiRunnerTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.source = self.root / "source tree"
        self.source.mkdir()
        subprocess.run(["git", "init", "-q", self.source], check=True)
        (self.source / ".gitignore").write_text("build*/\n")
        (self.source / "tracked.txt").write_text("old")
        (self.source / "deleted.txt").write_text("deleted")
        subprocess.run(["git", "-C", self.source, "add", "."], check=True)
        (self.source / "tracked.txt").write_text("uncommitted edit")
        (self.source / "deleted.txt").unlink()
        (self.source / "new.txt").write_text("untracked source")
        for directory in ("build-shared", ".aws", ".codex"):
            (self.source / directory).mkdir()
            (self.source / directory / "fixture.txt").write_text("exclude fixture")
        (self.source / ".env").write_text("exclude fixture")
        recipe = self.source / "scripts" / "ci-toolchain"
        recipe.mkdir(parents=True)
        (recipe / "Dockerfile").write_text("FROM fixture\n")
        self.work = self.root / "artifacts"
        self.bin = self.root / "bin"
        self.bin.mkdir()
        docker = self.bin / "docker"
        docker.write_text(f"#!{sys.executable}\n" + '''
import json, os, pathlib, subprocess, sys, time
with open(os.environ["CI_TEST_COMMANDS"], "a") as log:
    log.write(json.dumps(sys.argv[1:]) + "\\n")
if sys.argv[1:3] == ["image", "inspect"]:
    if os.environ.get("CI_TEST_MISSING") and not pathlib.Path(os.environ["CI_TEST_COMMANDS"] + ".built").exists():
        sys.exit(1)
    print("sha256:" + "a" * 64)
    sys.exit(0)
if sys.argv[1] == "build":
    print("image preparation output", flush=True)
    if os.environ.get("CI_TEST_PREP_SLEEP"):
        subprocess.Popen([sys.executable, "-c", "import time; time.sleep(10)"])
        time.sleep(10)
    if os.environ.get("CI_TEST_PREP_FAIL"):
        sys.exit(23)
    pathlib.Path(os.environ["CI_TEST_COMMANDS"] + ".built").touch()
    sys.exit(0)
if sys.argv[1] == "stop":
    sys.exit(0)
print("container check output", flush=True)
if os.environ.get("CI_TEST_SLEEP"):
    time.sleep(10)
sys.exit(17)
''')
        docker.chmod(0o755)
        self.commands = self.root / "commands.jsonl"

    def invoke(self, *extra, timeout=None):
        with contextlib.ExitStack() as stack:
            stack.enter_context(patch.object(runner, "SOURCE", self.source))
            stack.enter_context(patch.object(sys, "argv", ["run-ci.py", "--qt", "6.8.0",
                                                          "--artifact-dir", str(self.work), *extra]))
            stack.enter_context(patch.dict(os.environ, {
                "PATH": str(self.bin) + os.pathsep + os.environ["PATH"],
                "CI_TEST_COMMANDS": str(self.commands),
            }))
            stack.enter_context(contextlib.redirect_stdout(io.StringIO()))
            stack.enter_context(contextlib.redirect_stderr(io.StringIO()))
            if timeout is not None:
                stack.enter_context(patch.object(runner, "TIMEOUT_SECONDS", timeout))
                stack.enter_context(patch.dict(os.environ, {"CI_TEST_SLEEP": "1"}))
            return runner.main()

    def recorded_commands(self):
        if not self.commands.exists():
            return []
        return [json.loads(line) for line in self.commands.read_text().splitlines()]

    def test_local_image_is_reused_with_provenance_and_timings(self):
        self.assertEqual(self.invoke(), 17)
        self.assertFalse(any(command[0] == "build" for command in self.recorded_commands()))
        evidence = json.loads((self.work / "environment.json").read_text())
        self.assertTrue(evidence["imageReused"])
        self.assertEqual(evidence["imageId"], "sha256:" + "a" * 64)
        result = json.loads((self.work / "result.json").read_text())
        self.assertGreaterEqual(result["preparationSeconds"], 0)
        self.assertGreater(result["verificationSeconds"], 0)
        self.assertEqual(result["recipeFingerprint"], evidence["recipeFingerprint"])

    def test_missing_image_builds_before_verification(self):
        with patch.dict(os.environ, {"CI_TEST_MISSING": "1"}):
            self.assertEqual(self.invoke(), 17)
        commands = self.recorded_commands()
        build = next(command for command in commands if command[0] == "build")
        self.assertIn("CI_QT_VERSION=6.8.0", build)
        self.assertNotIn("--no-cache", build)
        self.assertEqual(commands[-1][0], "run")
        self.assertIn("image preparation output", (self.work / "image-preparation.log").read_text())

    def test_refresh_builds_uncached_even_with_existing_image(self):
        self.assertEqual(self.invoke("--rebuild-image"), 17)
        build = next(command for command in self.recorded_commands() if command[0] == "build")
        self.assertIn("--no-cache", build)
        self.assertIn("--pull", build)

    def test_preparation_failure_retains_evidence_without_checks(self):
        with patch.dict(os.environ, {"CI_TEST_PREP_FAIL": "1"}):
            self.assertEqual(self.invoke("--rebuild-image"), 23)
        self.assertFalse(any(command[0] == "run" for command in self.recorded_commands()))
        self.assertFalse((self.work / "source").exists())
        self.assertIn("image preparation output", (self.work / "image-preparation.log").read_text())
        self.assertEqual(json.loads((self.work / "result.json").read_text())["status"], "preparation failed")

    def test_preparation_timeout_does_not_start_checks(self):
        with patch.dict(os.environ, {"CI_TEST_PREP_SLEEP": "1"}):
            self.assertEqual(self.invoke("--rebuild-image", timeout=0.2), 124)
        self.assertFalse(any(command[0] in {"run", "stop"} for command in self.recorded_commands()))

    def test_metadata_is_read_only_and_identity_ignores_source_edits(self):
        context = self.source / "scripts" / "ci-toolchain"
        first = runner.image_metadata("6.8.0", context)
        (self.source / "tracked.txt").write_text("another edit")
        self.assertEqual(first, runner.image_metadata("6.8.0", context))
        self.assertNotEqual(first, runner.image_metadata("6.11.3", context))
        with patch.object(runner, "PLATFORM", "linux/arm64"):
            self.assertNotEqual(first, runner.image_metadata("6.8.0", context))
        (context / "Dockerfile").write_text("FROM changed\n")
        self.assertNotEqual(first, runner.image_metadata("6.8.0", context))
        with patch.object(runner.shutil, "which", return_value=None):
            self.assertEqual(self.invoke("--image-metadata"), 0)
        self.assertFalse(self.work.exists())
        self.assertFalse(self.commands.exists())

    def test_cancellation_during_preparation_never_starts_checks(self):
        original_wait = subprocess.Popen.wait
        interrupted = False

        def wait(process, *args, **kwargs):
            nonlocal interrupted
            if kwargs.get("timeout") == runner.TIMEOUT_SECONDS and not interrupted:
                interrupted = True
                raise KeyboardInterrupt()
            return original_wait(process, *args, **kwargs)

        with patch.object(subprocess.Popen, "wait", wait):
            self.assertEqual(self.invoke("--rebuild-image"), 130)
        self.assertFalse(any(command[0] in {"run", "stop"} for command in self.recorded_commands()))
        self.assertEqual(json.loads((self.work / "result.json").read_text())["exitCode"], 130)

    def test_cancellation_stops_verification_and_records_evidence(self):
        original_wait = subprocess.Popen.wait
        interrupted = False

        def wait(process, *args, **kwargs):
            nonlocal interrupted
            if kwargs.get("timeout") == runner.TIMEOUT_SECONDS and not interrupted:
                interrupted = True
                raise KeyboardInterrupt()
            return original_wait(process, *args, **kwargs)

        with patch.object(subprocess.Popen, "wait", wait):
            self.assertEqual(self.invoke(), 130)
        self.assertTrue(any(command[0] == "stop" for command in self.recorded_commands()))
        self.assertEqual(json.loads((self.work / "result.json").read_text())["status"], "interrupted")

    def test_current_source_is_isolated_and_failure_evidence_survives(self):
        self.assertEqual(self.invoke(), 17)
        copied = self.work / "source"
        self.assertEqual((copied / "tracked.txt").read_text(), "uncommitted edit")
        self.assertEqual((copied / "new.txt").read_text(), "untracked source")
        for excluded in ("deleted.txt", "build-shared", ".git", ".aws", ".codex", ".env"):
            self.assertFalse((copied / excluded).exists(), excluded)
        self.assertIn("container check output", (self.work / "ci.log").read_text())
        result = json.loads((self.work / "result.json").read_text())
        self.assertEqual((result["status"], result["exitCode"]), ("failed", 17))
        command = self.recorded_commands()[-1]
        self.assertIn(f"type=bind,src={copied},dst=/source,readonly", command)
        self.assertIn(f"type=bind,src={self.work},dst=/artifacts", command)
        self.assertIn(f"type=bind,src={copied},dst=/artifacts/source,readonly", command)
        self.assertIn("sha256:" + "a" * 64, command)
        self.assertIn("linux/amd64", command)
        self.assertNotIn("--privileged", command)

    def test_previous_evidence_is_never_overwritten(self):
        self.work.mkdir()
        (self.work / "ci.log").write_text("previous run")
        with self.assertRaises(SystemExit) as error:
            self.invoke()
        self.assertEqual(error.exception.code, 2)
        self.assertEqual((self.work / "ci.log").read_text(), "previous run")
        self.assertFalse(self.commands.exists())

    def test_custom_artifacts_inside_source_are_not_copied_as_inputs(self):
        self.work = self.source / "ci-output"
        self.assertEqual(self.invoke(), 17)
        self.assertFalse((self.work / "source" / "ci-output").exists())
        self.assertEqual((self.work / "source" / "tracked.txt").read_text(), "uncommitted edit")

    def test_unsupported_version_does_not_launch_docker(self):
        with self.assertRaises(SystemExit) as error:
            self.invoke("--qt", "6.9.0")
        self.assertEqual(error.exception.code, 2)
        self.assertFalse(self.work.exists())
        self.assertFalse(self.commands.exists())

    def test_timeout_stops_only_this_run_and_records_failure(self):
        self.assertEqual(self.invoke(timeout=0.2), 124)
        commands = [json.loads(line) for line in self.commands.read_text().splitlines()]
        stop = next(command for command in commands if command[0] == "stop")
        name = json.loads((self.work / "environment.json").read_text())["command"]
        self.assertEqual(stop[-1], name[name.index("--name") + 1])
        self.assertEqual(json.loads((self.work / "result.json").read_text())["status"], "timed out")


if __name__ == "__main__":
    unittest.main()
