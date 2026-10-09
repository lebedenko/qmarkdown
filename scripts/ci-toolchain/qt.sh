#!/usr/bin/env bash
set -euo pipefail
case "$CI_QT_VERSION" in 6.8.0|6.11.3) ;; *) exit 2 ;; esac
cd /opt/qmarkdown-provenance
/opt/aqt/bin/aqt install-qt linux desktop "$CI_QT_VERSION" linux_gcc_64 -O /opt/Qt
qt_root="/opt/Qt/$CI_QT_VERSION/gcc_64"
test "$("$qt_root/bin/qmake" -query QT_VERSION)" = "$CI_QT_VERSION"
test -f "$qt_root/lib/cmake/Qt6/Qt6Config.cmake"
printf '%s\n' "$CI_QT_VERSION" > qt-version.txt
