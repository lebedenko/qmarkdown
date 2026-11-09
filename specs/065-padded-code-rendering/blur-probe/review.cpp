// Standalone review uses the qualification fixture setup and painted-image geometry.
// Keep these diagnostic fixtures in sync with motion.cpp; actual raster equality
// against both final source variants checks raw pixels only. It does not qualify
// the complete scrolling proxy composition; owner artifacts were reported.
#include "private/formattedtext.h"
#include "private/inline.h"
#include <QGuiApplication>
#include <QQuickWindow>
#include <QQuickPaintedItem>
#include <QPainter>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QSGRendererInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QHash>
#include <QDir>
#include <QTimer>
#include <QtTest/QTest>
#include <memory>
#include <cstdio>
#include <cstdlib>
class BlurProbe {
public:
    static quint64 paints(const FormattedText &text) { return text.m_rasterPaintCount; }
    static QJsonObject geometry(const FormattedText &text) {
        QJsonArray cells, links, codes;
        auto rect=[](QRectF r) { return QJsonArray{r.x(),r.y(),r.width(),r.height()}; };
        for(const auto &cell:text.m_cursorCells)
            cells.append(QJsonObject{{"start",cell.start},{"end",cell.end},{"rect",rect(cell.rect)},{"reversed",cell.reversed}});
        for(const auto &hit:text.m_hitRegions) links.append(QJsonObject{{"rect",rect(hit.rect)},{"link",hit.link}});
        for(const auto &code:text.m_codeRects) codes.append(rect(code));
        return {{"cells",cells},{"links",links},{"codeRects",codes}};
    }
};
static void require(bool condition, const char *message) {
    if(!condition) { fprintf(stderr,"REVIEW GUARD: %s\n",message); std::exit(1); }
}
static QJsonDocument readJson(QString path) {
    QFile file(path); require(file.open(QIODevice::ReadOnly),"missing qualification evidence");
    return QJsonDocument::fromJson(file.readAll());
}
static QHash<QString,QString> rawImages(QString path) {
    QFile file(path); require(file.open(QIODevice::ReadOnly),"missing final source trace");
    QHash<QString,QString> images; QString prefix;
    while(!file.atEnd()) {
        const auto row=QJsonDocument::fromJson(file.readLine()).object();
        if(row["type"]=="rawCalibration") { require(row["equal"].toBool(),"raw raster unqualified"); prefix=row["prefix"].toString(); }
        if(row["type"]=="fixture" && row["surfaceCount"].toInt()) images.insert(row["fixture"].toString(),prefix+"-independent.png");
    }
    require(images.size()==8,"incomplete retained fixture evidence"); return images;
}
class PaintedControl : public QQuickPaintedItem {
public:
    explicit PaintedControl(QQuickPaintedItem *source) : QQuickPaintedItem(source->parentItem()),
        image(source->property("blurImage").value<QImage>()) {
        setPosition(source->position()); setSize(source->size());
        setAntialiasing(source->antialiasing()); setSmooth(source->smooth());
        setOpaquePainting(source->opaquePainting()); setFillColor(source->fillColor());
        setTextureSize(source->textureSize()); setOpacity(0);
    }
    void paint(QPainter *painter) override { painter->drawImage(boundingRect(), image); }
    QImage image;
};
#include "scene.h"
#include <QElapsedTimer>
#include <QSet>
#include <QSignalSpy>
#include <vector>

int main(int argc,char **argv) {
    QGuiApplication app(argc,argv);
    const auto args=app.arguments();
    if(args.size()!=4) return 2; // final evidence, fresh output, opengl|vulkan
    const auto stages=readJson(args[1]+"/stages.json").array();
    bool performanceRejected=false;
    for(const QString &qt:{QString("6.8.0"),QString("6.12.0")})
        for(const QString &backend:{QString("opengl"),QString("vulkan")}) {
            bool passed=false;
            for(const auto &stage:stages) {
                const auto row=stage.toObject();
                if(row["name"]=="correctness-"+qt+"-"+backend) passed=row["exit"].toInt(-1)==0;
            }
            require(passed,"native image comparison incomplete or failed");
        }
    for(const auto &stage:stages) {
        const auto row=stage.toObject();
        performanceRejected |= row["name"].toString().startsWith("performance-") && row["exit"].toInt(-1)!=0;
    }
    const QString outcome=performanceRejected?"performance rejected":"qualification pending";
    require(args[3]=="opengl" || args[3]=="vulkan","native backend required");
    require(QDir().mkdir(args[2]),"review output must be new");
    qputenv("BLUR_CALIBRATION",QFile::encodeName(QDir(args[2]).absolutePath()));
    const auto source=readJson(BLUR_MANIFEST).object();
    require((source["variant"]=="baseline" || source["variant"]=="candidate") && source["policy"]=="native-only-v2","review requires revised baseline snapshot");
    const QString qt=QString::fromLatin1(qVersion());
    require(qt=="6.8.0" || qt=="6.12.0","unqualified review Qt");
    const QString base=args[1]+"/correctness-"+qt+"-"+args[3]+"/"+args[3]+"-1-";
    const auto baseline=rawImages(base+"baseline-static/results.jsonl");
    const auto candidate=rawImages(base+"candidate-static/results.jsonl");
    QQmlEngine engine; QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
Rectangle {
    width: 800; height: 720; color: "white"
    property alias flick: flick
    property bool alternate: false
    property string outcome: ""
    property string variant: ""
    Text { x: 12; y: 8; font.pixelSize: 14; text: (parent.variant + (parent.alternate ? " — UNQUALIFIED image proxy" : " — actual source")) + " | " + parent.outcome }
    MouseArea { x: 0; y: 0; width: 530; height: 40; onClicked: parent.alternate = !parent.alternate }
    Text { x: 550; y: 8; font.pixelSize: 14; text: "pixelAligned: " + flick.pixelAligned }
    MouseArea { x: 540; y: 0; width: 260; height: 40; onClicked: flick.pixelAligned = !flick.pixelAligned }
    Flickable {
        id: flick; x: 0; y: 48; width: parent.width; height: 640
        contentWidth: 2400; contentHeight: 3200; clip: true; pixelAligned: true
        flickableDirection: Flickable.HorizontalAndVerticalFlick
        boundsBehavior: Flickable.StopAtBounds; flickDeceleration: 650
    }
})",QUrl());
    std::unique_ptr<QObject> root(component.create()); require(bool(root),"review QML failed");
    root->setProperty("outcome",outcome);
    root->setProperty("variant",source["variant"].toString().toUpper());
    auto *item=qobject_cast<QQuickItem *>(root.get());
    auto *flick=qobject_cast<QQuickItem *>(root->property("flick").value<QObject *>());
    auto *content=qobject_cast<QQuickItem *>(flick->property("contentItem").value<QObject *>());
    QQuickWindow window; window.resize(800,720);
    window.setTitle("Native-only review — " + args[3] + " — Qt " + qt + " / DPR 1.25 — " + outcome);
    item->setParentItem(window.contentItem()); window.show();
    require(QTest::qWaitForWindowExposed(&window),"review window not exposed"); QTest::qWait(100);
    require(window.effectiveDevicePixelRatio()==1.25,"unqualified review Qt/DPR");
    require(window.rendererInterface()->graphicsApi()==(args[3]=="opengl"?QSGRendererInterface::OpenGL:QSGRendererInterface::Vulkan),"review backend fallback");
    QList<QQuickItem *> paragraphs;
    QList<QQuickPaintedItem *> sources; QList<PaintedControl *> proxies;
    const auto cases=fixtures();
    for(int i=0;i<24;++i) {
        const auto f=cases[4+i%8];
        auto *text=createReviewParagraph(content,f); auto *clip=text->parentItem(); QTest::qWait(50); window.grabWindow();
        QQuickPaintedItem *surface=nullptr;
        for(auto *child:text->childItems()) if(auto *painted=dynamic_cast<QQuickPaintedItem *>(child)) surface=painted;
        require(surface && surface->smooth()==(source["variant"]=="baseline") && surface->property("blurRawEqual").toBool(),"review source raster unqualified");
        auto *proxy=new PaintedControl(surface);
        for(const auto &path:{baseline.value(f.id),candidate.value(f.id)}) {
            QImage expected(path); require(!expected.isNull(),"missing qualified raw image");
            require(proxy->image==expected.convertToFormat(proxy->image.format()),"review pixels differ from qualified source");
        }
        proxy->setSmooth(surface->smooth()); paragraphs.append(clip); sources.append(surface); proxies.append(proxy);
        clip->setY(40+i*120);
    }
    const QJsonObject ready{{"type","reviewReady"},{"qt",qt},{"backend",args[3]},
        {"graphicsApi",int(window.rendererInterface()->graphicsApi())},{"dpr",window.effectiveDevicePixelRatio()},
        {"source",source},{"outcome",outcome},{"ownerReview","proxy artifacts reported"},{"proxyValidation","raw pixels only; scrolling composition unqualified"}};
    puts(QJsonDocument(ready).toJson(QJsonDocument::Compact).constData()); fflush(stdout);
    if(qEnvironmentVariableIsSet("BLUR_SCAN")) {
        QList<quint64> paints;
        QList<QJsonObject> interactions;
        QList<QRectF> bounds;
        std::vector<std::unique_ptr<QSignalSpy>> layouts;
        for(auto *paragraph:paragraphs) {
            auto *text=static_cast<FormattedText *>(paragraph->childItems().first());
            paints.append(BlurProbe::paints(*text)); interactions.append(BlurProbe::geometry(*text));
            bounds.append(text->boundingRect());
            layouts.emplace_back(new QSignalSpy(text,&FormattedText::layoutChanged));
        }
        if(qEnvironmentVariableIsSet("BLUR_UNALIGNED")) flick->setProperty("pixelAligned",false);
        QElapsedTimer elapsed; elapsed.start();
        QSet<QString> visited;
        int capture=0;
        struct LivePose { QPointF position; QImage image; qint64 ns; };
        QList<LivePose> trace;
        auto captureLive=[&] {
            const QImage image=window.grabWindow().copy(QRect(0,60,1000,800));
            trace.append({content->position(),image,elapsed.nsecsElapsed()});
        };
        auto settle=[&] { QTest::qWait(40); window.grabWindow(); };
        auto viewport=[&] { return window.grabWindow().copy(QRect(0,60,1000,800)); };
        auto inspect=[&](double requested) {
            flick->setProperty("contentY",requested); settle();
            const QPointF actual=content->position();
            const QString key=QString::number(actual.x(),'g',17)+","+QString::number(actual.y(),'g',17);
            if(visited.contains(key)) return true;
            visited.insert(key);
            const QImage live=viewport();
            QMetaObject::invokeMethod(flick,"cancelFlick");
            flick->setProperty("contentX",-actual.x()); flick->setProperty("contentY",-actual.y()); settle();
            const QImage frozen=viewport();
            const QString prefix=args[2]+"/pose-"+QString::number(capture++);
            frozen.save(prefix+"-source.png");
            const bool replay=live==frozen && content->position()==actual;
            QJsonArray geometry;
            for(int i=0;i<paragraphs.size();++i) {
                auto *paragraph=paragraphs[i]; auto *text=static_cast<FormattedText *>(paragraph->childItems().first());
                require(BlurProbe::paints(*text)==paints[i] && layouts[i]->isEmpty(),"warmed paint/layout changed");
                require(BlurProbe::geometry(*text)==interactions[i] && text->boundingRect()==bounds[i],"interaction/bounds changed");
                require(sources[i]->parentItem()==text,"source surface changed");
                const auto bounds=text->mapRectToScene(text->boundingRect());
                const auto t=text->itemTransform(window.contentItem(),nullptr);
                geometry.append(QJsonObject{{"index",i},{"visible",text->isVisible()},{"z",text->z()},{"surface",QString::number(quintptr(sources[i]),16)},{"paintCount",double(BlurProbe::paints(*text))},{"interactions",interactions[i]},
                    {"bounds",QJsonArray{bounds.x(),bounds.y(),bounds.width(),bounds.height()}},
                    {"clip",paragraph->clip()},{"clipRect",QJsonArray{paragraph->x(),paragraph->y(),paragraph->width(),paragraph->height()}},
                    {"transform",QJsonArray{t.m11(),t.m12(),t.m21(),t.m22(),t.dx(),t.dy()}},
                    {"overlapsNext",i+1<paragraphs.size() && text->height()>120}});
            }
            bool equal=false;
            if(replay) {
                for(int i=0;i<sources.size();++i) { sources[i]->setOpacity(0); proxies[i]->setOpacity(1); }
                settle(); const QImage proxy=viewport(); equal=frozen==proxy;
                proxy.save(prefix+"-proxy.png");
                if(!equal) {
                    frozen.save(prefix+"-source.png"); proxy.save(prefix+"-proxy.png");
                    QImage diff=frozen.convertToFormat(QImage::Format_ARGB32);
                    for(int y=0;y<diff.height();++y) for(int x=0;x<diff.width();++x)
                        diff.setPixel(x,y,frozen.pixel(x,y)==proxy.pixel(x,y)?qRgb(0,0,0):qRgb(255,0,255));
                    diff.save(prefix+"-diff.png");
                }
                for(int i=0;i<sources.size();++i) { sources[i]->setOpacity(1); proxies[i]->setOpacity(0); }
                settle();
            } else { live.save(prefix+"-live.png"); frozen.save(prefix+"-frozen.png"); }
            QJsonObject row{{"type","artifactPose"},{"ns",double(elapsed.nsecsElapsed())},
                {"requestedY",requested},{"pixelAligned",flick->property("pixelAligned").toBool()},{"actual",QJsonArray{actual.x(),actual.y()}},
                {"replayEqual",replay},{"proxyEqual",equal},{"paragraphs",geometry},{"prefix",prefix}};
            puts(QJsonDocument(row).toJson(QJsonDocument::Compact).constData()); fflush(stdout);
            return replay && equal;
        };
        if(qEnvironmentVariableIsSet("BLUR_POSE")) {
            const double pose=qEnvironmentVariable("BLUR_POSE").toDouble();
            if(!inspect(pose)) return 3;
            for(double x:{0.,.25,.5,.75}) for(double y:{0.,.25,.5,.75}) {
                flick->setProperty("contentX",x/1.25);
                if(!inspect(pose+y/1.25)) return 3;
            }
            flick->setProperty("contentX",0); if(!inspect(0)) return 3;
            if(!inspect(2560)) return 3;
            return inspect(pose)?0:3;
        }
        for(double y=240;y<=360;y+=0.2) if(!inspect(y)) return 3;
        flick->setProperty("contentY",300); settle();
        QTest::mousePress(&window,Qt::LeftButton,Qt::NoModifier,{380,560});
        for(int y=550;y>=180;y-=10) { QTest::mouseMove(&window,{380,y},8); captureLive(); }
        QTest::mouseRelease(&window,Qt::LeftButton,Qt::NoModifier,{380,180});
        for(int i=0;i<30;++i) { QTest::qWait(16); captureLive(); }
        QTest::mousePress(&window,Qt::LeftButton,Qt::NoModifier,{380,180});
        for(int y=190;y<=560;y+=10) { QTest::mouseMove(&window,{380,y},8); captureLive(); }
        QTest::mouseRelease(&window,Qt::LeftButton,Qt::NoModifier,{380,560});
        for(int i=0;i<180;++i) { QTest::qWait(16); captureLive(); }
        QMetaObject::invokeMethod(flick,"cancelFlick");
        for(int i=0;i<trace.size();++i) {
            const auto &pose=trace[i];
            flick->setProperty("contentX",-pose.position.x()); flick->setProperty("contentY",-pose.position.y()); settle();
            const QImage frozen=viewport();
            const QString prefix=args[2]+"/live-"+QString::number(i);
            pose.image.save(prefix+"-source.png");
            const bool replay=frozen==pose.image && content->position()==pose.position;
            puts(QJsonDocument(QJsonObject{{"type","liveReplay"},{"ns",double(pose.ns)},
                {"actual",QJsonArray{pose.position.x(),pose.position.y()}},{"equal",replay},
                {"prefix",prefix}}).toJson(QJsonDocument::Compact).constData()); fflush(stdout);
            if(!replay) { frozen.save(prefix+"-frozen.png"); return 4; }
            visited.clear(); if(!inspect(-pose.position.y())) return 3;
        }
        for(double y=0;y<=2560;y+=0.8) if(!inspect(y)) return 3;
        return 0;
    }
    bool previous=false;
    QTimer timer;
    QObject::connect(&timer,&QTimer::timeout,&window,[&] {
        const bool alternate=root->property("alternate").toBool(); if(alternate==previous) return;
        const auto position=content->position();
        for(int i=0;i<sources.size();++i) { sources[i]->setOpacity(alternate?0:1); proxies[i]->setOpacity(alternate?1:0); }
        require(content->position()==position,"variant switch changed scroll position"); previous=alternate;
    });
    timer.start(16); return app.exec();
}
