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
        self.work = self.root / "artifacts"
        self.bin = self.root / "bin"
        self.bin.mkdir()
        docker = self.bin / "docker"
        docker.write_text(f"#!{sys.executable}\n" + '''
import json, os, pathlib, sys, time
with open(os.environ["CI_TEST_COMMANDS"], "a") as log:
    log.write(json.dumps(sys.argv[1:]) + "\\n")
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
        command = json.loads(self.commands.read_text())
        self.assertIn(f"type=bind,src={copied},dst=/source,readonly", command)
        self.assertIn(f"type=bind,src={self.work},dst=/artifacts", command)
        self.assertIn(f"type=bind,src={copied},dst=/artifacts/source,readonly", command)
        self.assertIn(runner.IMAGE, command)
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
