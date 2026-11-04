#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QtQml/QQmlExtensionPlugin>
#include <QtTest/QTest>
#include <cmath>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

namespace {
QList<QQuickItem *> named(QQuickItem *root, const QString &name)
{
    QList<QQuickItem *> result;
    for (auto *child : root->childItems()) {
        if (child->objectName() == name) result.append(child);
        result.append(named(child, name));
    }
    return result;
}

bool isRed(QRgb c) { return qRed(c) > 200 && qGreen(c) < 60 && qBlue(c) < 60; }
bool isWhite(QRgb c) { return qRed(c) > 245 && qGreen(c) > 245 && qBlue(c) > 245; }

// Lengths of runs of pure red along a line; `partial` counts pixels that are neither
// white nor pure red (anti-aliased coverage), which would betray unsnapped edges.
struct Runs { QList<int> lengths; int partial = 0; };

Runs scan(const QImage &image, bool vertical, int fixed, int from, int to)
{
    Runs runs;
    int current = 0;
    for (int i = from; i < to; ++i) {
        const QRgb c = vertical ? image.pixel(fixed, i) : image.pixel(i, fixed);
        if (isRed(c)) ++current;
        else {
            if (current) runs.lengths.append(current);
            current = 0;
            if (!isWhite(c)) ++runs.partial;
        }
    }
    if (current) runs.lengths.append(current);
    return runs;
}

QByteArray document(qreal borderWidth, qreal separator, qreal thematic, bool aligned, bool fractionalOrigin)
{
    return QByteArray(R"(
import QtQuick
import QMarkdown 1.18
Rectangle {
    width: 900; height: 700; color: "white"
    property alias shift: viewport.contentY
    Flickable {
        id: viewport
        objectName: "viewport"
        anchors.fill: parent
        contentHeight: document.height + 800
        clip: true
        pixelAligned: %1
        MarkdownView {
            id: document
            objectName: "document"
            x: %2; y: 24; width: 841
            selectionEnabled: true
            gfmEnabled: true
            markdown: "## Heading\n\n---\n\n| A | B | C |\n|---|---|---|\n"
                + "| One | A somewhat longer second cell that wraps onto further lines in a column | x |\n".repeat(3)
                + "|   |   |   |\n".repeat(6)
            style: MarkdownStyle {
                preset: MarkdownStyle.PolishedLight
                table.borderWidth: %3
                table.borderColor: "red"
                table.headerBackground: "white"
                table.bodyBackground: "white"
                table.alternateRowBackground: "white"
                headings.h2.separatorColor: "red"
                headings.h2.separatorThickness: %4
                thematicBreakColor: "red"
                thematicBreakThickness: %5
            }
        }
    }
}
)").replace("%1", aligned ? "true" : "false")
   .replace("%2", fractionalOrigin ? "24.5" : "24")
   .replace("%3", QByteArray::number(borderWidth))
   .replace("%4", QByteArray::number(separator))
   .replace("%5", QByteArray::number(thematic));
}
}

class LinesTest : public QObject
{
    Q_OBJECT
    struct Host {
        QQmlEngine engine;
        std::unique_ptr<QQuickWindow> window;
        std::unique_ptr<QObject> root;
        QQuickItem *viewport = nullptr;
        QQuickItem *view = nullptr;
    };
    static std::unique_ptr<Host> open(const QByteArray &source)
    {
        auto host = std::make_unique<Host>();
        QQmlComponent component(&host->engine);
        component.setData(source, QUrl());
        host->root.reset(component.create());
        if (!host->root) { qWarning() << component.errorString(); return nullptr; }
        auto *item = qobject_cast<QQuickItem *>(host->root.get());
        host->window = std::make_unique<QQuickWindow>();
        host->window->resize(900, 700);
        item->setParentItem(host->window->contentItem());
        host->window->show();
        if (!QTest::qWaitForWindowExposed(host->window.get())) return nullptr;
        host->viewport = named(item, "viewport").value(0);
        host->view = named(item, "document").value(0);
        return host;
    }
    static int expected(qreal logical, qreal dpr) { return qMax(1, int(std::llround(logical * dpr))); }

    // Checks every line in a vertical strip and one horizontal strip for exact thickness.
    static void verifyThickness(Host &host, int tableLine, int separator, int thematic)
    {
        QTest::qWait(30);
        const QImage image = host.window->grabWindow();
        QVERIFY(!image.isNull());
        const qreal dpr = host.window->effectiveDevicePixelRatio();
        // Strip through empty padding of the first cell of every row, away from text.
        auto cells = named(host.view, "tableCell");
        QVERIFY(cells.size() >= 3);
        // Scan inside the right half of the third column: it holds only "x" at the left.
        QQuickItem *probe = nullptr;
        for (auto *cell : cells) if (cell->x() > cell->width()) { probe = cell; break; }
        QVERIFY(probe);
        const QPointF origin = probe->mapToScene(QPointF(probe->width() * 0.9, 0));
        const int x = int(origin.x() * dpr);
        const Runs column = scan(image, true, x, 0, image.height());
        QCOMPARE(column.partial, 0);
        QVERIFY(!column.lengths.isEmpty());
        // Heading separator, thematic break and table borders appear in this column in order.
        QCOMPARE(column.lengths.value(0), separator);
        QCOMPARE(column.lengths.value(1), thematic);
        for (int i = 2; i < column.lengths.size(); ++i)
            QVERIFY2(column.lengths.at(i) == tableLine,
                     qPrintable(QString("table line %1 is %2 px, expected %3").arg(i - 2).arg(column.lengths.at(i)).arg(tableLine)));
        QVERIFY(column.lengths.size() > 8);

        // Horizontal strip through the middle of the last (empty) row crosses vertical lines only.
        QQuickItem *last = cells.last();
        const QPointF rowMid = last->mapToScene(QPointF(0, last->height() / 2));
        const int y = int(rowMid.y() * dpr);
        const Runs row = scan(image, false, y, 0, image.width());
        QCOMPARE(row.partial, 0);
        QCOMPARE(row.lengths.size(), 4);
        for (int length : row.lengths) QCOMPARE(length, tableLine);
    }

private slots:
    void idleLinesStopRequestingFrames_data()
    {
        QTest::addColumn<bool>("transparent");
        QTest::addColumn<qreal>("thickness");
        QTest::newRow("zero-thickness") << false << qreal(0);
        QTest::newRow("transparent") << true << qreal(1);
        QTest::newRow("ordinary-grid") << false << qreal(1);
    }
    void idleLinesStopRequestingFrames()
    {
        QFETCH(bool, transparent);
        QFETCH(qreal, thickness);
        auto source = document(thickness, 0, 0, false, true);
        if (transparent) source.replace("table.borderColor: \"red\"", "table.borderColor: \"transparent\"");
        auto host = open(source);
        QVERIFY(host);
        QTRY_VERIFY(named(host->view, "tableHorizontalLine").size() > 8);
        QTest::qWait(100);
        QSignalSpy frames(host->window.get(), &QQuickWindow::afterAnimating);
        QTest::qWait(150);
        QVERIFY2(frames.size() <= 1, "idle disabled lines must not keep scheduling frames");
    }

    void tableJoinsStayContinuous_data()
    {
        QTest::addColumn<qreal>("thickness");
        QTest::newRow("subpixel") << 0.2;
        QTest::newRow("normal") << 1.0;
        QTest::newRow("thick") << 2.0;
    }
    void tableJoinsStayContinuous()
    {
        QFETCH(qreal, thickness);
        auto source = document(thickness, 1, 1, false, true);
        source.replace("table.borderColor: \"red\"", "table.borderColor: \"#80ff0000\"");
        auto host = open(source);
        QVERIFY(host);
        QTRY_VERIFY(named(host->view, "tableHorizontalLine").size() > 8);
        const qreal dpr = host->window->effectiveDevicePixelRatio();
        for (qreal offset : {0.0, 0.25, 0.5, 0.75, 1.0}) {
            host->viewport->setProperty("contentY", offset);
            QTest::qWait(30);
            const auto image = host->window->grabWindow();
            const auto tops = named(host->view, "tableHorizontalLine");
            const auto bottoms = named(host->view, "tableBottomLine");
            const int top = qRound(tops.first()->mapToScene(QPointF()).y() * dpr);
            const int bottom = qRound(bottoms.last()->mapToScene(QPointF()).y() * dpr)
                + expected(thickness, dpr);
            // Every column border crosses all row joins, including the outer corners.
            for (auto *line : named(host->view, "tableVerticalLine")) {
                const int x = qRound(line->mapToScene(QPointF()).x() * dpr);
                for (int y = top; y < bottom; ++y) {
                    const auto pixel = image.pixel(x, y);
                    QVERIFY2(qRed(pixel) >= 250 && qAbs(qGreen(pixel) - 127) <= 2
                                 && qAbs(qBlue(pixel) - 127) <= 2,
                             qPrintable(QString("gap or overlap at %1,%2, offset %3: %4")
                                 .arg(x).arg(y).arg(offset).arg(QColor(pixel).name())));
                }
            }
        }
    }

    void equalThicknessAtEveryOffset_data()
    {
        QTest::addColumn<qreal>("contentY");
        QTest::addColumn<bool>("aligned");
        QTest::addColumn<bool>("fractionalOrigin");
        for (qreal y : {0.0, 0.25, 0.5, 0.75, 1.0, 37.3, 101.6})
            for (bool aligned : {false, true})
                for (bool fractional : {false, true})
                    QTest::newRow(qPrintable(QString("y%1-%2-%3").arg(y).arg(aligned ? "aligned" : "smooth").arg(fractional ? "frac" : "int")))
                        << y << aligned << fractional;
    }
    void equalThicknessAtEveryOffset()
    {
        QFETCH(qreal, contentY); QFETCH(bool, aligned); QFETCH(bool, fractionalOrigin);
        auto host = open(document(1, 1, 1, aligned, fractionalOrigin));
        QVERIFY(host);
        const qreal dpr = host->window->effectiveDevicePixelRatio();
        QTRY_VERIFY(named(host->view, "tableHorizontalLine").size() > 8);
        host->viewport->setProperty("contentY", contentY);
        verifyThickness(*host, expected(1, dpr), expected(1, dpr), expected(1, dpr));
    }

    void scrollingRepaintsSnappedLines()
    {
        auto host = open(document(1, 1, 1, false, true));
        QVERIFY(host);
        const qreal dpr = host->window->effectiveDevicePixelRatio();
        QTRY_VERIFY(named(host->view, "tableHorizontalLine").size() > 8);
        for (qreal y = 0; y < 12; y += 0.37) {
            host->viewport->setProperty("contentY", y);
            verifyThickness(*host, expected(1, dpr), expected(1, dpr), expected(1, dpr));
        }
    }

    void intentionalThicknessesStayDistinct()
    {
        auto host = open(document(2, 3, 1, false, true));
        QVERIFY(host);
        const qreal dpr = host->window->effectiveDevicePixelRatio();
        QTRY_VERIFY(named(host->view, "tableHorizontalLine").size() > 8);
        host->viewport->setProperty("contentY", 0.5);
        verifyThickness(*host, expected(2, dpr), expected(3, dpr), expected(1, dpr));
    }

    void disabledAndSubPixelLines()
    {
        {
            auto host = open(document(0, 0, 0, false, true));
            QVERIFY(host);
            QTRY_VERIFY(named(host->view, "tableHorizontalLine").size() > 8);
            QTest::qWait(30);
            const QImage image = host->window->grabWindow();
            for (int y = 0; y < image.height(); ++y)
                for (int x = 0; x < image.width(); ++x)
                    QVERIFY2(!isRed(image.pixel(x, y)), "disabled lines must not paint");
        }
        {
            // A positive thickness never vanishes: 0.2 logical px still paints one device pixel.
            auto host = open(document(0.2, 0.2, 0.2, false, true));
            QVERIFY(host);
            QTRY_VERIFY(named(host->view, "tableHorizontalLine").size() > 8);
            verifyThickness(*host, 1, 1, 1);
        }
    }

    void layoutIsUnchangedBySnapping()
    {
        auto host = open(document(1, 1, 1, false, true));
        QVERIFY(host);
        QTRY_VERIFY(named(host->view, "tableHorizontalLine").size() > 8);
        for (auto *line : named(host->view, "tableHorizontalLine")) {
            QCOMPARE(line->height() == 0 || line->height() == 1.0, true);
            QCOMPARE(line->boundingRect().size(), QSizeF(line->width(), line->height()));
        }
    }
};
QTEST_MAIN(LinesTest)
#include "tst_lines.moc"
