#!/usr/bin/env python3
"""Build, relocate, and consume both package variants without dependency downloads."""

import argparse
import os
import re
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
parser.add_argument("--shared-only", action="store_true")
parser.add_argument("--skip-library-only", action="store_true")
args = parser.parse_args()
version = re.search(r'project\(QMarkdown VERSION (\d+\.\d+\.\d+)', (source / 'CMakeLists.txt').read_text())[1]
qml_version = re.search(r'VERSION\s+(\d+\.\d+)', (source / 'src/QMarkdown/CMakeLists.txt').read_text())[1]
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
# Fixtures always import the current module contract.
for fixture in consumer_source.glob("*.cpp"):
    fixture.write_text(re.sub(r'import QMarkdown \d+\.\d+', 'import QMarkdown ' + qml_version, fixture.read_text()))
for variant in (("shared",) if args.shared_only else ("shared", "static")):
    build = work / f"build-{variant}"
    prefix = work / f"install-{variant}"
    relocated = work / f"relocated-{variant}"
    qml_dir = "lib/qt6/qml" if variant == "shared" else "share/qml"
    run("cmake", "-G", "Ninja", "-S", source, "-B", build,
        "-DBUILD_TESTING=ON", "-DQMARKDOWN_BUILD_EXAMPLES=ON",
        f"-DQMARKDOWN_WITH_KSYNTAXHIGHLIGHTING={args.syntax_highlighting}", f"-DQMARKDOWN_WITH_MATH={args.math}",
        f"-DBUILD_SHARED_LIBS={'ON' if variant == 'shared' else 'OFF'}",
        "-DCMAKE_INSTALL_LIBDIR=lib", f"-DCMAKE_INSTALL_PREFIX={prefix}",
        f"-DQMARKDOWN_QML_INSTALL_DIR={qml_dir}", *qt_args)
    if args.syntax_highlighting == "OFF":
        if "KF6SyntaxHighlighting_DIR:" in (build / "CMakeCache.txt").read_text():
            raise RuntimeError("Disabled build discovered KDE")
    run("cmake", "--build", build, "--parallel", "2")
    run("ctest", "--parallel", "1", "--test-dir", build, "--output-on-failure")
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
    run("cmake", "-G", "Ninja", "-S", consumer_source, "-B", consumer_build,
        *consumer_qt_args, f"-DCMAKE_PREFIX_PATH={consumer_prefix}",
        f"-DQMARKDOWN_REQUEST_VERSION={version}", "-DQMARKDOWN_REQUEST_EXACT=ON",
        f"-DQMARKDOWN_EXPECT_SYNTAX={int(args.syntax_highlighting == 'ON')}",
        f"-DQMARKDOWN_EXPECT_MATH={int(args.math == 'ON')}")
    run("cmake", "--build", consumer_build, "--parallel", "2")
    environment = os.environ.copy()
    environment["QT_QPA_PLATFORM"] = "offscreen"
    environment.pop("QML2_IMPORT_PATH", None)
    environment.pop("LD_LIBRARY_PATH", None)
    environment["QML_IMPORT_PATH"] = str(relocated / qml_dir) if variant == "shared" else ""
    run("ctest", "--parallel", "1", "--test-dir", consumer_build, "--output-on-failure", env=environment)
if not args.skip_library_only:
    run("cmake", "-G", "Ninja", "-S", source, "-B", work / "library-only", *qt_args,
        "-DBUILD_TESTING=OFF", "-DQMARKDOWN_BUILD_EXAMPLES=OFF", "-DQMARKDOWN_BUILD_BENCHMARKS=OFF",
        f"-DQMARKDOWN_WITH_KSYNTAXHIGHLIGHTING={args.syntax_highlighting}")
    run("cmake", "--build", work / "library-only", "--parallel", "2")
print("PASS: selected relocated package consumers", flush=True)
