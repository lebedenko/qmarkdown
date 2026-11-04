#include "private/formattedtext.h"
#include "private/inline.h"
#include <QGuiApplication>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QQuickPaintedItem>
#include <QFontInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QtTest/QTest>
#include <cmath>
#include <cstdio>

// Changes only the private surface in this executable. Layout and hit cells stay intact.
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    if (argc != 2) return 2;
    QDir().mkpath(argv[1]);
    QQuickWindow window; window.resize(800, 260); window.setColor(Qt::white);
    QQuickItem parent(window.contentItem());
    FormattedText text(&parent); text.setPosition({24.5, 40}); text.setLayoutWidth(740);
    QFont font("DejaVu Sans"); font.setPixelSize(18); text.setFont(font);
    QFont code("DejaVu Sans Mono"); text.setCodeFont(code);
    text.setCodeBackground(QColor("#eeeeee"));
    const QString content = "Try strong, emphasis, inline code, host links, and denied image fallback. <b>HTML stays literal</b>.";
    text.setText(content);
    auto span = [&](QString word, int flags) { return QVariantMap{{"start", content.indexOf(word)}, {"length", word.size()}, {"flags", flags}}; };
    window.show(); if (!QTest::qWaitForWindowExposed(&window)) return 3;
    for (const QString fixture : {"plain", "bold", "italic", "link", "code", "combined"}) {
        QVariantList formats;
        if (fixture == "bold" || fixture == "combined") formats.append(span("strong", QMarkdownPrivate::Strong));
        if (fixture == "italic" || fixture == "combined") formats.append(span("emphasis", QMarkdownPrivate::Emphasis));
        if (fixture == "code" || fixture == "combined") formats.append(span("inline code", QMarkdownPrivate::Code));
        text.setFormatRanges(formats);
        text.setLinkSpans(fixture == "link" || fixture == "combined" ? QVariantList{QVariantMap{{"start", content.indexOf("host links")}, {"length", 10}, {"logicalId", 0}, {"href", "#test"}}} : QVariantList{});
        QTest::qWait(60);
        const auto *layout = text.layout();
        const auto height = text.logicalHeight();
        const auto cells = text.rangeRectangles(0, content.size());
        QPointF retainedPosition;
        for (auto *child : text.childItems()) if (dynamic_cast<QQuickPaintedItem *>(child)) retainedPosition = child->position();
        for (const QString variant : {"current", "nearest", "aligned"}) {
            if (variant != "current" && fixture != "code" && fixture != "combined") continue;
            for (qreal scroll : {0., .25, .5, .75, 1.}) {
                parent.setY(-scroll); QTest::qWait(30);
                QQuickPaintedItem *surface = nullptr;
                for (auto *child : text.childItems()) if ((surface = dynamic_cast<QQuickPaintedItem *>(child))) break;
                if (surface) {
                    surface->setSmooth(variant == "current");
                    // Reset retained placement before applying the independent alignment control.
                    surface->setPosition(retainedPosition);
                    if (variant == "aligned") {
                        const auto scene = text.mapToScene({}); const qreal dpr = window.effectiveDevicePixelRatio();
                        surface->setPosition({std::floor((scene.x()-1)*dpr)/dpr-scene.x(), std::floor((scene.y()-1)*dpr)/dpr-scene.y()});
                    }
                    surface->update();
                }
                QTest::qWait(40);
                if (text.layout() != layout || text.logicalHeight() != height
                    || text.rangeRectangles(0, content.size()) != cells) return 6;
                auto image = window.grabWindow(); if (image.isNull()) return 4;
                const QString name = fixture + "-" + variant + "-" + QString::number(scroll);
                if (!image.save(QString(argv[1]) + "/" + name + ".png")) return 5;
                const qreal dpr = window.effectiveDevicePixelRatio();
                // Identical regular prefix before any formatting/padding/decorations.
                const QRect roi(qRound(24.5*dpr), qRound((40-scroll)*dpr), qRound(27*dpr), qRound(24.5*dpr));
                double edges = 0; int count = 0;
                for (int y=roi.top(); y<roi.bottom(); ++y) for (int x=roi.left(); x<roi.right(); ++x) {
                    edges += std::abs(qGray(image.pixel(x+1,y))-qGray(image.pixel(x,y)));
                    edges += std::abs(qGray(image.pixel(x,y+1))-qGray(image.pixel(x,y))); count += 2;
                }
                QJsonArray fonts;
                for (const auto &run : text.layout()->glyphRuns()) if (!fonts.contains(run.rawFont().familyName())) fonts.append(run.rawFont().familyName());
                const auto pos = (surface ? static_cast<QQuickItem *>(surface) : &text)->mapToScene({});
                QJsonObject record{{"capture", name}, {"qt", qVersion()}, {"graphicsApi", int(window.rendererInterface()->graphicsApi())}, {"platform", app.platformName()}, {"dpr", dpr}, {"fonts", fonts}, {"surface", surface ? "raster" : "scenegraph"}, {"smooth", surface && surface->smooth()}, {"physicalX", pos.x()*dpr}, {"physicalY", pos.y()*dpr}, {"edgeContrast", edges/count}, {"logicalHeight", text.logicalHeight()}};
                puts(QJsonDocument(record).toJson(QJsonDocument::Compact).constData());
            }
        }
    }
}
