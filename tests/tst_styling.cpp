#include "markdownstyle.h"
#include "private/formattedtext.h"
#include "private/inline.h"
#include "private/document.h"
#include <QGuiApplication>
#include <QQuickWindow>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlProperty>
#include <QSignalSpy>
#include <QPainter>
#include <QElapsedTimer>
#include <QDir>
#include <QTemporaryDir>
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#include <limits>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

class StylingTest : public QObject
{
    Q_OBJECT
    static QList<QQuickItem *> named(QQuickItem *item, const QString &name)
    {
        QList<QQuickItem *> result;
        if (item->objectName() == name) result.append(item);
        for (auto *child : item->childItems()) result.append(named(child, name));
        return result;
    }
    static QQuickItem *textItem(QQuickItem *item, const QString &text)
    {
        if (item->property("text").toString() == text) return item;
        for (auto *child : item->childItems()) if (auto *found = textItem(child, text)) return found;
        return nullptr;
    }
private slots:
    void selectableTextRetainsEdgeContrast()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(
            import QtQuick
            import QMarkdown
            MarkdownView {
                width: 600
                selectionEnabled: false
                markdown: "Ordinary text should stay crisp when selection is enabled."
                style: MarkdownStyle {
                    bodyFont.family: "DejaVu Sans"
                    bodyFont.pixelSize: 20
                    bodyColor: "black"
                }
            }
        )", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(640, 140); window.setColor(Qt::white);
        view->setParentItem(window.contentItem());
        // Both paths occupy the same physical pixels, including at 125% DPR.
        view->setPosition(QPointF(16, 16)); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        const auto source = view->property("markdown").toString();
        QTRY_VERIFY(textItem(view, source));
        auto *plain = textItem(view, source);
        QVERIFY(!qobject_cast<FormattedText *>(plain));
        const auto font = plain->property("font").value<QFont>();
        const auto reference = window.grabWindow(); QVERIFY(!reference.isNull());
        view->setProperty("selectionEnabled", true);
        QTRY_VERIFY(qobject_cast<FormattedText *>(textItem(view, source)));
        auto *selectable = qobject_cast<FormattedText *>(textItem(view, source));
        QTRY_VERIFY(selectable->logicalHeight() > 0);
        QCOMPARE(selectable->font(), font);
        const auto selected = window.grabWindow(); QVERIFY(!selected.isNull());
        QCOMPARE(selected.size(), reference.size());
        // Sum squared luminance gradients: additional texture filtering lowers
        // glyph-edge contrast even when glyph geometry and ink remain similar.
        const auto contrast = [](const QImage &image) {
            double sum = 0;
            for (int y = 1; y < image.height(); ++y)
                for (int x = 1; x < image.width(); ++x) {
                    const int value = qGray(image.pixel(x, y));
                    const int dx = value - qGray(image.pixel(x - 1, y));
                    const int dy = value - qGray(image.pixel(x, y - 1));
                    sum += dx * dx + dy * dy;
                }
            return sum;
        };
        const double referenceContrast = contrast(reference);
        QVERIFY(referenceContrast > 0);
        const double ratio = contrast(selected) / referenceContrast;
        qInfo("Selectable/reference edge contrast: %.3f", ratio);
        QVERIFY2(ratio >= .8, qPrintable(QString("Edge contrast ratio %1 is below 0.8").arg(ratio)));
    }
    void decorationPlacement_data()
    {
        QTest::addColumn<bool>("strike");
        QTest::addColumn<bool>("gfm");
        QTest::addColumn<int>("alignment");
        QTest::addColumn<int>("width");
        QTest::addColumn<qreal>("padding");
        QTest::addColumn<bool>("bidi");
        QTest::addColumn<qreal>("scale");
        QTest::addColumn<qreal>("scaleX");
        QTest::addColumn<bool>("pointSize");
        for (bool strike : {false, true}) {
            for (bool gfm : {false, true}) {
                if (strike && !gfm) continue;
                for (int alignment : {int(Qt::AlignLeft), int(Qt::AlignHCenter), int(Qt::AlignRight)})
                    for (int width : {180, 800})
                        for (qreal padding : {0., 4.})
                            for (bool bidi : {false, true})
                                for (bool pointSize : {false, true})
                                for (const auto scales : {QPointF(1, 1), QPointF(1.25, 1.25), QPointF(1.5, 1.5), QPointF(2, 2),
                                                         QPointF(1.250814, 1.25), QPointF(1001. / 800, 376. / 300)})
                                    QTest::newRow(qPrintable(QString("%1-%2-%3-%4-%5-%6-%7-%8")
                                        .arg(strike).arg(alignment).arg(width).arg(padding).arg(bidi).arg(gfm).arg(scales.x()).arg(pointSize)))
                                        << strike << gfm << alignment << width << padding << bidi << scales.y() << scales.x() << pointSize;
            }
        }
    }
    void decorationPlacement()
    {
        QFETCH(bool, strike); QFETCH(bool, gfm); QFETCH(int, alignment); QFETCH(int, width);
        QFETCH(qreal, padding); QFETCH(bool, bidi); QFETCH(qreal, scale); QFETCH(qreal, scaleX); QFETCH(bool, pointSize);
        FormattedText item;
        QFont font("DejaVu Sans"), codeFont("DejaVu Sans Mono");
        if (pointSize) { font.setPointSizeF(17.5); codeFont.setPointSizeF(15.5); }
        else { font.setPixelSize(24); codeFont.setPixelSize(20); }
        item.setFont(font); item.setCodeFont(codeFont);
        QString source = bidi ? QString::fromUtf8("prefix `code` אבגד **bold** *italic* `code` [link](target) remaining")
                              : QString("prefix `code` text **bold** *italic* `code` [link](target) remaining");
        if (strike) source = "~~" + source + "~~";
        const auto blocks = QMarkdownPrivate::parse(source, QMarkdownPrivate::ParseOptions{gfm, gfm, gfm, gfm, gfm});
        QCOMPARE(blocks.size(), 1);
        const auto &block = blocks.first();
        const QString text = block.text;
        item.setText(text); item.setLayoutWidth(width); item.setHorizontalAlignment(alignment);
        item.setCodeHorizontalPadding(padding); item.setCodeVerticalPadding(padding ? 2 : 0);
        QVariantList ranges, codes, links;
        for (const auto &range : block.ranges)
            ranges.append(QVariantMap{{"start", range.start}, {"length", range.length},
                {"flags", range.flags & ~QMarkdownPrivate::Strikethrough}});
        for (const auto &code : block.codes)
            codes.append(QVariantMap{{"start", code.start}, {"length", code.length}});
        for (const auto &link : block.links)
            links.append(QVariantMap{{"start", link.start}, {"length", link.length}, {"destination", link.destination}});
        QCOMPARE(links.size(), 1);
        const int link = block.links.first().start;
        const int linkLength = block.links.first().length;
        item.setCodeSpans(codes); item.setLinkSpans(links);
        item.setLinkUnderline(false);
        item.setFormatRanges(ranges); item.ensurePolished();
        auto raster = [&] {
            QImage image(qCeil(850 * scale), qCeil(300 * scale), QImage::Format_ARGB32_Premultiplied);
            image.setDevicePixelRatio(scale); image.fill(Qt::transparent);
            QPainter painter(&image);
            painter.scale(scaleX / scale, 1);
            item.paint(&painter);
            return image;
        };
        const QImage plain = raster();
        if (strike) {
            ranges.append(QVariantMap{{"start", 0}, {"length", text.size()}, {"flags", QMarkdownPrivate::Strikethrough}});
            item.setFormatRanges(ranges);
        } else item.setLinkUnderline(true);
        item.ensurePolished();
        const QImage decorated = raster();
        const int start = strike ? 0 : link, end = strike ? text.size() : link + linkLength;
        QVector<QRectF> expected = item.rangeRectangles(start, end);
        if (padding == 0) {
            // Wrapped bidi trailing spaces can have native glyph advances past
            // the cursor cells clamped to the assigned line width.
            expected.clear();
            for (int i = 0; i < item.layout()->lineCount(); ++i) {
                const auto line = item.layout()->lineAt(i);
                const int from = qMax(start, line.textStart());
                const int length = qMin(end, line.textStart() + line.textLength()) - from;
                if (length <= 0) continue;
                for (const auto &run : line.glyphRuns(from, length)) {
                    const auto positions = run.positions();
                    const auto advances = run.rawFont().advancesForGlyphIndexes(run.glyphIndexes());
                    for (int glyph = 0; glyph < positions.size(); ++glyph)
                        expected.append(QRectF(positions[glyph].x(), line.y(), advances[glyph].x(), line.height()));
                }
            }
        }
        if (strike) {
            // Cursor rectangles can be disjoint at bidi boundaries. Strikeout
            // also crosses the intervening visual advance on the same line.
            QVector<QRectF> lines;
            for (const auto &rect : expected) {
                bool merged = false;
                for (auto &line : lines) if (qAbs(line.center().y() - rect.center().y()) < 2) {
                    line = line.united(rect); merged = true; break;
                }
                if (!merged) lines.append(rect);
            }
            expected = lines;
        }
        // Bands come from the shaped fallback font metrics and baseline, not
        // the renderer's cached decoration geometry or whole character cells.
        QVector<QRectF> bands;
        for (int i = 0; i < item.layout()->lineCount(); ++i) {
            const auto line = item.layout()->lineAt(i);
            for (const auto &run : line.glyphRuns()) {
                if (run.positions().isEmpty()) continue;
                const auto raw = run.rawFont();
                const qreal y = run.positions().first().y()
                    + (strike ? -raw.ascent() / 3 : raw.underlinePosition());
                const qreal half = raw.lineThickness() / 2 + 1 / scale;
                bands.append(QRectF(0, y - half, 2000, 2 * half));
            }
        }
        QVector<QPair<QLineF, qreal>> gutterPaths;
        if (strike && padding > 0) {
            QHash<int, QPointF> metrics; // native strikeout y and pen thickness
            for (int i = 0; i < item.layout()->lineCount(); ++i)
                for (const auto &run : item.layout()->lineAt(i).glyphRuns(-1, -1,
                         QTextLayout::RetrieveGlyphPositions | QTextLayout::RetrieveStringIndexes)) {
                    for (int glyph = 0; glyph < run.stringIndexes().size(); ++glyph)
                        metrics.insert(int(run.stringIndexes()[glyph]),
                            QPointF(run.positions()[glyph].y() - run.rawFont().ascent() / 3, run.rawFont().lineThickness()));
                }
            for (const auto &code : block.codes)
                for (const auto &rect : item.rangeRectangles(code.start, code.start + code.length))
                    for (bool left : {true, false}) {
                        const qreal edge = left ? rect.left() : rect.right();
                        int codeSource = -1, neighbor = -1;
                        for (int pos = 0; pos < text.size(); ++pos) {
                            if (!metrics.contains(pos)) continue;
                            for (const auto &cell : item.rangeRectangles(pos, pos + 1)) {
                                if (qAbs(cell.center().y() - rect.center().y()) >= .01) continue;
                                const bool inCode = pos >= code.start && pos < code.start + code.length;
                                if (inCode && qAbs((left ? cell.left() : cell.right()) - edge) < .01) codeSource = pos;
                                if (!inCode && qAbs((left ? cell.right() : cell.left()) - edge) < .01) neighbor = pos;
                            }
                        }
                        if (codeSource < 0 || neighbor < 0) continue;
                        gutterPaths.append({QLineF(edge, metrics[neighbor].x(),
                            edge + (left ? padding : -padding), metrics[codeSource].x()), metrics[codeSource].y()});
                    }
        }
        int differences = 0;
        for (int y = 0; y < plain.height(); ++y) for (int x = 0; x < plain.width(); ++x) {
            if (qAbs(plain.pixelColor(x, y).alpha() - decorated.pixelColor(x, y).alpha()) < 20) continue;
            ++differences;
            bool inside = false;
            for (const auto &rect : expected)
                if (rect.adjusted(-2, -2, 2, 2).contains(QPointF((x + .5) / scaleX, (y + .5) / scale) - item.paintOffset())) inside = true;
            bool inBand = false;
            for (const auto &band : bands)
                if (band.contains(QPointF((x + .5) / scaleX, (y + .5) / scale) - item.paintOffset())) inBand = true;
            const QPointF point = QPointF((x + .5) / scaleX, (y + .5) / scale) - item.paintOffset();
            for (const auto &path : gutterPaths) {
                const auto line = path.first;
                const qreal t = (point.x() - line.x1()) / line.dx();
                if (t >= 0 && t <= 1 && qAbs(point.y() - line.pointAt(t).y()) <= path.second / 2 + 1 / scale)
                    inBand = true;
            }
            QVERIFY2(inBand, qPrintable(QString("Decoration outside metric band at pixel %1,%2").arg(x).arg(y)));
            QVERIFY2(inside, qPrintable(QString("Decoration outside range at pixel %1,%2").arg(x).arg(y)));
        }
        QVERIFY(differences > 20);
        // Each non-space character must receive decoration, including every
        // styled portion and wrapped/bidirectional continuation.
        for (int pos = start; pos < end; ++pos) {
            if (text[pos].isSpace()) continue;
            const auto cells = item.rangeRectangles(pos, pos + 1);
            if (cells.isEmpty()) continue; // Qt may expose no cell at a bidi boundary.
            int changed = 0;
            for (const auto &rect : cells) {
                const auto logical = rect.translated(item.paintOffset());
                const QRect pixels = QRectF(QPointF(logical.left() * scaleX, logical.top() * scale),
                    QSizeF(logical.width() * scaleX, logical.height() * scale))
                    .toAlignedRect().intersected(plain.rect());
                for (int y = pixels.top(); y <= pixels.bottom(); ++y)
                    for (int x = pixels.left(); x <= pixels.right(); ++x)
                        if (qAbs(plain.pixelColor(x, y).alpha() - decorated.pixelColor(x, y).alpha()) >= 20) ++changed;
            }
            QVERIFY2(changed > 0, qPrintable(QString("Missing decoration at character %1").arg(pos)));
        }
        if (strike && padding > 0) {
            QSet<int> shapedSources;
            for (int i = 0; i < item.layout()->lineCount(); ++i)
                for (const auto &run : item.layout()->lineAt(i).glyphRuns(-1, -1, QTextLayout::RetrieveStringIndexes))
                    for (auto source : run.stringIndexes()) shapedSources.insert(int(source));
            for (const auto &code : block.codes) {
                for (const auto &rect : item.rangeRectangles(code.start, code.start + code.length)) {
                    for (bool left : {true, false}) {
                        const qreal edge = left ? rect.left() : rect.right();
                        bool neighbor = false;
                        for (int pos = 0; pos < text.size(); ++pos) {
                            // Qt omits trailing whitespace glyphs at some wraps;
                            // those cells have no existing strikeout endpoint.
                            if (!shapedSources.contains(pos) || (pos >= code.start && pos < code.start + code.length)) continue;
                            for (const auto &cell : item.rangeRectangles(pos, pos + 1))
                                if (qAbs(cell.center().y() - rect.center().y()) < .01
                                    && qAbs((left ? cell.right() : cell.left()) - edge) < .01) neighbor = true;
                        }
                        if (!neighbor) continue; // Outer line padding has no adjacent struck text.
                        const qreal inner = edge + (left ? padding : -padding);
                        for (int x = qCeil((qMin(edge, inner) + item.paintOffset().x()) * scaleX + 1);
                             x < (qMax(edge, inner) + item.paintOffset().x()) * scaleX - 1; ++x) {
                            int covered = 0;
                            for (int y = qMax(0, qFloor((rect.top() + item.paintOffset().y()) * scale));
                                 y < qMin(plain.height(), qCeil((rect.bottom() + item.paintOffset().y()) * scale)); ++y)
                                if (qAbs(plain.pixelColor(x, y).alpha() - decorated.pixelColor(x, y).alpha()) >= 20) ++covered;
                            QVERIFY2(covered > 0, left ? "Missing wrapped/bidi left gutter" : "Missing wrapped/bidi right gutter");
                        }
                    }
                }
            }
        }
        item.setLinkUnderline(false);
        if (strike) { ranges.removeLast(); item.setFormatRanges(ranges); }
        item.ensurePolished();
        QCOMPARE(raster(), plain);
    }
    void strikeoutGutters_data()
    {
        QTest::addColumn<qreal>("padding");
        QTest::addColumn<QPointF>("scale");
        QTest::addColumn<int>("mode");
        for (qreal padding : {0., 4., 12.})
            for (auto scale : {QPointF(1, 1), QPointF(1.25, 1.25), QPointF(1.5, 1.5),
                               QPointF(2, 2), QPointF(1.250814, 1.25)})
                for (int mode = 0; mode < 7; ++mode)
                    QTest::newRow(qPrintable(QString("%1-%2-%3").arg(padding).arg(scale.x()).arg(mode)))
                        << padding << scale << mode;
    }
    void strikeoutGutters()
    {
        QFETCH(qreal, padding); QFETCH(QPointF, scale); QFETCH(int, mode);
        FormattedText item;
        QFont body("DejaVu Sans"), code("DejaVu Sans Mono");
        body.setPixelSize(28); code.setPixelSize(18);
        item.setFont(body); item.setCodeFont(code);
        item.setText("body CODE tail"); item.setLayoutWidth(600);
        item.setCodeHorizontalPadding(padding); item.setCodeVerticalPadding(2);
        item.setCodeSpans({QVariantMap{{"start", 5}, {"length", 4}}});
        QVariantList ranges{QVariantMap{{"start", 5}, {"length", 4}, {"flags", QMarkdownPrivate::Code}}};
        item.setFormatRanges(ranges); item.ensurePolished();
        auto raster = [&] {
            QImage image(qCeil(650 * scale.x()), qCeil(100 * scale.y()), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            QPainter painter(&image); painter.scale(scale.x(), scale.y()); item.paint(&painter);
            return image;
        };
        const auto plain = raster();
        // Continuous, isolated, beginning/ending on code, separated spans,
        // inherited strikeout, and linked code with its own decoration color.
        const int start = mode == 1 || mode == 2 ? 5 : 0;
        const int end = mode == 1 || mode == 3 ? 9 : 14;
        if (mode == 5) { body.setStrikeOut(true); item.setFont(body); }
        else {
            ranges.append(QVariantMap{{"start", start}, {"length", end - start}, {"flags", QMarkdownPrivate::Strikethrough}});
            if (mode == 4) ranges = {ranges.first(),
                QVariantMap{{"start", 0}, {"length", 4}, {"flags", QMarkdownPrivate::Strikethrough}},
                QVariantMap{{"start", 5}, {"length", 4}, {"flags", QMarkdownPrivate::Code | QMarkdownPrivate::Strikethrough}},
                QVariantMap{{"start", 10}, {"length", 4}, {"flags", QMarkdownPrivate::Strikethrough}}};
            item.setFormatRanges(ranges);
        }
        if (mode == 6) {
            item.setLinkSpans({QVariantMap{{"start", 5}, {"length", 4}, {"destination", "target"}}});
            item.setLinkUnderline(false);
        }
        item.ensurePolished();
        const auto decorated = raster();
        const auto rects = item.rangeRectangles(5, 9); QCOMPARE(rects.size(), 1);
        const QRectF rect = rects.first();
        const auto line = item.layout()->lineAt(0);
        const qreal baseline = line.y() + line.ascent();
        const qreal bodyY = baseline - QRawFont::fromFont(body).ascent() / 3;
        const qreal codeY = baseline - QRawFont::fromFont(code).ascent() / 3;
        for (bool left : {true, false}) {
            const bool connected = mode == 0 || mode == 5 || mode == 6
                || (left ? mode == 3 : mode == 2);
            const qreal outer = left ? rect.left() : rect.right();
            const qreal inner = outer + (left ? padding : -padding);
            // Inspect every physical column strictly inside each padding gutter.
            for (int x = qCeil((qMin(inner, outer) + item.paintOffset().x()) * scale.x() + 1);
                 x < (qMax(inner, outer) + item.paintOffset().x()) * scale.x() - 1; ++x) {
                const qreal logicalX = (x + .5) / scale.x() - item.paintOffset().x();
                const qreal t = (logicalX - outer) / (inner - outer);
                const qreal expectedY = bodyY + t * (codeY - bodyY);
                int coverage = 0;
                for (int y = 0; y < decorated.height(); ++y) {
                    if (qAbs(decorated.pixelColor(x, y).alpha() - plain.pixelColor(x, y).alpha()) < 20) continue;
                    const qreal logicalY = (y + .5) / scale.y() - item.paintOffset().y();
                    QVERIFY2(connected, "Strikeout extended into an unconnected outer gutter");
                    QVERIFY2(qAbs(logicalY - expectedY) < 1.5 / scale.y() + 1,
                             "Gutter strikeout left its interpolated native-metric path");
                    if (mode == 6) QVERIFY(decorated.pixelColor(x, y).blue() > decorated.pixelColor(x, y).red());
                    ++coverage;
                }
                if (connected) QVERIFY2(coverage > 0, left ? "Missing left gutter strikeout" : "Missing right gutter strikeout");
            }
        }
    }
    void adjacentCodeStrikeout()
    {
        for (auto scale : {QPointF(1, 1), QPointF(1.25, 1.25), QPointF(1.5, 1.5),
                           QPointF(2, 2), QPointF(1.250814, 1.25)}) {
            FormattedText item;
            QFont font("DejaVu Sans Mono"); font.setPixelSize(24);
            item.setFont(font); item.setCodeFont(font);
            item.setText("CODECODE"); item.setLayoutWidth(400);
            item.setCodeHorizontalPadding(12); item.setCodeForeground(Qt::red);
            item.setCodeSpans({QVariantMap{{"start", 0}, {"length", 4}},
                               QVariantMap{{"start", 4}, {"length", 4}}});
            item.setLinkSpans({QVariantMap{{"start", 4}, {"length", 4}, {"destination", "target"}}});
            item.setLinkUnderline(false); item.setLinkColor(Qt::blue);
            item.setFormatRanges({QVariantMap{{"start", 0}, {"length", 8},
                {"flags", QMarkdownPrivate::Code | QMarkdownPrivate::Strikethrough}}});
            item.ensurePolished();
            QImage image(qCeil(450 * scale.x()), qCeil(80 * scale.y()), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            { QPainter painter(&image); painter.scale(scale.x(), scale.y()); item.paint(&painter); }
            const auto left = item.rangeRectangles(0, 4).first();
            const auto right = item.rangeRectangles(4, 8).first();
            const qreal boundary = left.right();
            QVERIFY(qAbs(boundary - right.left()) < .01);
            const auto line = item.layout()->lineAt(0);
            const qreal y = line.y() + line.ascent() - QRawFont::fromFont(font).ascent() / 3;
            for (int px = qCeil((boundary - 12 + item.paintOffset().x()) * scale.x() + 1);
                 px < (boundary + 12 + item.paintOffset().x()) * scale.x() - 1; ++px) {
                int covered = 0;
                for (int py = qFloor((y - 2 + item.paintOffset().y()) * scale.y());
                     py <= qCeil((y + 2 + item.paintOffset().y()) * scale.y()); ++py) {
                    const auto color = image.pixelColor(px, py);
                    if (color.alpha() < 20) continue;
                    const qreal x = (px + .5) / scale.x() - item.paintOffset().x();
                    if (x < boundary - 1 / scale.x()) QVERIFY(color.red() > color.blue());
                    if (x > boundary + 1 / scale.x()) QVERIFY(color.blue() > color.red());
                    ++covered;
                }
                QVERIFY2(covered > 0, "Missing strikeout between adjacent code fragments");
            }
        }
    }
    void separatedDecorationEndpoints_data()
    {
        QTest::addColumn<int>("decoration");
        for (int decoration = 0; decoration < 4; ++decoration)
            QTest::newRow(qPrintable(QString::number(decoration))) << decoration;
    }
    void separatedDecorationEndpoints()
    {
        QFETCH(int, decoration);
        FormattedText item;
        QFont font("DejaVu Sans"); font.setPixelSize(24);
        item.setFont(font); item.setCodeFont(font);
        item.setText("code first intervening second tail");
        item.setLayoutWidth(800);
        const QVariantList plainRanges{QVariantMap{{"start", 0}, {"length", 4}, {"flags", QMarkdownPrivate::Code}}};
        item.setFormatRanges(plainRanges); item.ensurePolished();
        const qreal sx = 1.250814, sy = 1.25;
        auto raster = [&] {
            QImage image(1100, 100, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            QPainter painter(&image); painter.scale(sx, sy); item.paint(&painter);
            return image;
        };
        const auto plain = raster();
        // Identical font/color/shift can be merged by Qt into one glyph run.
        // Effective interval boundaries must still keep the two lines apart.
        const int first = 5, second = 23;
        if (decoration == 0) {
            auto ranges = plainRanges;
            for (int start : {first, second})
                ranges.append(QVariantMap{{"start", start}, {"length", start == first ? 5 : 6},
                    {"flags", QMarkdownPrivate::Strikethrough}});
            item.setFormatRanges(ranges);
        } else {
            // Underline and overline inherited from a body font also cross the
            // code boundary. Test their native endpoints over the whole line.
            font.setUnderline(decoration == 1 || decoration == 3); font.setOverline(decoration == 2);
            item.setFont(font); item.setCodeFont(font);
            if (decoration == 3) {
                item.setLinkSpans({QVariantMap{{"start", 10}, {"length", 13}, {"destination", "target"}}});
                item.setLinkColor(Qt::black); item.setLinkUnderline(false);
            }
        }
        item.ensurePolished();
        const auto decorated = raster();
        const auto line = item.layout()->lineAt(0);
        const auto raw = line.glyphRuns().first().rawFont();
        const qreal baseline = line.position().y() + line.ascent();
        const qreal y = baseline + (decoration == 0 ? -raw.ascent() / 3
            : decoration == 1 || decoration == 3 ? raw.underlinePosition() : -raw.ascent());
        QVector<QRectF> bands;
        const auto positions = decoration == 0 ? QList<int>{first, second}
            : decoration == 3 ? QList<int>{0, second} : QList<int>{0};
        for (int start : positions) {
            const int length = decoration == 0 ? (start == first ? 5 : 6)
                : decoration == 3 ? (start == 0 ? 10 : item.text().size() - second) : item.text().size();
            // Native shaped cursor advances plus the known two code gutters
            // provide an oracle independent of the renderer's cached fragments.
            const qreal left = line.cursorToX(start) + item.codeHorizontalPadding() * (start == 0 ? 1 : 2);
            const qreal right = line.cursorToX(start + length) + 2 * item.codeHorizontalPadding();
            bands.append(QRectF(left, y - raw.lineThickness() / 2,
                right - left, raw.lineThickness()).translated(item.paintOffset()));
        }
        int differences = 0;
        for (int py = 0; py < plain.height(); ++py) for (int px = 0; px < plain.width(); ++px) {
            if (qAbs(plain.pixelColor(px, py).alpha() - decorated.pixelColor(px, py).alpha()) < 20) continue;
            ++differences;
            bool inside = false;
            for (const auto &band : bands)
                if (band.adjusted(-1 / sx, -1 / sy, 1 / sx, 1 / sy)
                    .contains(QPointF((px + .5) / sx, (py + .5) / sy))) inside = true;
            QVERIFY2(inside, qPrintable(QString("Decoration outside advance/metric bounds at %1,%2").arg(px).arg(py)));
        }
        QVERIFY(differences > 20);
    }
    void paddingPreparationMeasurements()
    {
        QString text; QVariantList codes, ranges;
        for (int i=0; i<1000; ++i) {
            const int start=text.size(); text += "word code more ";
            codes.append(QVariantMap{{"start",start+5},{"length",4}});
            ranges.append(QVariantMap{{"start",start+5},{"length",4},{"flags",QMarkdownPrivate::Code}});
        }
        FormattedText item; QFont font("DejaVu Sans"); font.setPixelSize(16);
        item.setFont(font); item.setCodeFont(font); item.setText(text); item.setLayoutWidth(240);
        item.setFormatRanges(ranges); item.setCodeSpans(codes);
        for (qreal padding : {0.,4.}) {
            item.setCodeHorizontalPadding(padding); item.setCodeVerticalPadding(padding ? 2 : 0);
            QElapsedTimer timer; timer.start();
            for (int run=0; run<5; ++run) { item.setColor(run%2 ? Qt::black : Qt::darkGray); item.ensurePolished(); }
            QVERIFY(item.layout());
            qInfo() << "production padding preparation" << padding << "mean_ms" << timer.nsecsElapsed()/5.e6
                    << "characters" << text.size() << "spans" << codes.size() << "lines" << item.layout()->lineCount();
        }
    }
    void inlinePaddingDefaultsAndReset()
    {
        MarkdownStyle style;
        for (auto preset : {MarkdownStyle::Legacy, MarkdownStyle::PolishedLight, MarkdownStyle::PolishedDark,
                             MarkdownStyle::MinimalLight, MarkdownStyle::MinimalDark}) {
            style.setPreset(preset);
            QCOMPARE(style.inlineCode()->horizontalPadding(), 4.0);
            QCOMPARE(style.inlineCode()->verticalPadding(), 2.0);
        }
        auto *code = style.inlineCode();
        QSignalSpy horizontal(code, &InlineCodeStyle::horizontalPaddingChanged);
        QSignalSpy vertical(code, &InlineCodeStyle::verticalPaddingChanged);
        code->setHorizontalPadding(8); code->setHorizontalPadding(8);
        QCOMPARE(horizontal.size(), 1);
        style.setPreset(MarkdownStyle::Legacy); QCOMPARE(code->horizontalPadding(), 8.0);
        code->resetHorizontalPadding(); QCOMPARE(code->horizontalPadding(), 4.0);
        code->setVerticalPadding(-1); QCOMPARE(code->verticalPadding(), -1.0);
        code->setVerticalPadding(std::numeric_limits<qreal>::infinity()); QCOMPARE(vertical.size(), 2);
        code->resetVerticalPadding(); QCOMPARE(code->verticalPadding(), 2.0);
        code->setHorizontalPadding(12); style.resetOverrides(); QCOMPARE(code->horizontalPadding(), 4.0);
    }
    void inlinePaddingGeometry_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<int>("width");
        for (const QString &text : {QString(" "), QString("abc "), QString("firstsecond"), QString::fromUtf8("אבג abc אבג"),
                                   QString::fromUtf8("é🙂👩‍💻 words")})
            for (int width : {1, 32, 80, 220})
                QTest::newRow(qPrintable(text + QString::number(width))) << text << width;
    }
    void inlinePaddingGeometry()
    {
        QFETCH(QString, text); QFETCH(int, width);
        QQuickWindow window; window.resize(400, 400); window.show();
        FormattedText item(window.contentItem());
        QFont font("DejaVu Sans"); font.setPixelSize(16);
        item.setFont(font); item.setCodeFont(font); item.setText(text); item.setLayoutWidth(width);
        item.setFormatRanges({QVariantMap{{"start", 0}, {"length", text.size()}, {"flags", QMarkdownPrivate::Code}}});
        item.setCodeSpans({QVariantMap{{"start", 0}, {"length", text.size()}}});
        item.setLinkSpans({QVariantMap{{"start", 0}, {"length", text.size()}, {"destination", "target"}}});
        item.ensurePolished();
        QTRY_VERIFY(item.layout() && !item.codeRectangles().isEmpty());
        QCOMPARE(item.text(), text);
        QCOMPARE(item.layout()->text(), text);
        const qreal paddedHeight = item.logicalHeight();
        for (const auto &rect : item.codeRectangles()) {
            const QPointF offset(-item.x(), -item.y());
            QCOMPARE(item.linkAt(rect.left() + .1 + offset.x(), rect.center().y() + offset.y()), 0);
            QCOMPARE(item.linkAt(rect.right() - .1 + offset.x(), rect.center().y() + offset.y()), 0);
            QCOMPARE(item.linkAt(rect.center().x() + offset.x(), rect.top() + .1 + offset.y()), 0);
            QVERIFY(rect.bottom() <= paddedHeight + .01);
        }
        item.setCodeHorizontalPadding(0); item.setCodeVerticalPadding(0);
        QTRY_VERIFY(item.logicalHeight() < paddedHeight);
        QTextLayout baseline(text, font);
        baseline.setFormats(item.layout()->formats());
        QTextOption option; option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); baseline.setTextOption(option);
        baseline.beginLayout(); qreal height = 0; int count = 0;
        while (true) { auto line = baseline.createLine(); if (!line.isValid()) break; line.setLineWidth(width); height += line.height(); ++count; }
        baseline.endLayout(); QCOMPARE(item.logicalHeight(), height); QCOMPARE(item.layout()->lineCount(), count);
        item.setCodeHorizontalPadding(4); item.setCodeVerticalPadding(2);
        QTRY_COMPARE(item.logicalHeight(), paddedHeight);
        item.setFont(QFont("DejaVu Sans", 24)); item.setCodeFont(QFont("DejaVu Sans", 24)); QTRY_VERIFY(item.logicalHeight() != paddedHeight);
    }
    void codeFragmentWithPartialLink()
    {
        FormattedText item; QFont font("DejaVu Sans"); font.setPixelSize(20);
        item.setFont(font); item.setCodeFont(font); item.setText("abcDEF"); item.setLayoutWidth(200);
        item.setFormatRanges({QVariantMap{{"start",0},{"length",6},{"flags",QMarkdownPrivate::Code}}});
        item.setCodeSpans({QVariantMap{{"start",0},{"length",6}}});
        item.setLinkSpans({QVariantMap{{"start",3},{"length",3},{"destination","target"}}});
        item.setCodeForeground(Qt::red); item.setLinkColor(Qt::blue); item.ensurePolished();
        QCOMPARE(item.codeRectangles().size(),1); const auto rect=item.codeRectangles()[0];
        QCOMPARE(item.linkAt(rect.left()+1-item.x(),rect.center().y()-item.y()),-1);
        QCOMPARE(item.linkAt(rect.right()-1-item.x(),rect.center().y()-item.y()),0);
        QImage image(qCeil(item.width()),qCeil(item.height()),QImage::Format_ARGB32_Premultiplied); image.fill(Qt::transparent);
        { QPainter painter(&image); item.paint(&painter); }
        int red=0,blue=0;
        for(int y=0;y<image.height();++y) for(int x=0;x<image.width();++x) {
            const auto color=image.pixelColor(x,y);
            if(color.alpha()>100 && color.red()>200 && color.blue()<20) ++red;
            if(color.alpha()>100 && color.blue()>200 && color.red()<20) ++blue;
        }
        QVERIFY(red>10); QVERIFY(blue>10);
        item.setCodeHorizontalPadding(-1); item.setCodeVerticalPadding(std::numeric_limits<qreal>::quiet_NaN());
        QCOMPARE(item.codeHorizontalPadding(),0.0); QCOMPARE(item.codeVerticalPadding(),0.0);
    }
    void adjacentCodePaddingAndLinkBoundaries()
    {
        QQuickWindow window; window.show(); FormattedText item(window.contentItem());
        QFont font("DejaVu Sans"); font.setPixelSize(20); item.setFont(font); item.setCodeFont(font);
        item.setText("firstsecond"); item.setLayoutWidth(400);
        item.setFormatRanges({QVariantMap{{"start", 0}, {"length", 11}, {"flags", QMarkdownPrivate::Code}}});
        item.setCodeSpans({QVariantMap{{"start", 0}, {"length", 5}}, QVariantMap{{"start", 5}, {"length", 6}}});
        item.setLinkSpans({QVariantMap{{"start", 0}, {"length", 5}, {"destination", "first"}},
                           QVariantMap{{"start", 5}, {"length", 6}, {"destination", "second"}}});
        QTRY_VERIFY(item.layout() && item.codeRectangles().size() == 2);
        const auto rects = item.codeRectangles(); QVERIFY(rects[0].right() <= rects[1].left() + .01);
        for (int i = 0; i < 2; ++i) {
            QCOMPARE(item.linkAt(rects[i].left() + .1 - item.x(), rects[i].center().y() - item.y()), i);
            QCOMPARE(item.linkAt(rects[i].right() - .1 - item.x(), rects[i].center().y() - item.y()), i);
        }
        item.setHorizontalAlignment(Qt::AlignRight);
        QTRY_VERIFY(item.codeRectangles().last().right() > 399);
        item.setHorizontalAlignment(Qt::AlignHCenter);
        QTRY_VERIFY(std::abs((item.codeRectangles().first().left() + item.codeRectangles().last().right()) / 2 - 200) < .1);
    }

    void tableDefaultsOverridesAndReset()
    {
        MarkdownStyle style;
        for (const auto preset : {MarkdownStyle::Legacy, MarkdownStyle::PolishedLight, MarkdownStyle::PolishedDark,
                                  MarkdownStyle::MinimalLight, MarkdownStyle::MinimalDark}) {
            style.setPreset(preset); auto *table=style.table();
            QCOMPARE(table->headerForeground(),style.bodyColor());
            QCOMPARE(table->headerBackground(),QColor(Qt::transparent)); QCOMPARE(table->bodyBackground(),QColor(Qt::transparent));
            QCOMPARE(table->borderColor(),QColor(Qt::transparent)); QCOMPARE(table->borderWidth(),0.0);
            QCOMPARE(table->horizontalPadding(),4.0); QCOMPARE(table->verticalPadding(),2.0);
        }
        auto *table=style.table(); QSignalSpy changed(table,&TableStyle::horizontalPaddingChanged);
        table->setHorizontalPadding(12); table->setHorizontalPadding(12); QCOMPARE(changed.size(),1);
        style.setPreset(MarkdownStyle::Legacy); QCOMPARE(table->horizontalPadding(),12.0);
        table->resetHorizontalPadding(); QCOMPARE(table->horizontalPadding(),4.0); QCOMPARE(changed.size(),2);
        table->setHeaderForeground(Qt::red); style.setBodyColor(Qt::green); QCOMPARE(table->headerForeground(),QColor(Qt::red));
        table->resetHeaderForeground(); QCOMPARE(table->headerForeground(),QColor(Qt::green));
        table->setBodyBackground(Qt::blue); table->setBorderWidth(2); style.resetOverrides();
        QCOMPARE(table->bodyBackground(),QColor(Qt::transparent)); QCOMPARE(table->borderWidth(),0.0);
    }
    void defaultsAliasesAndNotifications()
    {
        MarkdownStyle style;
        QCOMPARE(style.preset(), MarkdownStyle::Legacy);
        auto *heading = style.headings()->h1();
        QCOMPARE(heading->font(), style.h1Font());
        QCOMPARE(heading->color(), style.h1Color());
        QVERIFY(!heading->separatorVisible());
        QCOMPARE(heading->separatorThickness(), 0.0);
        QCOMPARE(style.codeBlock()->backgroundColor(), QColor(Qt::transparent));
        QSignalSpy flat(&style, &MarkdownStyle::h1ColorChanged), grouped(heading, &HeadingStyle::colorChanged);
        heading->setColor(Qt::red);
        QCOMPARE(style.h1Color(), QColor(Qt::red));
        QCOMPARE(flat.count(), 1); QCOMPARE(grouped.count(), 1);
        style.setH1Color(Qt::blue);
        QCOMPARE(heading->color(), QColor(Qt::blue));
        QCOMPARE(flat.count(), 2); QCOMPARE(grouped.count(), 2);
        style.setH1Color(Qt::blue);
        QCOMPARE(flat.count(), 2); QCOMPARE(grouped.count(), 2);
        QFont font = style.inlineCodeFont(); font.setPixelSize(23);
        style.inlineCode()->setFont(font);
        QCOMPARE(style.inlineCodeFont(), font);
        QCOMPARE(style.inlineCodeFont().resolveMask(), font.resolveMask());
        style.setCodeBlockColor(Qt::cyan);
        QCOMPARE(style.codeBlock()->color(), QColor(Qt::cyan));
        style.codeBlock()->setColor(Qt::magenta);
        QCOMPARE(style.codeBlockColor(), QColor(Qt::magenta));
    }
    void allGroupedFontAliases()
    {
        MarkdownStyle style;
        for (int i = 1; i <= 6; ++i) {
            auto *group = style.headings()->property(qPrintable(QString("h%1").arg(i))).value<QObject *>();
            QVERIFY(group);
            const QByteArray flatName = QString("h%1Font").arg(i).toUtf8();
            QSignalSpy flat(&style, ("2h" + QByteArray::number(i) + "FontChanged()").constData());
            QSignalSpy grouped(group, SIGNAL(fontChanged()));
            QFont font = group->property("font").value<QFont>(); font.setPixelSize(30 + i);
            QVERIFY(group->setProperty("font", font));
            QCOMPARE(style.property(flatName).value<QFont>(), font);
            QCOMPARE(flat.count(), 1); QCOMPARE(grouped.count(), 1);
            font.setItalic(true); QVERIFY(style.setProperty(flatName, font));
            QCOMPARE(group->property("font").value<QFont>().resolveMask(), font.resolveMask());
            QCOMPARE(flat.count(), 2); QCOMPARE(grouped.count(), 2);
        }
        QSignalSpy flat(&style, &MarkdownStyle::inlineCodeFontChanged);
        QSignalSpy grouped(style.inlineCode(), &InlineCodeStyle::fontChanged);
        QFont overlay; overlay.setFamily("QMarkdown alias test");
        style.setInlineCodeFont(overlay);
        QCOMPARE(style.inlineCode()->font().resolveMask(), overlay.resolveMask());
        QCOMPARE(flat.count(), 1); QCOMPARE(grouped.count(), 1);
    }
    void sharedStyleDestruction()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.1\nItem { MarkdownStyle { id: shared; objectName: 'shared'; preset: MarkdownStyle.PolishedDark }"
            "MarkdownView { objectName: 'a'; width: 180; style: shared; markdown: '```\\ncode\\n```' }"
            "MarkdownView { objectName: 'b'; width: 180; style: shared; markdown: '```\\ncode\\n```' } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *host = qobject_cast<QQuickItem *>(object.get()); QVERIFY(host);
        auto *style = host->findChild<MarkdownStyle *>("shared"); QVERIFY(style);
        auto *a = host->findChild<QQuickItem *>("a"), *b = host->findChild<QQuickItem *>("b"); QVERIFY(a && b);
        QQuickWindow window; host->setParentItem(window.contentItem()); window.resize(500, 500); window.show();
        QTRY_COMPARE(named(a, "codeSurface").size(), 1);
        QTRY_COMPARE(named(b, "codeSurface").size(), 1);
        style->codeBlock()->setCornerRadius(10);
        QTRY_COMPARE(named(a, "codeSurface").first()->property("radius").toDouble(), 10.0);
        QTRY_COMPARE(named(b, "codeSurface").first()->property("radius").toDouble(), 10.0);
        QPointer<CodeBlockStyle> group(style->codeBlock());
        delete style; QVERIFY(group.isNull());
        QTRY_COMPARE(named(a, "codeSurface").first()->property("radius").toDouble(), 0.0);
        QTRY_COMPARE(named(b, "codeSurface").first()->property("radius").toDouble(), 0.0);
    }
    void presetsAndExplicitDefaults()
    {
        MarkdownStyle style;
        const auto font = style.bodyFont();
        style.setBodyColor(style.bodyColor());
        style.headings()->h1()->setSeparatorVisible(false);
        style.codeBlock()->setBorderWidth(0);
        style.inlineCode()->setBackgroundColor(Qt::transparent);
        style.setPreset(MarkdownStyle::PolishedDark);
        QCOMPARE(style.bodyColor(), QColor("#202020"));
        QVERIFY(!style.headings()->h1()->separatorVisible());
        QCOMPARE(style.codeBlock()->borderWidth(), 0.0);
        QCOMPARE(style.inlineCode()->backgroundColor(), QColor(Qt::transparent));
        QCOMPARE(style.h2Color(), QColor("#e6e6e6"));
        QCOMPARE(style.linkColor(), QColor("#8db9f2"));
        QCOMPARE(style.bodyFont(), font);
        style.resetOverrides();
        QCOMPARE(style.bodyColor(), QColor("#e6e6e6"));
        QVERIFY(style.headings()->h1()->separatorVisible());
        QCOMPARE(style.headings()->h2()->separatorSpacing(), 8.0);
        QCOMPARE(style.codeBlock()->borderWidth(), 1.0);
        QCOMPARE(style.inlineCode()->backgroundColor(), QColor("#30343b"));
        style.headings()->h1()->setSeparatorSpacing(22);
        style.headings()->h1()->resetSeparatorSpacing();
        QCOMPARE(style.headings()->h1()->separatorSpacing(), 8.0);
        style.setPreset(MarkdownStyle::MinimalLight);
        QCOMPARE(style.blockSpacing(), 8.0);
        QCOMPARE(style.thematicBreakColor(), QColor("#d0d7de"));
        QCOMPARE(style.bodyFont(), font);
        QVERIFY(!style.headings()->h1()->separatorVisible());
        QCOMPARE(style.codeBlock()->borderWidth(), 0.0);
        style.restoreDefaults();
        QCOMPARE(style.preset(), MarkdownStyle::Legacy);
        QCOMPARE(style.thematicBreakColor(), QColor("#202020"));
    }
    void declarationOrderAndFontEdits_data()
    {
        QTest::addColumn<QByteArray>("properties");
        QTest::newRow("preset-first") << QByteArray("preset: MarkdownStyle.PolishedDark; bodyColor: '#202020'; headings.h1.separatorVisible: false; codeBlock.borderWidth: 0; inlineCode.backgroundColor: 'transparent'");
        QTest::newRow("preset-last") << QByteArray("bodyColor: '#202020'; headings.h1.separatorVisible: false; codeBlock.borderWidth: 0; inlineCode.backgroundColor: 'transparent'; preset: MarkdownStyle.PolishedDark");
    }
    void declarationOrderAndFontEdits()
    {
        QFETCH(QByteArray, properties);
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.1\nMarkdownStyle { " + properties +
            "; headings.h1.font.pixelSize: 29; inlineCode.font.italic: true; function edit() { codeBlock.font.pixelSize = 31 } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *style = qobject_cast<MarkdownStyle *>(object.get()); QVERIFY(style);
        QCOMPARE(style->bodyColor(), QColor("#202020"));
        QVERIFY(!style->headings()->h1()->separatorVisible());
        QCOMPARE(style->codeBlock()->borderWidth(), 0.0);
        QCOMPARE(style->inlineCode()->backgroundColor(), QColor(Qt::transparent));
        QCOMPARE(style->h1Font().pixelSize(), 29);
        QVERIFY(style->inlineCodeFont().italic());
        QVERIFY(QMetaObject::invokeMethod(style, "edit"));
        QCOMPARE(style->codeBlockFont().pixelSize(), 31);
        style->setPreset(MarkdownStyle::MinimalLight);
        QCOMPARE(style->h1Font().pixelSize(), 29);
        QCOMPARE(style->codeBlockFont().pixelSize(), 31);
        QVERIFY(QQmlProperty(style, "codeBlock.font", &engine).reset());
        QVERIFY(style->codeBlockFont().pixelSize() != 31);
    }
    void resetPreservesBindingsAndOwnedGroups()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.1\nItem { id: host; property color ink: 'red'; property real pad: 33; MarkdownStyle { id: s; objectName: 'style'; headings.h1.color: host.ink; codeBlock.horizontalPadding: host.pad; preset: MarkdownStyle.PolishedDark } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *style = object->findChild<MarkdownStyle *>("style"); QVERIFY(style);
        QPointer<HeadingStyle> group(style->headings()->h1());
        QCOMPARE(style->h1Color(), QColor(Qt::red));
        style->resetOverrides();
        QCOMPARE(style->h1Color(), QColor("#e6e6e6"));
        QCOMPARE(style->codeBlock()->horizontalPadding(), 12.0);
        object->setProperty("ink", QColor(Qt::blue)); object->setProperty("pad", 7);
        QCOMPARE(style->h1Color(), QColor(Qt::blue));
        QCOMPARE(style->codeBlock()->horizontalPadding(), 7.0);
        style->setPreset(MarkdownStyle::MinimalLight);
        QCOMPARE(style->h1Color(), QColor(Qt::blue));
        QVERIFY(group == style->headings()->h1());
        object.reset(); QVERIFY(group.isNull());
    }
    void revisionedApi()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QMarkdown 1.0\nMarkdownStyle { preset: 1 }", {});
        QVERIFY(component.isError());
        QQmlComponent legacyComponent(&engine);
        legacyComponent.setData("import QMarkdown 1.0\nMarkdownStyle { bodyColor: 'red' }", {});
        QVERIFY2(legacyComponent.isReady(), qPrintable(legacyComponent.errorString()));
        std::unique_ptr<QObject> legacy(legacyComponent.create()); QVERIFY(legacy);
        QQmlComponent styledComponent(&engine);
        styledComponent.setData("import QMarkdown 1.1\nMarkdownStyle { preset: MarkdownStyle.PolishedLight }", {});
        QVERIFY2(styledComponent.isReady(), qPrintable(styledComponent.errorString()));
        std::unique_ptr<QObject> styled(styledComponent.create()); QVERIFY(styled);
        QQmlComponent padding(&engine);
        padding.setData("import QMarkdown 1.6\nMarkdownStyle { inlineCode.horizontalPadding: 8; inlineCode.verticalPadding: 3 }", {});
        QVERIFY2(padding.isReady(),qPrintable(padding.errorString()));
        std::unique_ptr<QObject> padded(padding.create()); QVERIFY(padded);
        auto *style=qobject_cast<MarkdownStyle *>(padded.get()); QVERIFY(style);
        QCOMPARE(style->inlineCode()->horizontalPadding(),8.0); QCOMPARE(style->inlineCode()->verticalPadding(),3.0);
    }
    void surfacesAndInvalidMetrics()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.1\nMarkdownView { width: 120; style: MarkdownStyle { preset: MarkdownStyle.PolishedLight } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(400, 800); window.show();
        view->setProperty("markdown", "# one\n\n## *two*\n\n### three\n\n#\n\n> ## quoted\n\n- # listed\n\n```\n  literal *x*  \n```\n\n```\n```\n\n    longlonglonglonglonglonglonglong\n\n> ~~~\n> nested quote\n> ~~~\n\n- item\n\n      nested list");
        QTRY_VERIFY(textItem(view, "one"));
        auto *style = view->property("style").value<MarkdownStyle *>(); QVERIFY(style);
        QTRY_COMPARE(named(view, "codeSurface").size(), 5);
        int visibleRules = 0;
        for (auto *rule : named(view, "headingSeparator")) if (rule->height() > 0) {
            ++visibleRules;
            QCOMPARE(rule->height(), 1.0);
            QVERIFY(rule->width() > 0);
            QCOMPARE(rule->property("color").value<QColor>(), QColor("#d0d7de"));
        }
        QCOMPARE(visibleRules, 5);
        auto *literal = textItem(view, "  literal *x*  "); QVERIFY(literal);
        auto *codeSurface = named(view, "codeSurface")[0];
        QVERIFY(codeSurface->isAncestorOf(literal));
        QCOMPARE(literal->mapToItem(codeSurface, QPointF()), QPointF(12, 12));
        QCOMPARE(literal->width(), 96.0);
        QCOMPARE(codeSurface->height(), literal->height() + 24);
        for (auto *surface : named(view, "codeSurface")) {
            QCOMPARE(surface->property("color").value<QColor>(), QColor("#eff1f3"));
            QCOMPARE(surface->property("radius").toDouble(), 6.0);
        }
        style->codeBlock()->setHorizontalPadding(std::numeric_limits<qreal>::infinity());
        style->codeBlock()->setVerticalPadding(-10);
        style->codeBlock()->setCornerRadius(std::numeric_limits<qreal>::quiet_NaN());
        style->headings()->h1()->setSeparatorThickness(std::numeric_limits<qreal>::infinity());
        style->headings()->h1()->setSeparatorSpacing(-4);
        QTRY_COMPARE(literal->mapToItem(codeSurface, QPointF()), QPointF());
        QCOMPARE(codeSurface->property("radius").toDouble(), 0.0);
        view->setWidth(0.5); style->codeBlock()->setHorizontalPadding(100);
        QTRY_VERIFY(literal->width() >= 1.0);
        QVERIFY(std::isfinite(view->property("contentHeight").toDouble()));
    }
    void separatorAtEveryLevel_data()
    {
        QTest::addColumn<int>("level");
        for (int i = 1; i <= 6; ++i) QTest::newRow(qPrintable(QString("h%1").arg(i))) << i;
    }
    void separatorAtEveryLevel()
    {
        QFETCH(int, level);
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.1\nMarkdownView { width: 180 }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300, 500); window.show();
        auto *style = view->property("style").value<MarkdownStyle *>(); QVERIFY(style);
        auto *group = style->headings()->property(qPrintable(QString("h%1").arg(level))).value<QObject *>(); QVERIFY(group);
        group->setProperty("separatorVisible", true); group->setProperty("separatorThickness", 3);
        group->setProperty("separatorSpacing", 4); group->setProperty("separatorColor", QColor(Qt::red));
        const QString prefix(level, '#');
        view->setProperty("markdown", prefix + " plain\n\n" + prefix + " *formatted*");
        QTRY_COMPARE(named(view, "headingSeparator").size(), 2);
        for (auto *rule : named(view, "headingSeparator")) {
            QCOMPARE(rule->height(), 3.0); QCOMPARE(rule->width(), 180.0);
            QCOMPARE(rule->property("color").value<QColor>(), QColor(Qt::red));
            QTRY_VERIFY(rule->y() > 4);
        }
        QVERIFY(QQmlProperty(group, "separatorVisible", &engine).reset());
        for (auto *rule : named(view, "headingSeparator")) QTRY_COMPARE(rule->height(), 0.0);
    }
    void headingImageTransitions()
    {
        QTemporaryDir directory; QVERIFY(directory.isValid());
        QImage image(40, 20, QImage::Format_RGB32); image.fill(Qt::red);
        QVERIFY(image.save(directory.filePath("image.png")));
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.1\nMarkdownView { width: 180; style: MarkdownStyle { preset: MarkdownStyle.PolishedLight } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(400, 500); window.show();
        view->setProperty("baseUrl", QUrl::fromLocalFile(directory.filePath("doc.md")));
        view->setProperty("markdown", "# before ![image](image.png) *after*\n\n## ![only](image.png)");
        const auto visibleRules = [&] {
            int count = 0;
            for (auto *rule : named(view, "headingSeparator")) if (rule->height() > 0) ++count;
            return count;
        };
        QTRY_COMPARE(visibleRules(), 2);
        auto *policy = view->property("resourcePolicy").value<QObject *>(); QVERIFY(policy);
        policy->setProperty("allowedFileRoots", QVariant::fromValue(QList<QUrl>{QUrl::fromLocalFile(directory.path())}));
        QTRY_COMPARE(named(view, "markdownImage").size(), 2);
        QCOMPARE(visibleRules(), 2);
        for (auto *rule : named(view, "headingSeparator")) if (rule->height() > 0)
            QVERIFY(rule->mapToItem(view, QPointF()).y() >= 20);
        policy->setProperty("allowedFileRoots", QVariant::fromValue(QList<QUrl>{}));
        QTRY_COMPARE(named(view, "markdownImage").size(), 0);
        QCOMPARE(visibleRules(), 2);
    }
    void inlineGeometryAndPaint_data()
    {
        QTest::addColumn<QString>("text");
        QTest::newRow("wrap") << QString("code words wrap words words words");
        QTest::newRow("bidi") << QString::fromUtf8("abc אבג xyz مرحبا");
        QTest::newRow("graphemes") << QString::fromUtf8("á é 👩‍💻 🙂 code words");
        QTest::newRow("newlines") << QString("first\nsecond\nthird");
    }
    void inlineGeometryAndPaint()
    {
        QFETCH(QString, text);
        QQuickWindow window; window.resize(400, 300); window.show();
        FormattedText item(window.contentItem());
        QFont font = QGuiApplication::font(); font.setPixelSize(20);
        item.setFont(font); item.setCodeFont(font); item.setLayoutWidth(130); item.setText(text);
        item.setFormatRanges({QVariantMap{{"start", 0}, {"length", text.size()},
            {"flags", QMarkdownPrivate::Code | QMarkdownPrivate::Strong | QMarkdownPrivate::Emphasis}}});
        item.setLinkSpans({QVariantMap{{"start", 0}, {"length", text.size() / 2}, {"destination", "target"}}});
        QTRY_VERIFY(item.layout() && item.logicalHeight() > 0);
        const QRectF geometry(item.x(), item.y(), item.width(), item.height());
        const qreal height = item.logicalHeight();
        QList<int> hits;
        for (int y = 0; y < item.height(); y += 3)
            for (int x = 0; x < item.width(); x += 3) hits.append(item.linkAt(x, y));
        item.setCodeBackground(QColor("#eff1f3")); item.setCodeCornerRadius(4);
        item.setCodeForeground(Qt::red);
        QTRY_VERIFY(item.layout() && !item.layout()->formats().isEmpty());
        QTRY_COMPARE(item.layout()->formats().last().format.foreground().color(), QColor(Qt::red));
        QCOMPARE(QRectF(item.x(), item.y(), item.width(), item.height()), geometry);
        QCOMPARE(item.logicalHeight(), height);
        int index = 0;
        for (int y = 0; y < item.height(); y += 3)
            for (int x = 0; x < item.width(); x += 3) QCOMPARE(item.linkAt(x, y), hits[index++]);
        for (const auto &range : item.layout()->formats())
            QCOMPARE(range.format.foreground().color(), range.start < text.size() / 2 ? item.linkColor() : QColor(Qt::red));
        QImage image(qCeil(item.width()), qCeil(item.height()), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        { QPainter painter(&image); item.paint(&painter); }
        int backgroundPixels = 0;
        for (int y = 0; y < image.height(); ++y) for (int x = 0; x < image.width(); ++x)
            if (image.pixelColor(x, y) == QColor("#eff1f3")) ++backgroundPixels;
        QVERIFY(backgroundPixels > 20);
        item.setCodeCornerRadius(std::numeric_limits<qreal>::infinity());
        QCOMPARE(item.codeCornerRadius(), 0.0);
    }
    void sharpnessPresentationFixtures()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(
            import QtQuick
            import QMarkdown
            Flickable {
                width: 600; height: 440; clip: true
                contentHeight: document.height + 32
                MarkdownView {
                    id: document
                    objectName: "sharpnessDocument"
                    x: 16.3; y: 16.3; width: 560
                    selectionEnabled: true
                    tablesEnabled: true
                    syntaxHighlightingEnabled: true
                    style: MarkdownStyle { preset: MarkdownStyle.PolishedLight }
                }
            }
        )", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *flick = qobject_cast<QQuickItem *>(object.get()); QVERIFY(flick);
        auto *view = flick->findChild<QQuickItem *>("sharpnessDocument"); QVERIFY(view);
        view->setProperty("markdown", "# Heading **bold** and *italic*\n\n"
            "## Second heading with `code`\n\n"
            "Body with **strong**, *emphasis*, `inline code` and [linked text](target).\n\n"
            "| Column | Details |\n| --- | --- |\n| **Bold** | *Italic* and `code` |\n"
            "| Link | [destination](target) |\n\n"
            "```cpp\n// Highlighted code\nint main() {\n    const auto text = \"crisp edges\";\n    return 42;\n}\n```\n\n"
            "### After scrolling\n\nMore **formatted** text and `inline code`.\n\n"
            "Trailing paragraph.\n\nTrailing paragraph.\n\nTrailing paragraph.");
        QQuickWindow window; window.resize(600, 440); window.setColor(Qt::white);
        flick->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(view->height() > flick->height());
        if (view->property("syntaxHighlightingAvailable").toBool()) {
            const QString code = "// Highlighted code\nint main() {\n    const auto text = \"crisp edges\";\n    return 42;\n}";
            QTRY_VERIFY(qobject_cast<FormattedText *>(textItem(view, code)));
            auto *formatted = qobject_cast<FormattedText *>(textItem(view, code));
            QTRY_VERIFY(!formatted->foregroundRanges().isEmpty());
        }
        const QString directory = QString(QMARKDOWN_PRESENTATION_DIR) + "/sharpness/"
            + QGuiApplication::platformName() + "/" + qEnvironmentVariable("QT_SCALE_FACTOR", "native");
        QVERIFY(QDir().mkpath(directory));
        for (qreal offset : {0., 80.3, 160.6}) {
            flick->setProperty("contentY", offset);
            const auto image = window.grabWindow(); QVERIFY(!image.isNull());
            QVERIFY(image.save(directory + QString("/scroll-%1.png").arg(offset)));
        }
    }
    void presentationFixtures()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.1\nMarkdownView { width: 420 }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(440, 640); view->setParentItem(window.contentItem());
        view->setX(10); view->setY(10); window.show();
        view->setProperty("markdown", "# Heading `code`\n\n## Second heading\n\nBody with **strong**, *emphasis*, `inline code` and [linked `code`](target).\n\n> A quote with `code`.\n\n- A list item\n\n```\n  literal <tag>\n  long words that wrap onto another line\n```\n\n---");
        auto *style = view->property("style").value<MarkdownStyle *>(); QVERIFY(style);
        const QStringList names = {"legacy", "polished-light", "polished-dark", "minimal-light", "minimal-dark"};
        const QString directory = QString(QMARKDOWN_PRESENTATION_DIR) + "/" + qEnvironmentVariable("QT_SCALE_FACTOR", "1");
        QVERIFY(QDir().mkpath(directory));
        for (int i = 0; i < names.size(); ++i) {
            const auto preset = static_cast<MarkdownStyle::Preset>(i);
            style->setPreset(preset);
            window.setColor(i == 2 || i == 4 ? QColor("#181a1f") : QColor(Qt::white));
            QTRY_VERIFY(view->property("contentHeight").toDouble() > 0);
            QTRY_COMPARE(named(view, "codeSurface").size(), 1);
            auto *surface = named(view, "codeSurface").first();
            QTRY_COMPARE(surface->property("radius").toDouble(), i == 1 || i == 2 ? 6.0 : 0.0);
            const bool polished = i == 1 || i == 2;
            QCOMPARE(QQmlProperty(surface, "border.width", &engine).read().toDouble(), polished ? 1.0 : 0.0);
            QCOMPARE(QQmlProperty(surface, "border.color", &engine).read().value<QColor>(),
                     i > 0 ? QColor(i == 2 || i == 4 ? "#454b54" : "#d0d7de") : QColor(Qt::transparent));
            const auto image = window.grabWindow(); QVERIFY(!image.isNull());
            const qreal scale = qreal(image.width()) / window.width();
            const auto point = surface->mapToScene(QPointF(surface->width() / 2, surface->height() - 6));
            if (polished)
                QCOMPARE(image.pixelColor(qRound(point.x() * scale), qRound(point.y() * scale)),
                         QColor(i == 2 ? "#30343b" : "#eff1f3"));
            QCOMPARE(image.pixelColor(0, 0), window.color());
            QVERIFY(image.save(directory + "/" + names[i] + ".png"));
        }
    }
};
QTEST_MAIN(StylingTest)
#include "tst_styling.moc"
