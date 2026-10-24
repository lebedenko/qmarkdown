#include "private/formattedtext.h"
#include <QGuiApplication>
#include <QPainter>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QSet>
#include <cstdio>
#include <cmath>
#include <algorithm>

static QJsonArray point(QPointF p) { return {p.x(), p.y()}; }
static QJsonArray transform(const QTransform &t)
{ return {t.m11(),t.m12(),t.m21(),t.m22(),t.dx(),t.dy()}; }
static QJsonObject compare(const QImage &a, const QImage &b)
{
    int changed=0,lost=0,extra=0; int delta[4]={};
    for(int y=0;y<a.height();++y) for(int x=0;x<a.width();++x) {
        QRgb p=a.pixel(x,y),q=b.pixel(x,y); changed+=p!=q;
        lost+=qAlpha(p)>0 && qAlpha(q)==0; extra+=qAlpha(p)==0 && qAlpha(q)>0;
        for(int c=0;c<4;++c) delta[c]=qMax(delta[c],qAbs(int((p>>(8*c))&255)-int((q>>(8*c))&255)));
    }
    return {{"changed",changed},{"lost",lost},{"extra",extra},
            {"maxBGRA",QJsonArray{delta[0],delta[1],delta[2],delta[3]}}};
}
static void output(const QJsonObject &o)
{ auto s=QJsonDocument(o).toJson(QJsonDocument::Compact); std::puts(s.constData()); }
int main(int argc,char **argv)
{
    QGuiApplication app(argc,argv);
    const QString selection=qEnvironmentVariable("DIAG_COMPARE","all");
    const QString captures=qEnvironmentVariable("DIAG_CAPTURES","captures");
    QDir().mkpath(captures);
    output({{"type","identity"},{"qt",qVersion()},{"compiler",__VERSION__},
            {"platform",QGuiApplication::platformName()},{"engine","QPainter raster"},
            {"selection",selection}});
    const bool reverse=qEnvironmentVariableIntValue("DIAG_REVERSE")!=0;
    auto ordered=[&](QStringList paths) { if(reverse) std::reverse(paths.begin(),paths.end()); return paths; };
    int failures=0, cases=0; QSet<QString> saved;
    auto runCase=[&](QString fixture,qreal dpr,QPointF origin,QString id) {
        FormattedText text; QFont font("DejaVu Sans",40); font.setItalic(true);
        text.setFont(font);text.setText(fixture);text.setLayoutWidth(400);text.diagnosticPrepare();
        // Matrix phases refer to the first prepared glyph, not merely the canvas origin.
        if(!id.startsWith("historical")) {
            const auto runs=text.diagnosticRuns();
            if(runs.isEmpty() || runs.first().glyphs.positions().isEmpty()) qFatal("No prepared glyph");
            const QPointF anchor=(runs.first().origin+runs.first().glyphs.positions().first())*dpr;
            origin-=QPointF(anchor.x()-std::floor(anchor.x()),anchor.y()-std::floor(anchor.y()))/dpr;
        }
        QImage blank(QSize(qRound(480*dpr),qRound(160*dpr)),QImage::Format_ARGB32_Premultiplied);
        blank.setDevicePixelRatio(dpr);blank.fill(Qt::transparent);
        QMap<QString,QImage> images;
        for(auto name:{"reference","reconstructed","preserved","local","translated","crop","legacy"}) images[name]=blank;
        QVector<int> coverage(blank.width()*blank.height(),0);
        int occurrences=0; QJsonArray glyphLog; QMap<QString,int> borders,emissions;
        for(const auto &run:text.diagnosticRuns()) {
            auto all=run.glyphs;all.setUnderline(false);all.setStrikeOut(false);all.setOverline(false);
            if(!reverse) {QPainter p(&images["reference"]);p.setPen(run.color);p.drawGlyphRun(origin+run.origin,all);}
            auto indexes=all.glyphIndexes();auto positions=all.positions();
            if(indexes.size()!=positions.size()) qFatal("Invalid prepared run");
            for(qsizetype i=0;i<indexes.size();++i) {
                ++occurrences;
                QGlyphRun reconstructed;reconstructed.setRawFont(all.rawFont());
                reconstructed.setGlyphIndexes({indexes[i]});reconstructed.setPositions({positions[i]});
                auto single=all;single.setGlyphIndexes({indexes[i]});single.setPositions({positions[i]});
                QImage full=blank;
                for(auto name:ordered({"reconstructed","preserved","crop"})) {
                    QPainter p(QString(name)=="crop"? &full : &images[name]);p.setPen(run.color);
                    p.drawGlyphRun(origin+run.origin,QString(name)=="reconstructed"?reconstructed:single);
                    if(QString(name)!="crop") ++emissions[name];
                }
                for(int y=0;y<full.height();++y) for(int x=0;x<full.width();++x)
                    coverage[y*full.width()+x]+=qAlpha(full.pixel(x,y))>0;
                QPointF physical=(origin+run.origin+positions[i])*dpr;
                QPointF phase(physical.x()-std::floor(physical.x()),physical.y()-std::floor(physical.y()));
                QRectF ink=all.rawFont().boundingRect(indexes[i]);
                QRect local=QRectF(ink.topLeft()*dpr+phase,ink.size()*dpr).toAlignedRect().adjusted(-2,-2,2,2);
                QPoint base(int(std::floor(physical.x())),int(std::floor(physical.y())));
                QRect bounds=local.translated(base);
                QJsonObject detail{{"index",int(indexes[i])},{"position",point(positions[i])},
                    {"origin",point(origin+run.origin)},{"phase",point(phase)},
                    {"flags",int(all.flags())},{"singleFlags",int(single.flags())},
                    {"reconstructedFlags",int(reconstructed.flags())},
                    {"fontFamily",all.rawFont().familyName()},{"fontStyle",all.rawFont().styleName()},
                    {"pixelSize",all.rawFont().pixelSize()},{"fontWeight",all.rawFont().weight()},
                    {"hinting",int(all.rawFont().hintingPreference())},{"color",run.color.name(QColor::HexArgb)},
                    {"bounds",QJsonArray{bounds.x(),bounds.y(),bounds.width(),bounds.height()}},{"dpr",dpr}};
                for(auto name:ordered({"local","translated","crop","legacy"})) {
                    QImage tile;
                    if(QString(name)=="crop") tile=full.copy(bounds);
                    else {
                        tile=QImage(bounds.size(),blank.format());tile.setDevicePixelRatio(dpr);tile.fill(Qt::transparent);
                        QPainter p(&tile);p.setPen(run.color);
                        if(QString(name)=="translated") {
                            p.translate(-QPointF(bounds.topLeft())/dpr);
                            detail["translatedDeviceTransform"]=transform(p.deviceTransform());
                            p.drawGlyphRun(origin+run.origin,single);
                        } else {
                            QGlyphRun zero;zero.setRawFont(all.rawFont());
                            zero.setGlyphIndexes({indexes[i]});zero.setPositions({QPointF()});
                            QPointF offset=(phase-QPointF(local.topLeft()))/dpr;
                            detail["localDrawOrigin"]=point(offset);
                            detail["localDeviceTransform"]=transform(p.deviceTransform());
                            p.drawGlyphRun(offset,zero);
                        }
                    }
                    for(int y=0;y<tile.height();++y) for(int x=0;x<tile.width();++x)
                        if((x==0||y==0||x==tile.width()-1||y==tile.height()-1)&&qAlpha(tile.pixel(x,y))) ++borders[name];
                    QImage &target=images[name];
                    if(QString(name)=="legacy") {QPainter p(&target);p.drawImage(QPointF(bounds.topLeft())/dpr,tile);}
                    else {
                        target.setDevicePixelRatio(1);tile.setDevicePixelRatio(1);
                        {QPainter p(&target);p.drawImage(bounds.topLeft(),tile);}
                        target.setDevicePixelRatio(dpr);
                    }
                    ++emissions[name];
                }
                glyphLog.append(detail);
            }
            if(reverse) {QPainter p(&images["reference"]);p.setPen(run.color);p.drawGlyphRun(origin+run.origin,all);}
        }
        int overlapChanges=0, isolatedChanges=0;
        for(int y=0;y<blank.height();++y) for(int x=0;x<blank.width();++x)
            if(images["reference"].pixel(x,y)!=images["preserved"].pixel(x,y)) {
                if(coverage[y*blank.width()+x]>1) ++overlapChanges; else ++isolatedChanges;
            }
        QJsonObject metrics,counts,border;
        QList<QPair<QString,QString>> pairs={{"reference","reconstructed"},{"reference","preserved"},
            {"reconstructed","preserved"},{"preserved","local"},{"preserved","translated"},{"preserved","crop"},
            {"reference","local"},{"reference","translated"},{"reference","crop"},{"local","legacy"},{"reference","legacy"}};
        if(reverse) std::reverse(pairs.begin(),pairs.end());
        for(const auto &pair:pairs) {
            QString key=pair.first+"->"+pair.second;
            if(selection!="all"&&!selection.split(',').contains(key)) continue;
            auto m=compare(images[pair.first],images[pair.second]);metrics[key]=m;
            bool failed=m["changed"].toInt()!=0; failures+=failed;
            QString captureKey=QString::number(dpr)+"-"+key;
            if(id.startsWith("historical") || (failed&&!saved.contains(captureKey))) {
                saved.insert(captureKey);QString prefix=captures+"/"+id+"-"+pair.first+"-"+pair.second;
                images[pair.first].save(prefix+"-reference.png");images[pair.second].save(prefix+"-candidate.png");
                QImage diff=blank;diff.setDevicePixelRatio(1);
                for(int y=0;y<diff.height();++y)for(int x=0;x<diff.width();++x) {
                    QRgb a=images[pair.first].pixel(x,y),b=images[pair.second].pixel(x,y);
                    int d=0;for(int c=0;c<4;++c)d=qMax(d,qAbs(int((a>>(8*c))&255)-int((b>>(8*c))&255)));
                    diff.setPixel(x,y,qRgb(qMin(255,d*16),qMin(255,d*16),qMin(255,d*16)));
                }
                diff.save(prefix+"-difference.png");
            }
        }
        for(auto it=emissions.begin();it!=emissions.end();++it) counts[it.key()]=it.value();
        for(auto it=borders.begin();it!=borders.end();++it) border[it.key()]=it.value();
        output({{"type","case"},{"id",id},{"fixture",fixture},{"dpr",dpr},{"origin",point(origin)},
                {"occurrences",occurrences},{"directChangesInOverlap",overlapChanges},
                {"directChangesOutsideOverlap",isolatedChanges},{"emissions",counts},{"borderInk",border},{"metrics",metrics},{"glyphs",glyphLog}});
        ++cases;
    };
    for(qreal dpr:{1.,1.25})runCase("fX",dpr,{24.25,24.25},QString("historical-%1").arg(dpr));
    for(QString fixture:{"f","X","fX","fXfX"})for(qreal dpr:{1.,1.25,1.5,2.})
        for(int x=0;x<4;++x)for(int y=0;y<4;++y)
            runCase(fixture,dpr,{(24*dpr+x*.25)/dpr,(24*dpr+y*.25)/dpr},
                    QString("%1-%2-%3-%4").arg(fixture).arg(dpr).arg(x).arg(y));
    output({{"type","summary"},{"cases",cases},{"failedComparisons",failures}});
    return failures?1:0;
}
