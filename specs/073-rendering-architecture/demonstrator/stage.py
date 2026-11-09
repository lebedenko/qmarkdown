#!/usr/bin/env python3
"""Stage the approved prepared geometry without changing shipping files."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import subprocess
import tarfile

COMMIT = "43d6bba842bb5b7ab23889e49d5f138c6f22a7fb"


def stage(destination):
    root = Path(__file__).resolve().parents[3]
    destination.mkdir(parents=True, exist_ok=False)
    archive = subprocess.check_output(["git", "archive", COMMIT], cwd=root)
    with tarfile.open(fileobj=io.BytesIO(archive)) as source:
        source.extractall(destination, filter="data")
    header = destination / "src/QMarkdown/private/formattedtext.h"
    header.write_text(header.read_text().replace(
        "friend class StylingTest;", "friend class StylingTest;\n    friend class DemoAccess;"))
    cpp = destination / "src/QMarkdown/private/formattedtext.cpp"
    source = cpp.read_text()
    # Preparation only. The demonstrator owns all rendering resources.
    begin = source.index("    if (sceneGraphEligible()) {", source.index("void FormattedText::update(int layer)"))
    end = source.index("\n}\nFormattedText::FormattedText", begin)
    source = source[:begin] + "    return; // Isolated demonstrator owns its surfaces." + source[end:]
    marker = "        QVector<Formula> layoutRequests = m_mathRequests;"
    assert source.count(marker) == 1
    source = source.replace(marker, marker + """
        for (const auto &value : property("demoObjects").toList()) {
            const auto object = value.toMap();
            auto recording = std::make_shared<QMarkdownPrivate::MathRecording>();
            recording->ascent = object.value("ascent").toDouble();
            recording->descent = object.value("descent").toDouble();
            recording->size = {object.value("width").toDouble(), recording->ascent + recording->descent};
            QPainter painter(&recording->picture);
            painter.setPen(QColor("#176b58")); painter.setBrush(QColor("#b9eadb"));
            painter.drawRoundedRect(QRectF(QPointF(), recording->size).adjusted(1,1,-1,-1), 4, 4);
            painter.end();
            layoutRequests.append({object.value("start").toInt(), object.value("length").toInt(),
                                   {}, recording, true});
        }
""")
    cpp.write_text(source)
    patch = Path(__file__).with_name("prepared-cursor.patch")
    subprocess.run(["git", "apply", "--check", str(patch)], cwd=destination, check=True)
    subprocess.run(["git", "apply", str(patch)], cwd=destination, check=True)
    manifest = {"patchSha256": hashlib.sha256(patch.read_bytes()).hexdigest(), "commit": COMMIT, "purpose": "preparation-only; explicit fixture objects",
                "files": {str(p.relative_to(destination)): hashlib.sha256(p.read_bytes()).hexdigest()
                          for p in (header, cpp)}}
    (destination / "demo-source.json").write_text(json.dumps(manifest, indent=2) + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("destination", type=Path)
    stage(parser.parse_args().destination)
