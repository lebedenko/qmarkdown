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
parser.add_argument("--syntax-highlighting", choices=("OFF", "ON"), default="OFF")
parser.add_argument("--math", choices=("OFF", "ON"), default="ON")
parser.add_argument("--kde-prefix", type=Path)
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
if args.kde_prefix:
    qt_args = [a for a in qt_args if not a.startswith("-DCMAKE_PREFIX_PATH=")]
    qt_args.append(f"-DCMAKE_PREFIX_PATH={args.kde_prefix};{qt_root if args.qt_root else '/usr'}")
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
        "-DBUILD_TESTING=ON", "-DQMARKDOWN_BUILD_EXAMPLES=ON",
        f"-DQMARKDOWN_WITH_KSYNTAXHIGHLIGHTING={args.syntax_highlighting}", f"-DQMARKDOWN_WITH_MATH={args.math}",
        f"-DBUILD_SHARED_LIBS={'ON' if variant == 'shared' else 'OFF'}",
        "-DCMAKE_INSTALL_LIBDIR=lib", f"-DCMAKE_INSTALL_PREFIX={prefix}",
        f"-DQMARKDOWN_QML_INSTALL_DIR={qml_dir}", *qt_args)
    if args.syntax_highlighting == "OFF":
        if "KF6SyntaxHighlighting_DIR:" in (build / "CMakeCache.txt").read_text():
            raise RuntimeError("Disabled build discovered KDE")
    run("cmake", "--build", build, "--parallel", "2")
    run("ctest", "--test-dir", build, "--output-on-failure")
    run("cmake", "--build", build, "--target", "all_qmllint")
    if os.name == "posix" and shutil.which("nm"):
        run("python3", source / "scripts/verify-cmark-symbols.py", build)
    run("cmake", "--install", build)
    prefix.rename(relocated)
    module_payload = relocated / qml_dir / "QMarkdown"
    for name in ("MarkdownView.qml", "BlockSequence.qml", "TextBlock.qml", "SelectableText.qml",
                 "ImageBlock.qml", "ListBlock.qml", "HeadingSeparator.qml", "CodeSurface.qml",
                 "TaskIndicatorHost.qml", "CodeActionHost.qml", "TableBlock.qml", "private/Layout.js",
                 "Private/qmldir", "Private/private.qmltypes"):
        if not (module_payload / name).is_file():
            raise RuntimeError(f"Missing installed QML payload: {name}")
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
    for notice in ("microtex/LICENSE", "microtex/PROVENANCE.md", "microtex/fonts/OFL.txt", "microtex/fonts/Knuth_License.txt", "microtex/fonts/License_for_dsrom.txt", "tinyxml2/LICENSE.txt", "tinyxml2/PROVENANCE.md"):
        if not (relocated / "share/licenses/QMarkdown" / notice).is_file():
            raise RuntimeError(f"Missing math license/provenance: {notice}")
    if list(relocated.rglob("*.h")):
        raise RuntimeError("Private C++/cmark headers were installed")
    consumer_build = work / f"consumer-{variant}"
    # Keep the selected SDK searchable for its tools and transitive dependencies.
    consumer_qt_args = [arg for arg in qt_args if not arg.startswith("-DCMAKE_PREFIX_PATH=")]
    consumer_qt_args.append(f"-DQMARKDOWN_PACKAGE_PREFIX={relocated}")
    if variant == "shared" or args.syntax_highlighting == "OFF":
        consumer_qt_args.append("-DCMAKE_DISABLE_FIND_PACKAGE_KF6SyntaxHighlighting=TRUE")
    consumer_prefix = f"{relocated};{qt_root}" if args.qt_root else str(relocated)
    if args.kde_prefix:
        consumer_prefix += f";{args.kde_prefix}"
    run("cmake", "-S", consumer_source, "-B", consumer_build,
        *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}")
    for version, exact, accepted in (("1.0", False, True), ("1.0.0", True, False),
                                     ("1.1", False, True), ("1.1.0", True, False),
                                     ("0.7", False, False), ("1.2", False, True), ("1.2.0", True, False),
                                     ("1.3", False, True), ("1.3.0", True, False), ("1.4", False, True), ("1.4.0", True, False), ("1.5", False, True), ("1.5.0", True, False), ("1.6", False, True), ("1.6.0", True, False), ("1.7", False, True), ("1.7.0", True, False), ("1.8", False, True), ("1.8.0", True, False), ("1.9", False, True), ("1.9.0", True, False), ("1.10", False, True), ("1.10.0", True, False), ("1.11", False, True), ("1.11.0", True, False), ("1.12", False, True), ("1.12.0", True, False), ("1.13", False, True), ("1.13.0", True, False), ("1.14", False, True), ("1.14.0", True, False), ("1.15", False, True), ("1.15.0", True, False), ("1.16", False, True), ("1.16.0", True, False), ("1.17", False, True), ("1.17.0", False, True), ("1.17.0", True, False), ("1.17.1", True, True), ("1.17.2", False, False), ("1.18", False, False), ("2.0", False, False)):
        request_name = f"request-{variant}-{version}-{'exact' if exact else 'compatible'}"
        request_build = work / request_name
        command = ["cmake", "-S", str(consumer_source), "-B", str(request_build),
                   *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
                   f"-DQMARKDOWN_REQUEST_VERSION={version}",
                   f"-DQMARKDOWN_REQUEST_EXACT={'ON' if exact else 'OFF'}"]
        result = subprocess.run(command, capture_output=True, text=True)
        (work / f"{request_name}.log").write_text(result.stdout + result.stderr)
        if (result.returncode == 0) != accepted:
            raise RuntimeError(f"Unexpected package compatibility: {variant} {version}: {result.stdout}{result.stderr}")
        expected_error = "exactly matches requested version" if exact else "compatible with requested version"
        if not accepted and (expected_error not in " ".join(result.stderr.split())
                             or "version: 1.17.1" not in result.stderr):
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
            "-DQMARKDOWN_REQUEST_EXACT=OFF", f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", autolink_build, "--parallel", "2")
        run(autolink_build / executable, env=environment)
        gfm_build = work / f"gfm-{variant}-{link}"
        run("cmake", "-S", consumer_source, "-B", gfm_build,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_GFM_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.6.0",
            "-DQMARKDOWN_REQUEST_EXACT=OFF", f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", gfm_build, "--parallel", "2")
        run(gfm_build / executable, env=environment)
        selection_build = work / f"selection-{variant}-{link}"
        run("cmake", "-S", consumer_source, "-B", selection_build,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_SELECTION_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.8.0",
            "-DQMARKDOWN_REQUEST_EXACT=OFF", f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", selection_build, "--parallel", "2")
        run(selection_build / executable, env=environment)
        renderer_build = work / f"renderers-{variant}-{link}"
        run("cmake", "-S", consumer_source, "-B", renderer_build,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_RENDERERS_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.8.0",
            "-DQMARKDOWN_REQUEST_EXACT=OFF", f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", renderer_build, "--parallel", "2")
        run(renderer_build / executable, env=environment)
        reading_build = work / f"reading-{variant}-{link}"
        run("cmake", "-S", consumer_source, "-B", reading_build,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_READING_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.17.1",
            "-DQMARKDOWN_REQUEST_EXACT=ON", f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", reading_build, "--parallel", "2")
        run(reading_build / executable, env=environment)
        math_build = work / f"math-{variant}-{link}"
        run("cmake", "-S", consumer_source, "-B", math_build,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_MATH_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.17.1",
            f"-DQMARKDOWN_EXPECT_MATH={1 if args.math == 'ON' else 0}",
            f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", math_build, "--parallel", "2")
        run(math_build / executable, env=environment)
        notes_build = work / f"footnotes-{variant}-{link}"
        run("cmake", "-S", consumer_source, "-B", notes_build,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_FOOTNOTES_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.17.1",
            "-DQMARKDOWN_REQUEST_EXACT=ON", f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", notes_build, "--parallel", "2")
        run(notes_build / executable, env=environment)
        providers_build = work / f"providers-{variant}-{link}"
        run("cmake", "-S", consumer_source, "-B", providers_build,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_PROVIDERS_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.17.1",
            f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", providers_build, "--parallel", "2")
        run(providers_build / executable, env=environment)
        syntax_build = work / f"syntax-{variant}-{link}"
        run("cmake", "-S", consumer_source, "-B", syntax_build,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_SYNTAX_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.17.1",
            f"-DQMARKDOWN_EXPECT_SYNTAX={1 if args.syntax_highlighting == 'ON' else 0}",
            "-DQMARKDOWN_REQUEST_EXACT=ON", f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", syntax_build, "--parallel", "2")
        run(syntax_build / executable, env=environment)
        keyboard_build = work / f"keyboard-{variant}-{link}"
        run("cmake", "-S", consumer_source, "-B", keyboard_build,
            *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
            "-DQMARKDOWN_KEYBOARD_CONSUMER=ON", "-DQMARKDOWN_REQUEST_VERSION=1.17.1",
            "-DQMARKDOWN_REQUEST_EXACT=ON", f"-DQMARKDOWN_CONSUMER_LINK_MODULE={link}")
        run("cmake", "--build", keyboard_build, "--parallel", "2")
        run(keyboard_build / executable, env=environment)
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
    "-DBUILD_TESTING=OFF", "-DQMARKDOWN_BUILD_EXAMPLES=OFF", "-DQMARKDOWN_BUILD_BENCHMARKS=OFF",
    f"-DQMARKDOWN_WITH_KSYNTAXHIGHLIGHTING={args.syntax_highlighting}")
run("cmake", "--build", work / "library-only", "--parallel", "2")
print("PASS: shared and static relocated package consumers", flush=True)
