#!/bin/bash
set -euo pipefail
[[ -f /.dockerenv && $(uname -m) == x86_64 && $EUID == 0 ]]
mode=$1
pacman -Syu --noconfirm
if [[ $mode == build ]]; then
    pacman -S --needed --noconfirm base-devel cmake ninja git python noto-fonts qt6-base qt6-declarative
else
    pacman -S --needed --noconfirm base-devel cmake ninja git python namcap noto-fonts qt6-base qt6-declarative
fi
retain_build_evidence() {
    local status=$?
    if [[ $mode == build && -d /work ]]; then
        find /work -name '*.log' -o -name 'commonmark-report.json' -o -name 'CMakeConfigureLog.yaml' | while read -r path; do
            local relative=${path#/work/}
            mkdir -p "/evidence/build-details/$(dirname "$relative")"
            cp "$path" "/evidence/build-details/$relative"
        done
    fi
    return "$status"
}
trap retain_build_evidence EXIT
pacman -Q > "/evidence/$mode-packages.txt"
cp /etc/os-release "/evidence/$mode-os-release"
unset LD_LIBRARY_PATH QML_IMPORT_PATH QML2_IMPORT_PATH QT_PLUGIN_PATH CMAKE_PREFIX_PATH
export QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software
useradd -m builder
mkdir /work
chown builder:builder /work
if [[ $mode == build ]]; then
    cp /evidence/qmarkdown-source.tar.gz /work/
    python - <<'PY'
import importlib.util, os
from pathlib import Path
import subprocess
spec = importlib.util.spec_from_file_location('runner', '/tools/build-arch-package.py')
runner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)
qt = subprocess.check_output(['qmake6', '-query', 'QT_VERSION'], text=True).strip()
Path('/work/PKGBUILD').write_text(runner.recipe(Path('/tools/PKGBUILD.in').read_text(),
    os.environ['PKG_VERSION'], os.environ['PKG_RELEASE'], qt, os.environ['SOURCE_SHA256']))
PY
    cp /work/PKGBUILD /evidence/PKGBUILD
    chown -R builder:builder /work
    runuser -u builder -- bash -ec 'cd /work; makepkg --noconfirm --check --log'
    cp /work/*.pkg.tar.zst /evidence/
    cp /work/*.log /evidence/
else
    python /tools/verify.py
fi
