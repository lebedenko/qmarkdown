#!/usr/bin/env python3
"""Stage two immutable committed-source variants; never patch the checkout."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import subprocess
import tarfile

p = argparse.ArgumentParser()
p.add_argument('destination', type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[3]
commit = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip()
archive = subprocess.check_output(['git', 'archive', commit], cwd=root)
for variant in ('baseline', 'candidate'):
    target = a.destination / variant
    target.mkdir(parents=True, exist_ok=False)
    with tarfile.open(fileobj=io.BytesIO(archive)) as source:
        source.extractall(target, filter='data')
    header = target / 'src/QMarkdown/private/formattedtext.h'
    header.write_text(header.read_text().replace('friend class StylingTest;', 'friend class StylingTest;\n    friend class BlurProbe;'))
    renderer = target / 'src/QMarkdown/private/formattedtext.cpp'
    original = renderer.read_text()
    assert original.count('setSmooth(retained);') == 1
    if variant == 'candidate':
        renderer.write_text(original.replace('setSmooth(retained);', 'setSmooth(false);'))
    manifest = {'commit': commit, 'variant': variant, 'rendererSha256': hashlib.sha256(renderer.read_bytes()).hexdigest(),
                'instrumentation': 'BlurProbe friendship only', 'candidateChange': 'setSmooth(retained) -> setSmooth(false)'}
    (target / 'blur-source.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print(json.dumps(manifest))
