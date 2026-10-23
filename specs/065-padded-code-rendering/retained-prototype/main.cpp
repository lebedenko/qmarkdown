#include "private/formattedtext.h"
#include <QGuiApplication>
#include <QPainter>
#include <QRawFont>
#include <QDebug>
#include <cmath>
#include <cstdio>

// Preparation-only correctness gate. All font operations occur on this thread.
struct Tile {
    QRawFont font;
    quint32 glyph;
    QColor color;
    qreal dpr;
    QPointF phase;
    QImage image;
};
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    const qreal dpr = qEnvironmentVariable("GATE_DPR", "1").toDouble();
    if (!(dpr > 0 && dpr <= 4)) return 2;
    FormattedText text;
    QFont font("DejaVu Sans", 40);
    font.setItalic(true);
    text.setFont(font);
    text.setText("fX");
    text.setLayoutWidth(400);
    text.diagnosticPrepare();
    const auto runs = text.diagnosticRuns();
    QImage reference(QSize(qRound(480*dpr), qRound(160*dpr)), QImage::Format_ARGB32_Premultiplied);
    reference.setDevicePixelRatio(dpr);
    reference.fill(Qt::transparent);
    QImage candidate = reference;
    QImage control = reference;
    const QPointF offset(24.25, 24.25);
    QPainter ref(&reference), composed(&candidate), direct(&control);
    QVector<Tile> cache;
    int occurrences = 0, emitted = 0;
    qint64 bytes = 0;
    int borderInk = 0;
    for (const auto &run : runs) {
        auto all = run.glyphs;
        all.setUnderline(false); all.setStrikeOut(false); all.setOverline(false);
        ref.setPen(run.color);
        ref.drawGlyphRun(offset + run.origin, all);
        const auto indexes = all.glyphIndexes();
        const auto positions = all.positions();
        if (indexes.size() != positions.size()) return 2;
        for (qsizetype i = 0; i < indexes.size(); ++i) {
            ++occurrences;
            QGlyphRun occurrence;
            occurrence.setRawFont(all.rawFont());
            occurrence.setGlyphIndexes({indexes[i]});
            occurrence.setPositions({positions[i]});
            direct.setPen(run.color);
            direct.drawGlyphRun(offset + run.origin, occurrence);
            const QPointF position = offset + run.origin + positions[i];
            const QPointF physical = position*dpr;
            const QPointF phase(physical.x()-std::floor(physical.x()), physical.y()-std::floor(physical.y()));
            const QRectF ink = all.rawFont().boundingRect(indexes[i]);
            const QRect bounds = QRectF(ink.topLeft()*dpr+phase, ink.size()*dpr).toAlignedRect().adjusted(-2,-2,2,2);
            qsizetype entry = 0;
            for (; entry < cache.size(); ++entry) {
                const auto &t = cache[entry];
                if (t.font == all.rawFont() && t.glyph == indexes[i] && t.color == run.color && t.dpr == dpr && t.phase == phase) break;
            }
            if (entry == cache.size()) {
                QImage image(bounds.size(), QImage::Format_ARGB32_Premultiplied);
                image.setDevicePixelRatio(dpr);
                image.fill(Qt::transparent);
                QPainter painter(&image);
                painter.setPen(run.color);
                QGlyphRun single;
                single.setRawFont(all.rawFont());
                single.setGlyphIndexes({indexes[i]});
                single.setPositions({QPointF()});
                painter.drawGlyphRun((phase-QPointF(bounds.topLeft()))/dpr, single);
                painter.end();
                for (int y=0; y<image.height(); ++y) for (int x=0; x<image.width(); ++x)
                    if ((x==0 || y==0 || x==image.width()-1 || y==image.height()-1) && qAlpha(image.pixel(x,y))) ++borderInk;
                bytes += image.sizeInBytes();
                cache.append({all.rawFont(), indexes[i], run.color, dpr, phase, image});
            }
            const QPointF topLeft = (QPointF(std::floor(physical.x()), std::floor(physical.y()))+QPointF(bounds.topLeft()))/dpr;
            composed.drawImage(topLeft, cache[entry].image);
            ++emitted;
        }
    }
    ref.end(); composed.end(); direct.end();
    int changed=0, lost=0, extra=0, controlChanged=0, maxAlphaDelta=0;
    for (int y=0; y<reference.height(); ++y) for (int x=0; x<reference.width(); ++x) {
        auto a=reference.pixel(x,y), b=candidate.pixel(x,y);
        changed += a!=b;
        controlChanged += a!=control.pixel(x,y);
        maxAlphaDelta = qMax(maxAlphaDelta, qAbs(qAlpha(a)-qAlpha(b)));
        lost += qAlpha(a)>0 && qAlpha(b)==0;
        extra += qAlpha(a)==0 && qAlpha(b)>0;
    }
    reference.save("retained-reference.png"); candidate.save("retained-candidate.png");
    std::fprintf(stdout, "Qt %s DPR %.2f occurrences %d emitted %d unique images %lld image bytes %lld changed %d lost %d extra %d control changed %d border ink %d max alpha delta %d\n",
        qVersion(), double(dpr), occurrences, emitted, static_cast<long long>(cache.size()),
        static_cast<long long>(bytes), changed, lost, extra, controlChanged, borderInk, maxAlphaDelta);
    return changed || controlChanged || borderInk || occurrences==0 || occurrences!=emitted ? 1 : 0;
}
