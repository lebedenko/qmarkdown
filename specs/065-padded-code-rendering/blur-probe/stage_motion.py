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
commit = subprocess.check_output(['git', 'rev-parse', '43d6bba842bb5b7ab23889e49d5f138c6f22a7fb'], cwd=root, text=True).strip()
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
    original = original.replace('#include <QSGTextNode>', '#include <QPixmap>\n#include <QPainterPath>\n#include <QSGRendererInterface>\n#include <QJsonDocument>\n#include <QJsonArray>\n#include <QJsonObject>\n#include <QFile>\n#include <QDir>\n#include <QSGTextNode>')
    original = original.replace('        ++m_owner->m_rasterPaintCount;', '''        const bool calibrate = !qEnvironmentVariableIsEmpty("BLUR_CALIBRATION") && !property("blurCaptured").toBool();
        QImage replay;
        QImage pixmapImage;
        auto *pixmap = dynamic_cast<QPixmap *>(painter->device());
        if (calibrate && pixmap) pixmapImage = pixmap->toImage();
        QImage *actual = pixmap ? &pixmapImage : dynamic_cast<QImage *>(painter->device());
        if (calibrate && actual) {
            replay = QImage(actual->size(), actual->format());
            replay.setDevicePixelRatio(actual->devicePixelRatio());
            replay.fill(Qt::transparent);
            QPainter independent(&replay);
            independent.setRenderHints(painter->renderHints());
            independent.setWorldTransform(painter->worldTransform());
            if (painter->hasClipping()) independent.setClipPath(painter->clipPath());
            independent.translate(-position());
            m_owner->paint(&independent);
        }
        ++m_owner->m_rasterPaintCount;''')
    original = original.replace('        painter->restore();\n    }\nprivate:', '''        painter->restore();
        if (calibrate) {
            if (pixmap) pixmapImage = pixmap->toImage();
            setProperty("blurCaptured", true);
            const auto transform = painter->worldTransform();
            QJsonObject result{{"type", "rawCalibration"}, {"imageDevice", bool(actual)},
                {"equal", actual && *actual == replay}, {"renderHints", int(painter->renderHints())},
                {"clipping", painter->hasClipping()},
                {"transform", QJsonArray{transform.m11(), transform.m12(), transform.m21(), transform.m22(), transform.dx(), transform.dy()}}};
            if (actual) {
                result.insert("format", int(actual->format()));
                result.insert("width", actual->width()); result.insert("height", actual->height());
                result.insert("dpr", actual->devicePixelRatio());
                const QString prefix = qEnvironmentVariable("BLUR_CALIBRATION") + "/" + QString::number(quintptr(this), 16);
                actual->save(prefix + "-actual.png"); replay.save(prefix + "-independent.png");
                result.insert("prefix", prefix);
                setProperty("blurImage", replay);
            }
            setProperty("blurRawEqual", actual && *actual == replay);
            puts(QJsonDocument(result).toJson(QJsonDocument::Compact).constData()); fflush(stdout);
        }
    }
private:''')
    renderer.write_text(original)
    if variant == 'candidate':
        renderer.write_text(original.replace('setSmooth(retained);', '''const auto api = window() ? window()->rendererInterface()->graphicsApi() : QSGRendererInterface::Unknown;
    setSmooth(retained && api != QSGRendererInterface::OpenGL && api != QSGRendererInterface::Vulkan);'''))
    manifest = {'commit': commit, 'variant': variant, 'rendererSha256': hashlib.sha256(renderer.read_bytes()).hexdigest(),
                'instrumentation': 'BlurProbe friendship and opt-in first real paint calibration v2', 'candidateChange': 'retained smoothing off only on observed OpenGL/Vulkan', 'policy': 'native-only-v2', 'stagerSha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}
    (target / 'blur-source.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print(json.dumps(manifest))
