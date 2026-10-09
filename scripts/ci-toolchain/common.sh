#!/usr/bin/env bash
set -euo pipefail
mkdir -p /opt/qmarkdown-provenance
export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y --no-install-recommends \
ca-certificates python3 python3-venv util-linux \
build-essential cmake ninja-build fonts-noto-core fonts-noto-mono \
libfontconfig1 libfreetype6 libdbus-1-3 libglib2.0-0t64 \
libgl1-mesa-dev libxkbcommon-dev libegl1 libopengl0 \
libxcb-cursor0 libxkbcommon-x11-0 libxcb-icccm4 libxcb-image0 \
libxcb-keysyms1 libxcb-render-util0 libxcb-xinerama0 libxcb-xkb1
python3 -m venv /opt/aqt
/opt/aqt/bin/pip install aqtinstall==3.3.0 py7zr==0.22.0
/opt/aqt/bin/pip freeze > /opt/qmarkdown-provenance/installer-packages.txt
dpkg-query -W > /opt/qmarkdown-provenance/ubuntu-packages.txt
cp /etc/os-release /opt/qmarkdown-provenance/os-release
