#!/usr/bin/env python3
"""Build, relocate, and consume both package variants without dependency downloads."""

import os
from pathlib import Path
import shutil
import subprocess
import tempfile

source = Path(__file__).resolve().parents[1]
work = Path(tempfile.mkdtemp(prefix="qmarkdown-packaging-"))
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
        f"-DQMARKDOWN_QML_INSTALL_DIR={qml_dir}")
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
    if list(relocated.rglob("cmark*.h")):
        raise RuntimeError("Private cmark headers were installed")
    consumer_build = work / f"consumer-{variant}"
    run("cmake", "-S", consumer_source, "-B", consumer_build,
        f"-DCMAKE_PREFIX_PATH={relocated}")
    run("cmake", "--build", consumer_build, "--parallel", "2")
    environment = os.environ.copy()
    environment["QT_QPA_PLATFORM"] = "offscreen"
    environment.pop("QML2_IMPORT_PATH", None)
    environment.pop("LD_LIBRARY_PATH", None)
    environment["QML_IMPORT_PATH"] = str(relocated / qml_dir) if variant == "shared" else ""
    executable = "qmarkdown-consumer.exe" if os.name == "nt" else "qmarkdown-consumer"
    run(consumer_build / executable, env=environment)
    if variant == "shared":
        plugin_consumer = work / "consumer-shared-plugin"
        run("cmake", "-S", consumer_source, "-B", plugin_consumer,
            f"-DCMAKE_PREFIX_PATH={relocated}", "-DQMARKDOWN_CONSUMER_LINK_MODULE=OFF")
        run("cmake", "--build", plugin_consumer, "--parallel", "2")
        run(plugin_consumer / executable, env=environment)
print("PASS: shared and static relocated package consumers", flush=True)
