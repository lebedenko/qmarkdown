#!/usr/bin/env python3
"""Verify the private cmark bundle's external definitions in a Linux build."""
from pathlib import Path
import subprocess
import sys

source = Path(__file__).resolve().parents[1]
build = Path(sys.argv[1]).resolve()
expected = {
    line.split()[2]
    for line in (source / "third_party/cmark/symbols.h").read_text().splitlines()
    if line.startswith("#define ")
}
objects = sorted((build / "third_party/cmark").rglob("*.o"))
if not objects:
    raise RuntimeError("No bundled cmark objects found")
output = subprocess.check_output(["nm", "-g", "--defined-only", *map(str, objects)], text=True)
actual = {line.split()[-1] for line in output.splitlines() if len(line.split()) == 3}
if actual != expected:
    raise RuntimeError(f"Symbol mapping mismatch: extra={actual - expected}, missing={expected - actual}")
libraries = sorted((build / "src/QMarkdown").glob("libQMarkdown.so.*"))
if libraries:
    exports = subprocess.check_output(["nm", "-D", "--defined-only", str(libraries[-1])], text=True)
    if any("cmark" in line for line in exports.splitlines()):
        raise RuntimeError("cmark symbol/type leaked into dynamic exports")
print(f"PASS: {len(actual)} bundled external definitions prefixed; shared exports private when present")
