#!/usr/bin/env bash
# Runtime initialization and checks for local tasks and the GitHub matrix.
set -euo pipefail

case "${CI_QT_VERSION:-}" in
    6.8.0|6.11.3) ;;
    *) echo "Unsupported CI Qt version" >&2; exit 2 ;;
esac

if [[ "${1:-}" != "verify" ]]; then
    install -o "$CI_UID" -g "$CI_GID" -m 644 /opt/qmarkdown-provenance/* /artifacts/
    mkdir /tmp/qmarkdown-cache /tmp/qmarkdown-runtime
    chown "$CI_UID:$CI_GID" /tmp/qmarkdown-cache /tmp/qmarkdown-runtime
    chmod 700 /tmp/qmarkdown-runtime
    exec setpriv --reuid="$CI_UID" --regid="$CI_GID" --clear-groups \
        bash /source/scripts/ci-container.sh verify
fi

export QT_ROOT_DIR="/opt/Qt/$CI_QT_VERSION/gcc_64"
export PATH="$QT_ROOT_DIR/bin:$PATH"
export QT_QPA_PLATFORM=offscreen QT_QUICK_CONTROLS_STYLE=Fusion
export LC_ALL=C.UTF-8 LANG=C.UTF-8
export XDG_CACHE_HOME=/tmp/qmarkdown-cache XDG_RUNTIME_DIR=/tmp/qmarkdown-runtime
unset QML_IMPORT_PATH QML2_IMPORT_PATH QT_PLUGIN_PATH LD_LIBRARY_PATH
cd /artifacts
{
    printf 'Qt: %s\nUID: %s\nGID: %s\n' "$CI_QT_VERSION" "$(id -u)" "$(id -g)"
    uname -a
    cmake --version
    c++ --version
    python3 --version
    "$QT_ROOT_DIR/bin/qmake" -query
    findmnt -n -o OPTIONS /source
    findmnt -n -o OPTIONS /artifacts/source
} > tool-versions.txt
python3 /source/scripts/verify-packaging.py --qt-root "$QT_ROOT_DIR" --work-dir /artifacts/packaging
cmake -S /source -B /artifacts/benchmarks \
    -DQt6_DIR="$QT_ROOT_DIR/lib/cmake/Qt6" -DCMAKE_PREFIX_PATH="$QT_ROOT_DIR" \
    -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DQMARKDOWN_BUILD_EXAMPLES=OFF \
    -DQMARKDOWN_BUILD_BENCHMARKS=ON
cmake --build /artifacts/benchmarks --target qmarkdown-benchmark --parallel 2
QML_IMPORT_PATH=/artifacts/benchmarks/qml timeout 120 \
    /artifacts/benchmarks/tests/qmarkdown-benchmark --smoke > /artifacts/benchmarks/benchmark.json
python3 /source/scripts/verify-benchmark.py /artifacts/benchmarks/benchmark.json
echo "PASS: Ubuntu container packaging and benchmark smoke"
