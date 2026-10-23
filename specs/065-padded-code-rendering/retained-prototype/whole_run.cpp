#include "private/formattedtext.h"
#include "private/inline.h"
#include <QGuiApplication>
#include <QPainter>
#include <QFontDatabase>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QSet>
#include <cmath>
#include <algorithm>
#include <cstdio>

static void output(const QJsonObject &object)
{ std::puts(QJsonDocument(object).toJson(QJsonDocument::Compact).constData()); }
static QJsonArray point(QPointF p) { return {p.x(), p.y()}; }
static int changed(const QImage &a, const QImage &b, QImage *difference = nullptr)
{
    int count = 0;
    if (difference) { *difference = a; difference->fill(Qt::transparent); }
    for (int y = 0; y < a.height(); ++y) for (int x = 0; x < a.width(); ++x) {
        if (a.pixel(x,y) != b.pixel(x,y)) {
            ++count;
            if (difference) difference->setPixel(x,y,qRgba(255,0,255,255));
        }
    }
    return count;
}
static QString loadFont(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) qFatal("Cannot read explicit font");
    const auto bytes = file.readAll();
    const int id = QFontDatabase::addApplicationFontFromData(bytes);
    if (id < 0) qFatal("Cannot load explicit font");
    const auto family = QFontDatabase::applicationFontFamilies(id).first();
    output({{"type","font"},{"path",path},{"family",family},
            {"sha256",QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())}});
    return family;
}
int main(int argc, char **argv)
{
    QGuiApplication app(argc,argv);
    const auto args = app.arguments();
    if (args.size() != 6) qFatal("Usage: whole-run-gate oblique.ttf mono.ttf basic|extended|historical forward|reverse captures");
    const auto family = loadFont(args[1]), monoFamily = loadFont(args[2]);
    const bool reverse = args[4] == "reverse";
    QDir().mkpath(args[5]);
    output({{"type","identity"},{"qt",qVersion()},{"compiler",__VERSION__},
            {"platform",QGuiApplication::platformName()},{"engine","QPainter raster"},
            {"stage",args[3]},{"order",args[4]}});
    int failures = 0, cases = 0;
    QSet<QString> captured;
    auto runCase = [&](QString fixture, qreal width, int alignment, int kind,
                       qreal dpr, QPointF phase, QString id, bool historical) {
        FormattedText text;
        QFont font(family,40); font.setStyle(QFont::StyleOblique);
        QFont mono(monoFamily,40);
        mono.setStyle(QFont::StyleNormal); mono.setWeight(QFont::Normal);
        text.setFont(font); text.setCodeFont(mono); text.setText(fixture);
        text.setLayoutWidth(width); text.setHorizontalAlignment(alignment);
        text.setWrapEnabled(true); text.setCodeHorizontalPadding(7); text.setCodeVerticalPadding(3);
        if (kind == 1 || kind == 2)
            text.setFormatRanges({QVariantMap{{"start",0},{"length",kind == 1 ? 2 : 4},{"flags",QMarkdownPrivate::Code}}});
        if (kind == 1) text.setCodeSpans({QVariantMap{{"start",0},{"length",2}}});
        if (kind == 2) text.setCodeSpans({QVariantMap{{"start",0},{"length",2}},QVariantMap{{"start",2},{"length",2}}});
        if (kind == 3) text.setForegroundRanges({QVariantMap{{"start",1},{"length",2},{"color",QColor(20,80,190,180)}}});
        text.diagnosticPrepare();
        if ((kind == 1 || kind == 2) && text.codeRectangles().size() != kind)
            qFatal("Code fixture did not prepare expected padded spans");
        const auto runs = text.diagnosticRuns();
        if (runs.isEmpty() || runs.first().glyphs.positions().isEmpty()) qFatal("Missing prepared runs");
        // Reserve room for negative aligned origins at sub-glyph layout widths.
        QPointF origin(96,32);
        if (historical) origin = QPointF(24.25,24.25);
        else {
            const auto anchor = (origin + runs.first().origin + runs.first().glyphs.positions().first())*dpr;
            origin += (phase - QPointF(anchor.x()-std::floor(anchor.x()),anchor.y()-std::floor(anchor.y())))/dpr;
        }
        QImage blank(QSize(qCeil(600*dpr),qCeil((text.logicalHeight()+100)*dpr)),QImage::Format_ARGB32_Premultiplied);
        blank.setDevicePixelRatio(dpr); blank.fill(Qt::transparent);
        QMap<QString,QImage> images;
        for (auto name : {"direct","tile","crop","glyph"}) images[name] = blank;
        QJsonArray runLog;
        int glyphs = 0, tileRuns = 0, cropRuns = 0, directRuns = 0, glyphEmitted = 0;
        int croppedInk = 0, borderInk = 0;
        QStringList order{"direct","tile","crop","glyph"};
        if (reverse) std::reverse(order.begin(),order.end());
        for (const auto &run : runs) {
            auto all = run.glyphs;
            all.setUnderline(false); all.setStrikeOut(false); all.setOverline(false);
            const auto indexes = all.glyphIndexes(); const auto positions = all.positions();
            // Font resolution must use the supplied files, including code-font style.
            const QRawFont explicitFont(all.rawFont().familyName() == monoFamily ? args[2] : args[1], all.rawFont().pixelSize());
            for (const char *table : {"head", "name", "cmap", "glyf"})
                if (all.rawFont().fontTable(table) != explicitFont.fontTable(table))
                    qFatal("Prepared font differs from explicit font file");
            if (indexes.size() != positions.size() || indexes.isEmpty()) qFatal("Invalid prepared run");
            QRectF ink; QJsonArray sequence;
            for (qsizetype i = 0; i < indexes.size(); ++i) {
                const auto rect = all.rawFont().boundingRect(indexes[i]).translated(origin+run.origin+positions[i]);
                if (!rect.isEmpty()) ink = ink.isEmpty() ? rect : ink.united(rect);
                sequence.append(QJsonObject{{"index",int(indexes[i])},{"position",point(positions[i])}});
            }
            glyphs += indexes.size();
            const QRect bounds = QRectF(ink.topLeft()*dpr,ink.size()*dpr).toAlignedRect().adjusted(-4,-4,4,4);
            // Independent full-canvas run support catches underestimated raw-font bounds.
            QImage full = blank;
            { QPainter p(&full); p.setPen(run.color); p.drawGlyphRun(origin+run.origin,all); }
            int canvasBorder = 0;
            for (int y=0;y<full.height();++y) for (int x=0;x<full.width();++x) if (qAlpha(full.pixel(x,y))) {
                croppedInk += !bounds.contains(x,y);
                canvasBorder += x==0 || y==0 || x==full.width()-1 || y==full.height()-1;
            }
            croppedInk += canvasBorder;
            for (const auto &path : order) {
                if (path == "direct") {
                    QPainter p(&images[path]); p.setPen(run.color); p.drawGlyphRun(origin+run.origin,all); ++directRuns;
                } else if (path == "glyph") {
                    QPainter p(&images[path]); p.setPen(run.color);
                    for (qsizetype i=0;i<indexes.size();++i) {
                        auto single = all; single.setGlyphIndexes({indexes[i]}); single.setPositions({positions[i]});
                        p.drawGlyphRun(origin+run.origin,single); ++glyphEmitted;
                    }
                } else {
                    QImage tile;
                    if (path == "crop") { tile = full.copy(bounds); ++cropRuns; }
                    else {
                        tile = QImage(bounds.size(),blank.format()); tile.setDevicePixelRatio(dpr); tile.fill(Qt::transparent);
                        QPainter p(&tile); p.setPen(run.color); p.translate(-QPointF(bounds.topLeft())/dpr);
                        p.drawGlyphRun(origin+run.origin,all); ++tileRuns;
                    }
                    for (int y=0;y<tile.height();++y) for (int x=0;x<tile.width();++x)
                        if ((x==0 || y==0 || x==tile.width()-1 || y==tile.height()-1) && qAlpha(tile.pixel(x,y))) ++borderInk;
                    tile.setDevicePixelRatio(1);
                    QPainter p(&images[path]); p.resetTransform(); p.scale(1/dpr,1/dpr);
                    p.drawImage(bounds.topLeft(),tile);
                }
            }
            runLog.append(QJsonObject{{"ordinal",runLog.size()},{"origin",point(run.origin)},
                {"drawOrigin",point(origin+run.origin)},{"family",all.rawFont().familyName()},
                {"style",all.rawFont().styleName()},{"pixelSize",all.rawFont().pixelSize()},
                {"color",run.color.name(QColor::HexArgb)},{"flags",int(all.flags())},
                {"bounds",QJsonArray{bounds.x(),bounds.y(),bounds.width(),bounds.height()}},{"glyphs",sequence}});
        }
        const int tileDiff = changed(images["direct"],images["tile"]);
        const int cropDiff = changed(images["direct"],images["crop"]);
        const int pairDiff = changed(images["tile"],images["crop"]);
        const int glyphDiff = changed(images["direct"],images["glyph"]);
        const bool failed = tileDiff || cropDiff || pairDiff || croppedInk || glyphs != glyphEmitted
            || directRuns != runs.size() || tileRuns != runs.size() || cropRuns != runs.size();
        failures += failed; ++cases;
        output({{"type","case"},{"id",id},{"text",fixture},{"dpr",dpr},{"phase",point(phase)},
            {"alignment",alignment},{"width",width},{"codeRectangles",text.codeRectangles().size()},{"runs",runLog},{"runCount",runs.size()},
            {"glyphCount",glyphs},{"directEmissions",directRuns},{"tileEmissions",tileRuns},
            {"cropEmissions",cropRuns},{"glyphEmissions",glyphEmitted},{"tileChanged",tileDiff},
            {"cropChanged",cropDiff},{"tileCropChanged",pairDiff},{"glyphChanged",glyphDiff},
            {"croppedInk",croppedInk},{"borderInk",borderInk},{"failed",failed}});
        QString key = QString::number(dpr)+(failed?"-failure":"-sample");
        if (!captured.contains(key)) {
            captured.insert(key);
            const auto prefix = args[5]+"/"+id;
            images["direct"].save(prefix+"-reference.png"); images["tile"].save(prefix+"-candidate.png");
            QImage difference; changed(images["direct"],images["tile"],&difference); difference.save(prefix+"-difference.png");
        }
    };
    const QList<qreal> dprs{1,1.25,1.5,2}, phases{0,.25,.5,.75};
    if (args[3] == "historical") {
        for (qreal dpr : {1.,1.25}) runCase("fX",400,Qt::AlignLeft,0,dpr,{},QString("historical-%1").arg(dpr),true);
    } else if (args[3] == "basic" || args[3] == "extended") {
        int fixtureIndex = 0;
        for (const QString &fixture : args[3] == "basic" ? QStringList{"f","X","fX","fXfX"} : QStringList{"fXfX","fXfX","fXfX","fX fX fX","fXfX"}) {
            const bool extended = args[3] == "extended";
            for (int alignment : extended ? QList<int>{Qt::AlignLeft,Qt::AlignHCenter,Qt::AlignRight} : QList<int>{Qt::AlignLeft})
                for (qreal dpr : dprs) for (qreal x : phases) for (qreal y : phases)
                    runCase(fixture,extended && fixtureIndex==3 ? 95 : extended && fixtureIndex==4 ? 1 : 400,
                            alignment,extended && fixtureIndex<3 ? fixtureIndex+1 : 0,dpr,{x,y},
                            QString("case-%1-%2-%3-%4-%5").arg(fixtureIndex).arg(alignment).arg(dpr).arg(x).arg(y),false);
            ++fixtureIndex;
        }
    } else qFatal("Invalid stage");
    output({{"type","summary"},{"cases",cases},{"failures",failures}});
    return failures ? 1 : 0;
}
