#!/usr/bin/env python3
"""Build, relocate, and consume both package variants without dependency downloads."""

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

source = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--work-dir", type=Path, help="Keep builds and failure artifacts in this new directory")
parser.add_argument("--qt-root", type=Path, help="Explicit Qt SDK root; every configuration uses its Qt6Config")
args = parser.parse_args()
work = args.work_dir.resolve() if args.work_dir else Path(tempfile.mkdtemp(prefix="qmarkdown-packaging-"))
work.mkdir(parents=True, exist_ok=True)
qt_args = []
if args.qt_root:
    qt_root = args.qt_root.resolve()
    qt_config = qt_root / "lib/cmake/Qt6"
    if not (qt_config / "Qt6Config.cmake").is_file():
        parser.error("--qt-root must contain lib/cmake/Qt6/Qt6Config.cmake")
    qt_args = [f"-DQt6_DIR={qt_config}", f"-DCMAKE_PREFIX_PATH={qt_root}"]
print(f"Verification artifacts: {work}", flush=True)


def run(*args, env=None):
    print("+", " ".join(map(str, args)), flush=True)
    subprocess.run(list(map(str, args)), check=True, env=env)


consumer_source = work / "consumer-source"
shutil.copytree(source / "tests/installed-consumer", consumer_source)
for variant in ("shared", "static"):
    build = work / f"build-{variant}"
    prefix = work / f"install-{variant}"
    relocated = work / f"relocated-{variant}"
    qml_dir = "lib/qt6/qml" if variant == "shared" else "share/qml"
    run("cmake", "-S", source, "-B", build,
        f"-DBUILD_SHARED_LIBS={'ON' if variant == 'shared' else 'OFF'}",
        "-DCMAKE_INSTALL_LIBDIR=lib", f"-DCMAKE_INSTALL_PREFIX={prefix}",
        f"-DQMARKDOWN_QML_INSTALL_DIR={qml_dir}", *qt_args)
    run("cmake", "--build", build, "--parallel", "2")
    run("ctest", "--test-dir", build, "--output-on-failure")
    run("cmake", "--build", build, "--target", "all_qmllint")
    if os.name == "posix" and shutil.which("nm"):
        run("python3", source / "scripts/verify-cmark-symbols.py", build)
    run("cmake", "--install", build)
    prefix.rename(relocated)
    for metadata in (relocated / "lib/cmake/QMarkdown").glob("*.cmake"):
        text = metadata.read_text()
        for forbidden in (str(source), str(build), str(prefix), "qmarkdown_cmark", "third_party/cmark"):
            if forbidden in text:
                raise RuntimeError(f"Nonrelocatable path in {metadata}: {forbidden}")
    notices = relocated / "share/licenses/QMarkdown/cmark"
    if (notices / "COPYING").read_bytes() != (source / "third_party/cmark/COPYING").read_bytes():
        raise RuntimeError("Incomplete installed cmark notices")
    if not (notices / "PROVENANCE.md").is_file():
        raise RuntimeError("Missing installed cmark provenance")
    if list(relocated.rglob("*.h")):
        raise RuntimeError("Private C++/cmark headers were installed")
    consumer_build = work / f"consumer-{variant}"
    # Keep the selected SDK searchable for its tools and transitive dependencies.
    consumer_qt_args = [arg for arg in qt_args if not arg.startswith("-DCMAKE_PREFIX_PATH=")]
    consumer_qt_args.append(f"-DQMARKDOWN_PACKAGE_PREFIX={relocated}")
    consumer_prefix = f"{relocated};{qt_root}" if args.qt_root else str(relocated)
    run("cmake", "-S", consumer_source, "-B", consumer_build,
        *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}")
    for version, exact, accepted in (("1.0", False, True), ("1.0.0", True, False),
                                     ("1.1", False, True), ("1.1.0", True, False),
                                     ("0.7", False, False), ("1.2", False, True), ("1.2.0", True, False),
                                     ("1.3", False, True), ("1.3.0", True, False), ("1.4", False, True), ("1.4.0", True, False), ("1.5", False, True), ("1.5.0", True, True), ("1.6", False, False), ("2.0", False, False)):
        request_build = work / f"request-{variant}-{version}"
        command = ["cmake", "-S", str(consumer_source), "-B", str(request_build),
                   *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
                   f"-DQMARKDOWN_REQUEST_VERSION={version}",
                   f"-DQMARKDOWN_REQUEST_EXACT={'ON' if exact else 'OFF'}"]
        result = subprocess.run(command, capture_output=True, text=True)
        (work / f"request-{variant}-{version}.log").write_text(result.stdout + result.stderr)
        if (result.returncode == 0) != accepted:
            raise RuntimeError(f"Unexpected package compatibility: {variant} {version}: {result.stdout}{result.stderr}")
        expected_error = "exactly matches requested version" if exact else "compatible with requested version"
        if not accepted and (expected_error not in " ".join(result.stderr.split())
                             or "version: 1.5.0" not in result.stderr):
            raise RuntimeError(f"Package request failed for an unrelated reason: {result.stderr}")
        print(f"PASS: {variant} package request {version}, exact={exact}, accepted={accepted}", flush=True)
    run("cmake", "--build", consumer_build, "--parallel", "2")
    environment = os.environ.copy()
    environment["QT_QPA_PLATFORM"] = "offscreen"
    environment.pop("QML2_IMPORT_PATH", None)
    environment.pop("LD_LIBRARY_PATH", None)
    environment["QML_IMPORT_PATH"] = str(relocated / qml_dir) if variant == "shared" else ""
    executable = "qmarkdown-consumer.exe" if os.name == "nt" else "qmarkdown-consumer"
    run(consumer_build / executable, env=environment)
    styling_build = work / f"styling-{variant}"
    run("cmake", "-S", consumer_source, "-B", styling_build,
        *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
        "-DQMARKDOWN_STYLING_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.1")
    run("cmake", "--build", styling_build, "--parallel", "2")
    run(styling_build / executable, env=environment)
    for link in (("ON", "OFF") if variant == "shared" else ("ON",)):
        actions_build = work / f"actions-{variant}-{link}"
        run("cmake", "-S", consumer_source, "-B", actions_build,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_ACTIONS_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.2.0",
            "-DQMARKDOWN_REQUEST_EXACT=OFF", f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", actions_build, "--parallel", "2")
        run(actions_build / executable, env=environment)
        tasks_build = work / f"tasks-{variant}-{link}"
        run("cmake", "-S", consumer_source, "-B", tasks_build,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_TASKS_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.3.0",
            "-DQMARKDOWN_REQUEST_EXACT=OFF", f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", tasks_build, "--parallel", "2")
        run(tasks_build / executable, env=environment)
        strike_build = work / f"strike-{variant}-{link}"
        run("cmake", "-S", consumer_source, "-B", strike_build,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_STRIKE_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.4.0",
            "-DQMARKDOWN_REQUEST_EXACT=OFF", f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", strike_build, "--parallel", "2")
        run(strike_build / executable, env=environment)
        autolink_build = work / f"autolink-{variant}-{link}"
        run("cmake", "-S", consumer_source, "-B", autolink_build,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_AUTOLINK_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.5.0",
            "-DQMARKDOWN_REQUEST_EXACT=ON", f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", autolink_build, "--parallel", "2")
        run(autolink_build / executable, env=environment)
    if variant == "shared":
        plugin_consumer = work / "consumer-shared-plugin"
        run("cmake", "-S", consumer_source, "-B", plugin_consumer,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}", "-DQMARKDOWN_CONSUMER_LINK_MODULE=OFF")
        run("cmake", "--build", plugin_consumer, "--parallel", "2")
        run(plugin_consumer / executable, env=environment)
        styling_plugin = work / "styling-shared-plugin"
        run("cmake", "-S", consumer_source, "-B", styling_plugin,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_STYLING_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.1",
            "-DQMARKDOWN_CONSUMER_LINK_MODULE=OFF")
        run("cmake", "--build", styling_plugin, "--parallel", "2")
        run(styling_plugin / executable, env=environment)
run("cmake", "-S", source, "-B", work / "library-only", *qt_args,
    "-DBUILD_TESTING=OFF", "-DQMARKDOWN_BUILD_EXAMPLES=OFF", "-DQMARKDOWN_BUILD_BENCHMARKS=OFF")
run("cmake", "--build", work / "library-only", "--parallel", "2")
print("PASS: shared and static relocated package consumers", flush=True)
