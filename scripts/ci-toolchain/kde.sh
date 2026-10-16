#!/usr/bin/env bash
# KDE Frameworks 6.10.0, built against this container's exact Qt SDK.
set -euo pipefail
qt_root="/opt/Qt/$CI_QT_VERSION/gcc_64"
kde_prefix=/opt/kde-6.10.0
kde_work=$(mktemp -d)
cd "$kde_work"
python3 - <<'PY'
import hashlib
import urllib.request
from pathlib import Path
archives = {
    'extra-cmake-modules': '506989a0d400913403e669c1912238db053cd6b38dff74b17e2e6f879c79cca0',
    'syntax-highlighting': 'b5b5e343ff27bc5c95be0051d5606dfcb3295f835830e7fc6dac8d2863891699',
}
records = []
for name, expected in archives.items():
    filename = f'{name}-6.10.0.tar.xz'
    url = f'https://download.kde.org/stable/frameworks/6.10/{filename}'
    data = urllib.request.urlopen(url, timeout=120).read()
    actual = hashlib.sha256(data).hexdigest()
    if actual != expected:
        raise SystemExit(f'KDE archive checksum mismatch: {filename}')
    Path(filename).write_bytes(data)
    records.append(f'{expected}  {url}')
Path('/opt/qmarkdown-provenance/kde-archives.txt').write_text('\n'.join(records) + '\n')
PY
tar -xf extra-cmake-modules-6.10.0.tar.xz
tar -xf syntax-highlighting-6.10.0.tar.xz
# Qt 6.12 adds its own _sv literal. Rename only KDE's indexer-local literal;
# the pinned 6.10.0 API and runtime definitions remain unchanged.
python3 - <<'PY_PATCH'
import hashlib
from pathlib import Path
path = Path('syntax-highlighting-6.10.0/src/indexer/katehighlightingindexer.cpp')
original = path.read_bytes()
expected = "a0ecc4f4b5905cd44a97ee73162c774b674e31d8fc08e4987e1c5173deeaca14"
if hashlib.sha256(original).hexdigest() != expected:
    raise SystemExit('Unexpected KDE indexer source before compatibility patch')
patched = original.replace(b'_sv', b'_qmarkdown_sv')
path.write_bytes(patched)
Path('/opt/qmarkdown-provenance/kde-compatibility-patch.txt').write_text(
    f'{path}: local _sv renamed to _qmarkdown_sv for Qt 6.12; '
    f'before={expected}; after={hashlib.sha256(patched).hexdigest()}\n')
PY_PATCH
cmake -S extra-cmake-modules-6.10.0 -B ecm-build -G Ninja \
    -DCMAKE_INSTALL_PREFIX="$kde_prefix" -DBUILD_TESTING=OFF -DBUILD_DOC=OFF
cmake --build ecm-build --parallel 2
cmake --install ecm-build
cmake -S syntax-highlighting-6.10.0 -B syntax-build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$kde_prefix" \
    -DCMAKE_PREFIX_PATH="$qt_root;$kde_prefix" -DQt6_DIR="$qt_root/lib/cmake/Qt6" \
    -DBUILD_TESTING=OFF -DBUILD_QCH=OFF -DBUILD_EXAMPLES=OFF
cmake --build syntax-build --parallel 2
cmake --install syntax-build
find "$kde_prefix" -name libKF6SyntaxHighlighting.so -printf '%h\n' > /etc/ld.so.conf.d/qmarkdown-kde.conf
ldconfig
printf '%s\n' "$CI_QT_VERSION" > /opt/qmarkdown-provenance/kde-qt-version.txt
