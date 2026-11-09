#include <QGuiApplication>
#include <QQuickWindow>
#include <QQuickItem>
#include <QSGTextNode>
#include <QSGRendererInterface>
#include <QTextLayout>
#include <QFontDatabase>
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QPainter>
#include <QTimer>
#include <QElapsedTimer>
#include <QScreen>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <cstdio>
#include <cmath>

static void record(QJsonObject value)
{
    const auto bytes = QJsonDocument(value).toJson(QJsonDocument::Compact);
    std::fprintf(stdout, "%s\n", bytes.constData());
    std::fflush(stdout);
}

class Control final : public QQuickItem
{
public:
    QTextLayout layout;
    QSGTextNode::RenderType type;
    Control(QQuickWindow &w, bool curve, const QFont &font) : QQuickItem(w.contentItem()),
        type(curve ? QSGTextNode::CurveRendering : QSGTextNode::QtRendering)
    {
        setFlag(ItemHasContents);
        setSize({760, 360});
        setPosition({20, 20});
        layout.setFont(font);
        layout.setText(QString::fromUtf8("office ffi e\u0301 fX — العربية עברית 日本語\u2028"
                                        "Before code after \uFFFC object; mixed italic and bold."));
        QTextCharFormat italic, bold;
        italic.setFontItalic(true);
        bold.setFontWeight(QFont::Bold);
        layout.setFormats({{7, 3, italic}, {11, 2, bold}});
        layout.setCacheEnabled(true);
        layout.beginLayout();
        qreal y = 0;
        for (;;) {
            auto line = layout.createLine();
            if (!line.isValid()) break;
            line.setLineWidth(720);
            line.setPosition({0, y});
            y += line.height();
        }
        layout.endLayout();
    }
    void rebuild() { update(); }
protected:
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override
    {
        delete old;
        auto *node = window()->createTextNode();
        node->setRenderType(type);
        node->setColor(Qt::black);
        node->addTextLayout({}, &layout);
        return node;
    }
};

int main(int argc, char **argv)
{
    record({{"stage", "before-application"}});
    QGuiApplication app(argc, argv);
    const auto args = app.arguments();
    if (args.size() < 4) return 2;
    const bool curve = args[1] == "curve";
    const QString output = args[2], fontPath = args[3];
    const bool interactive = args.contains("--interactive");
    QFile fontFile(fontPath);
    if (!fontFile.open(QIODevice::ReadOnly)) return 3;
    const auto fontHash = QCryptographicHash::hash(fontFile.readAll(), QCryptographicHash::Sha256).toHex();
    const int fontId = QFontDatabase::addApplicationFont(fontPath);
    if (fontId < 0) return 3;
    QFont font(QFontDatabase::applicationFontFamilies(fontId).first());
    font.setPixelSize(30);
    QDir().mkpath(output);
    record({{"stage", "layout"}, {"qt", qVersion()}, {"fontSha256", QString(fontHash)}});
    QQuickWindow window;
    window.resize(800, 400);
    window.setColor(Qt::white);
    window.setTitle(QStringLiteral("Placement gate — %1 — full architecture NOT qualified").arg(args[1]));
    Control item(window, curve, font);
    const auto original = item.layout.glyphRuns(0, -1, QTextLayout::RetrieveGlyphIndexes
        | QTextLayout::RetrieveGlyphPositions | QTextLayout::RetrieveStringIndexes);
    const int codeStart = item.layout.text().indexOf("code");
    const int objectStart = item.layout.text().indexOf(QChar::ObjectReplacementCharacter);
    auto moved = original;
    for (auto &run : moved) {
        auto positions = run.positions();
        const auto indexes = run.stringIndexes();
        for (qsizetype i = 0; i < positions.size(); ++i) {
            // Model code gutters and an object's reserved advance after shaping.
            const int source = int(indexes[i]);
            positions[i].rx() += source >= codeStart + 4 ? 16 : source >= codeStart ? 8 : 0;
            if (source > objectStart) positions[i].rx() += 40;
        }
        run.setPositions(positions);
    }
    bool detached = true, changed = false;
    const auto unchanged = item.layout.glyphRuns(0, -1, QTextLayout::RetrieveGlyphIndexes
        | QTextLayout::RetrieveGlyphPositions | QTextLayout::RetrieveStringIndexes);
    for (qsizetype i = 0; i < original.size(); ++i) {
        detached = detached && unchanged[i].positions() == original[i].positions();
        changed = changed || moved[i].positions() != original[i].positions();
    }
    detached = detached && changed;
    record({{"stage", "placement-contract"}, {"copiedRunsDoNotUpdateLayout", detached},
            {"codeStart", codeStart}, {"objectStart", objectStart}});
    const auto paintRuns = [&](const QList<QGlyphRun> &runs, const QString &name) {
        QImage image(800, 400, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);
        QPainter painter(&image);
        painter.setPen(Qt::black);
        for (const auto &run : runs) painter.drawGlyphRun({20, 20}, run);
        painter.end();
        if (!image.save(output + "/" + name + ".png")) app.exit(4);
        return image;
    };
    const bool placementDiffers = paintRuns(original, "original-runs") != paintRuns(moved, "moved-runs");
    QElapsedTimer clock;
    clock.start();
    QJsonArray swaps;
    QObject::connect(&window, &QQuickWindow::frameSwapped, &app, [&] {
        swaps.append(double(clock.nsecsElapsed()) / 1e6);
    }, Qt::QueuedConnection);
    record({{"stage", "before-exposure"}});
    window.show();
    QTimer motion;
    int frame = 0;
    QObject::connect(&motion, &QTimer::timeout, &app, [&] {
        item.setY(20 + 12 * std::sin(++frame * .04));
    });
    QTimer::singleShot(600, &app, [&] {
        const auto api = window.rendererInterface()->graphicsApi();
        record({{"stage", "capture-before"}, {"api", int(api)},
                {"dpr", window.devicePixelRatio()}, {"refreshHz", window.screen()->refreshRate()},
                {"platform", QGuiApplication::platformName()}});
        auto before = window.grabWindow();
        if (before.isNull() || !before.save(output + "/before.png")) { app.exit(4); return; }
        item.rebuild();
        QTimer::singleShot(200, &app, [&, before] {
            record({{"stage", "capture-after"}});
            auto after = window.grabWindow();
            if (after.isNull() || !after.save(output + "/after.png")) { app.exit(4); return; }
            const bool same = before == after;
            record({{"stage", "placement-result"}, {"exactLayoutReplay", same},
                    {"requestedPlacementDiffers", placementDiffers},
                    {"adapterBlocked", detached && same && placementDiffers}});
            if (!same || !detached || !placementDiffers) { app.exit(5); return; }
            swaps = {};
            clock.restart();
            motion.start(16);
            QTimer::singleShot(1500, &app, [&] {
                record({{"stage", "motion-smoke"}, {"swapNotificationMs", swaps},
                        {"presentationQualified", false}});
                if (!interactive) app.quit();
            });
        });
    });
    const int result = app.exec();
    record({{"stage", "exit"}, {"code", result}});
    return result;
}
