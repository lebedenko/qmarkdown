#include "private/formattedtext.h"
#include "private/inline.h"
#include <QGuiApplication>
#include <QQuickWindow>
#include <QQuickPaintedItem>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QSGRendererInterface>
#include <QPainter>
#include <QElapsedTimer>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QProcess>
#include <QCryptographicHash>
#include <QtTest/QTest>
#include <algorithm>
#include <functional>
#include <cstdio>
#include <cstdlib>
#include <QRawFont>
#include <QSGSimpleTextureNode>
#include <QSGTexture>

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
static void record(QJsonObject value) {
    puts(QJsonDocument(value).toJson(QJsonDocument::Compact).constData()); fflush(stdout);
}
static void require(bool value, const char *message) { if (!value) { fprintf(stderr,"GATE FAILED: %s\n",message); std::exit(1); } }
static QQuickPaintedItem *surface(FormattedText &text) {
    for (auto *child : text.childItems()) if (auto *item = dynamic_cast<QQuickPaintedItem *>(child)) return item;
    return nullptr;
}
// Independently retained pixels; never consults the text during movement.
class ImageControl : public QQuickItem {
public:
    explicit ImageControl(QQuickPaintedItem *source) : QQuickItem(source->parentItem()) {
        setFlag(ItemHasContents);
        connect(this,&QQuickItem::smoothChanged,this,[this]{update();});
        const auto dpr = source->window()->effectiveDevicePixelRatio();
        const bool software=source->window()->rendererInterface()->graphicsApi()==QSGRendererInterface::Software;
        // Qt native painted textures use RGBA; matching the raster target also
        // preserves premultiplied colored-glyph rounding before texture upload.
        image = QImage(QSize(qRound(source->width()*dpr), qRound(source->height()*dpr)),
                       software?QImage::Format_ARGB32_Premultiplied:QImage::Format_RGBA8888_Premultiplied);
        image.setDevicePixelRatio(dpr); image.fill(Qt::transparent);
        { QPainter painter(&image); painter.setRenderHint(QPainter::Antialiasing); source->paint(&painter); }
        setPosition(source->position()); setSize(source->size()); setSmooth(false); setVisible(false);
    }
    QSGNode *updatePaintNode(QSGNode *old,UpdatePaintNodeData *) override {
        auto *node=static_cast<QSGSimpleTextureNode *>(old);
        if(!node) {
            node=new QSGSimpleTextureNode;
            node->setTexture(window()->createTextureFromImage(image)); node->setOwnsTexture(true);
        }
        node->setRect(boundingRect()); node->setFiltering(smooth()?QSGTexture::Linear:QSGTexture::Nearest);
        return node;
    }
    QImage image;
};
// Independent retained painted image, with the source's Qt sampling geometry.
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
struct Fixture { QString id, text; qreal width = 740; int kind = 0, alignment = Qt::AlignLeft; bool clip = false; };
static QList<Fixture> fixtures() {
    QList<Fixture> result;
    for (QString id : {"plain", "bold", "italic", "link", "code", "combined"})
        result.append({id, "Try strong, emphasis, inline code, host links, and denied image fallback. <b>HTML stays literal</b>."});
    for (bool clip : {false,true}) {
        const QString suffix = clip ? "-clipped" : "";
        result.append({"mixed-decorated"+suffix, QString::fromUtf8("office é العربية שלום code text"),400,1,Qt::AlignLeft,clip});
        result.append({"wrapped-adjacent-linked"+suffix,"adjacentcode office linked strikeout wraps",120,1,Qt::AlignHCenter,clip});
        result.append({"tiny-right"+suffix,"fX code",1,1,Qt::AlignRight,clip});
    }
    return result;
}
static void configure(FormattedText &text, const Fixture &f) {
    text.setText(f.text); text.setLayoutWidth(f.width); text.setHorizontalAlignment(f.alignment);
    QFont font("DejaVu Sans"); font.setPixelSize(18); text.setFont(font);
    text.setCodeFont(QFont("DejaVu Sans Mono")); text.setCodeBackground(QColor("#eeeeee"));
    text.setSelectionColor(QColor("#8099bbff")); text.setFocusColor(Qt::red);
    QVariantList ranges;
    auto span = [&](QString word,int flags) { return QVariantMap{{"start",f.text.indexOf(word)},{"length",word.size()},{"flags",flags}}; };
    if (f.kind) {
        text.setCodeCornerRadius(4);
        ranges = {QVariantMap{{"start",0},{"length",6},{"flags",QMarkdownPrivate::Code|QMarkdownPrivate::Strikethrough}},
                  QVariantMap{{"start",6},{"length",4},{"flags",QMarkdownPrivate::Code|QMarkdownPrivate::Strikethrough}}};
    } else {
        if (f.id=="bold" || f.id=="combined") ranges.append(span("strong",QMarkdownPrivate::Strong));
        if (f.id=="italic" || f.id=="combined") ranges.append(span("emphasis",QMarkdownPrivate::Emphasis));
        if (f.id=="code" || f.id=="combined") ranges.append(span("inline code",QMarkdownPrivate::Code));
    }
    text.setFormatRanges(ranges);
    if (f.kind || f.id=="link" || f.id=="combined")
        text.setLinkSpans({QVariantMap{{"start",f.kind?6:f.text.indexOf("host links")},{"length",f.kind?4:10},{"logicalId",0},{"href","#test"}}});
}
static QJsonArray fontRecords(const FormattedText &text) {
    QJsonArray fonts;
    for(const auto &run:text.layout()->glyphRuns()) {
        QProcess match; match.start("fc-match", {"-f", "%{file}", run.rawFont().familyName()+":style="+run.rawFont().styleName()});
        match.waitForFinished(); const QString path=QString::fromUtf8(match.readAllStandardOutput());
        QFile file(path); QByteArray digest; if(file.open(QIODevice::ReadOnly)) digest=QCryptographicHash::hash(file.readAll(),QCryptographicHash::Sha256).toHex();
        QRawFont matched(path,run.rawFont().pixelSize());
        require(matched.fontTable("head")==run.rawFont().fontTable("head"),"resolved font file differs from glyph font");
        fonts.append(QJsonObject{{"family",run.rawFont().familyName()},{"style",run.rawFont().styleName()},
            {"matchedFile",path},{"fileSha256",QString::fromLatin1(digest)},
            {"headTableSha256",QString::fromLatin1(QCryptographicHash::hash(run.rawFont().fontTable("head"),QCryptographicHash::Sha256).toHex())}});
    }
    return fonts;
}
int main(int argc,char **argv) {
    QGuiApplication app(argc,argv);
    const auto args = app.arguments();
    if (args.size()!=3 || !QStringList{"static","motion","performance","view","calibrate"}.contains(args[1])) return 2;
    QDir().mkpath(args[2]);
    if(args[1]=="calibrate" || args[1]=="static" || args[1]=="motion") qputenv("BLUR_CALIBRATION", args[2].toUtf8());
    else qunsetenv("BLUR_CALIBRATION");
    QFile manifest(BLUR_MANIFEST); require(manifest.open(QIODevice::ReadOnly),"manifest missing");
    const auto source = QJsonDocument::fromJson(manifest.readAll()).object();
    QQmlEngine engine; QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
Rectangle {
    width: 800; height: 720; color: "white"
    property alias flick: flick
    property string label: ""
    property bool alternate: false
    Text { text: parent.label; x: 12; y: 8; font.pixelSize: 16 }
    MouseArea { x: 0; y: 0; width: parent.width; height: 40; onClicked: parent.alternate = !parent.alternate }
    Flickable {
        id: flick; x: 0; y: 48; width: parent.width; height: 640
        contentWidth: 2400; contentHeight: 3200; clip: true
        flickableDirection: Flickable.HorizontalAndVerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        flickDeceleration: 650
    }
})", QUrl());
    std::unique_ptr<QObject> root(component.create());
    require(bool(root),qPrintable(component.errorString()));
    auto *item=qobject_cast<QQuickItem *>(root.get());
    auto *flick=qobject_cast<QQuickItem *>(root->property("flick").value<QObject *>());
    auto *content=qobject_cast<QQuickItem *>(flick->property("contentItem").value<QObject *>());
    require(content,"Flickable content missing");
    QQuickWindow window; window.resize(800,720); window.setColor(Qt::white); item->setParentItem(window.contentItem());
    if(args[1]=="static" || args[1]=="motion") { window.resize(1600,720); item->setWidth(1600); }
    window.show(); require(QTest::qWaitForWindowExposed(&window),"window not exposed");
    QTest::qWait(100);
    const auto dpr=window.effectiveDevicePixelRatio();
    record({{"type","environment"},{"source",source},{"qt",qVersion()},{"dpr",dpr},{"platform",app.platformName()},
            {"graphicsApi",int(window.rendererInterface()->graphicsApi())},{"timing","capture-trace, not display frame rate"}});
    auto grab=[&] { auto image=window.grabWindow(); require(!image.isNull(),"capture failed"); return image; };
    auto settle=[&] { QTest::qWait(50); return grab(); };
    if (args[1]=="performance") {
        for (bool dense : {false,true}) {
            QElapsedTimer preparation; preparation.start();
            QList<FormattedText *> texts;
            for(int i=0;i<12;++i) {
                auto *text=new FormattedText(content); texts.append(text); text->setPosition({24,qreal(24+i*55)});
                text->setLayoutWidth(720); text->setText(QString("office code example with links and decorated text. ").repeated(2));
                text->setFormatRanges({QVariantMap{{"start",0},{"length",dense?96:6},{"flags",QMarkdownPrivate::Code|QMarkdownPrivate::Strikethrough}}}); text->setCodeCornerRadius(4);
            }
            grab(); const auto prep=preparation.nsecsElapsed(); settle();
            quint64 before=0; qint64 bytes=0; QList<QQuickPaintedItem *> identities;
            for(auto *text:texts) { before+=BlurProbe::paints(*text); auto *s=surface(*text); require(s,"missing workload surface"); identities.append(s); bytes+=qRound(s->width()*dpr)*qRound(s->height()*dpr)*4; }
            // Spies outlive warmed movement.
            std::vector<std::unique_ptr<QSignalSpy>> layoutSpies;
            for(auto *text:texts) layoutSpies.emplace_back(new QSignalSpy(text,&FormattedText::layoutChanged));
            const auto fonts=fontRecords(*texts.first());
            QList<qint64> frames;
            for(int i=0;i<120;++i) { QElapsedTimer t; t.start(); content->setY(-(i%40)*.27); QCoreApplication::processEvents(); grab(); frames.append(t.nsecsElapsed()); }
            quint64 after=0; qint64 afterBytes=0;
            for(int i=0;i<texts.size();++i) {
                after+=BlurProbe::paints(*texts[i]); auto *s=surface(*texts[i]); require(s==identities[i],"surface changed");
                require(layoutSpies[i]->isEmpty(),"workload layout rebuilt");
                afterBytes+=qRound(s->width()*dpr)*qRound(s->height()*dpr)*4;
            }
            require(afterBytes==bytes,"workload allocation changed");
            std::sort(frames.begin(),frames.end());
            record({{"type","performance"},{"fonts",fonts},{"dense",dense},{"preparationNs",double(prep)},{"medianNs",double(frames[60])},{"p95Ns",double(frames[114])},{"paints",double(after-before)},{"surfaces",12},{"allocationBytes",double(bytes)}});
            require(after==before,"workload repainted"); qDeleteAll(texts); content->setY(0);
        }
        return 0;
    }
    if(args[1]=="view") {
        QList<FormattedText *> texts; QList<ImageControl *> controls; QList<QQuickPaintedItem *> surfaces;
        const auto cases=fixtures();
        for(int i=0;i<24;++i) {
            auto f=cases[4+i%8];
            auto *clip=new QQuickItem(content); clip->setPosition({24.5,qreal(40+i*120)}); clip->setSize({560,100}); clip->setClip(f.clip);
            auto *text=new FormattedText(clip); configure(*text,f); texts.append(text);
        }
        settle();
        for(auto *text:texts) {
            auto *s=surface(*text); require(s,"view requires raster"); surfaces.append(s);
            auto *control=new ImageControl(s); control->setSmooth(!s->smooth()); controls.append(control);
        }
        auto update=[&] {
            const bool alternate=root->property("alternate").toBool();
            for(int i=0;i<surfaces.size();++i) { surfaces[i]->setVisible(!alternate); controls[i]->setVisible(alternate); }
            const bool smooth=alternate?controls[0]->smooth():surfaces[0]->smooth();
            root->setProperty("label",smooth?"BASELINE sampling — click to switch; drag below to scroll":"CANDIDATE nearest sampling — click to switch; drag below");
        };
        QTimer timer; QObject::connect(&timer,&QTimer::timeout,&window,update); timer.start(30); update();
        return app.exec();
    }
    for (const auto &f:fixtures()) {
        if(args[1]=="calibrate" && f.id!="mixed-decorated" && f.id!="code" && f.id!="combined") continue;
        if(!qEnvironmentVariable("BLUR_FIXTURE").isEmpty() && f.id!=qEnvironmentVariable("BLUR_FIXTURE")) continue;
        flick->setProperty("pixelAligned",false);
        QQuickItem clip(content); clip.setPosition({24.5,40}); clip.setSize({560,100}); clip.setClip(f.clip);
        FormattedText text(&clip); configure(text,f); settle();
        auto *s=surface(text);
        const bool expectedSmooth=source["variant"]=="baseline" || (window.rendererInterface()->graphicsApi()!=QSGRendererInterface::OpenGL && window.rendererInterface()->graphicsApi()!=QSGRendererInterface::Vulkan);
        if(s) require(s->smooth()==expectedSmooth,"unexpected smoothing policy");
        if(args[1]=="calibrate") {
            require(s && s->property("blurCaptured").toBool(),"control-calibration: first real paint missing");
            require(s->property("blurRawEqual").toBool(),"control-calibration: raw raster mismatch");
            PaintedControl painted(s); settle();
            const auto before=BlurProbe::paints(text);
            QSignalSpy calibrationLayouts(&text,&FormattedText::layoutChanged);
            for(int y=0;y<4;++y) for(int x=0;x<4;++x) {
                flick->setProperty("contentX",x*.25/dpr); flick->setProperty("contentY",y*.25/dpr); settle();
                s->setOpacity(1); painted.setOpacity(0); const auto sourceFrame=settle();
                s->setOpacity(0); painted.setOpacity(1); const auto controlFrame=settle();
                s->setOpacity(1); painted.setOpacity(0); settle();
                const bool equal=sourceFrame==controlFrame;
                const auto id=f.id+QString("-%1-%2").arg(x).arg(y);
                record({{"type","calibration"},{"fixture",f.id},{"id",id},{"equal",equal},
                    {"paints",double(BlurProbe::paints(text)-before)},{"layoutRebuilds",calibrationLayouts.size()}});
                if(!equal) { sourceFrame.save(args[2]+"/"+id+"-source.png"); controlFrame.save(args[2]+"/"+id+"-control.png"); }
                require(BlurProbe::paints(text)==before && calibrationLayouts.isEmpty(),"control-calibration: warmed counters changed");
                require(s->smooth()==expectedSmooth,"control-calibration: smoothing changed");
                require(equal,"control-calibration: same-coordinate framebuffer mismatch");
            }
            flick->setProperty("contentX",0); flick->setProperty("contentY",0); settle();
            continue;
        }
        const auto cells=text.rangeRectangles(0,f.text.size()); const auto height=text.logicalHeight();
        const auto interactions=BlurProbe::geometry(text); const auto *layout=text.layout();
        QJsonArray rectangles, fonts;
        for(const auto &rect:cells) rectangles.append(QJsonArray{rect.x(),rect.y(),rect.width(),rect.height()});
        fonts=fontRecords(text);
        record({{"type","fixture"},{"fixture",f.id},{"text",f.text},{"width",f.width},{"height",height},{"cells",rectangles},{"interactions",interactions},{"fonts",fonts},
                {"surfaceId",QString::number(quintptr(s),16)},{"surfaceCount",s?1:0},{"allocationBytes",s?double(qRound(s->width()*dpr)*qRound(s->height()*dpr)*4):0}});
        QSignalSpy layouts(&text,&FormattedText::layoutChanged);
        const auto pos=s?s->position():QPointF(); const auto size=s?s->size():QSizeF();
        QQuickItem controlClip(content); controlClip.setPosition({824.5,40}); controlClip.setSize({560,100}); controlClip.setClip(f.clip);
        std::unique_ptr<ImageControl> control(s?new ImageControl(s):nullptr);
        if(control) { control->setParentItem(&controlClip); control->setSmooth(s->smooth()); control->setVisible(false); }
        std::unique_ptr<PaintedControl> authoritative(s?new PaintedControl(s):nullptr);
        if(s) require(s->property("blurRawEqual").toBool(),"control-calibration: raw raster mismatch");
        struct Pose { QString id; QImage image; QPointF content; };
        QList<Pose> poses;
        settle(); const auto before=BlurProbe::paints(text); QElapsedTimer trace; trace.start();
        auto check=[&](QString id) {
            const auto image=grab();
            require(text.rangeRectangles(0,f.text.size())==cells && text.logicalHeight()==height,"geometry changed");
            require(BlurProbe::geometry(text)==interactions,"interaction cells or links changed");
            require(layouts.isEmpty() && text.layout()==layout,"layout rebuilt during movement"); require(BlurProbe::paints(text)==before,"warmed raster paint");
            require(surface(text)==s,"surface identity changed");
            if(s) require(s->position()==pos && s->size()==size,"surface allocation changed");
            if(s) require(s->smooth()==expectedSmooth,"smoothing policy changed during measurement");
            poses.append({id,image,content->position()});
            if(control && args[1]=="static") {
                control->setVisible(true); const auto translated=settle(); control->setVisible(false); settle();
                const int half=translated.width()/2;
                record({{"type","translatedDiagnostic"},{"fixture",f.id},{"id",id},
                    {"equal",translated.copy(0,0,half,translated.height())==translated.copy(half,0,half,translated.height())}});
            }
            const auto scene=text.mapToScene({});
            const auto transform=text.itemTransform(window.contentItem(),nullptr);
            double edges=0; int count=0;
            if(!f.kind) {
                QRect roi(qRound(scene.x()*dpr),qRound(scene.y()*dpr),qRound(27*dpr),qRound(24*dpr));
                roi=roi.intersected(image.rect().adjusted(0,0,-1,-1));
                for(int y=roi.top();y<roi.bottom();++y) for(int x=roi.left();x<roi.right();++x) {
                    edges+=std::abs(qGray(image.pixel(x+1,y))-qGray(image.pixel(x,y)))+std::abs(qGray(image.pixel(x,y+1))-qGray(image.pixel(x,y))); count+=2;
                }
            }
            record({{"type","capture"},{"fixture",f.id},{"id",id},{"ns",double(trace.nsecsElapsed())},{"velocityY",flick->property("verticalVelocity").toDouble()},{"moving",flick->property("moving").toBool()},{"actualContentX",content->x()},{"actualContentY",content->y()},{"clipX",clip.x()},{"clipY",clip.y()},{"clipWidth",clip.width()},{"clipHeight",clip.height()},{"clipped",clip.clip()},{"contentX",flick->property("contentX").toDouble()},{"contentY",flick->property("contentY").toDouble()},{"physicalX",scene.x()*dpr},{"physicalY",scene.y()*dpr},{"pixelAligned",flick->property("pixelAligned").toBool()},{"surfaceSmooth",s&&s->smooth()},{"layoutRebuilds",layouts.size()},{"comparisonStage","live-capture-only"},{"sourceTransform",QJsonArray{transform.m11(),transform.m12(),transform.m21(),transform.m22(),transform.dx(),transform.dy()}},{"edgeContrast",count?edges/count:0},{"paints",double(BlurProbe::paints(text)-before)}});
            if(args[1]=="static" || (id.startsWith("motion-") || id.startsWith("drag-"))) require(image.save(args[2]+"/"+f.id+"-"+id+".png"),"save failed");
            return image;
        };
        if(args[1]=="static") {
            const auto origin=check("origin");
            for(int y=0;y<4;++y) for(int x=0;x<4;++x) {
                flick->setProperty("contentX",x*.25/dpr); flick->setProperty("contentY",y*.25/dpr); settle();
                check(QString("%1-%2").arg(x).arg(y));
            }
            flick->setProperty("contentX",0); flick->setProperty("contentY",0); settle(); require(check("return")==origin,"return image differs");
        } else {
            for(bool aligned:{false,true}) {
                flick->setProperty("pixelAligned",aligned); flick->setProperty("contentY",0); settle();
                const auto origin=check(QString("motion-origin-%1").arg(aligned));
                QTest::mousePress(&window,Qt::LeftButton,Qt::NoModifier,{480,400});
                for(int step=0;step<8;++step) {
                    QTest::mouseMove(&window,{480,400-step*3},12);
                    check(QString("drag-%1-%2").arg(aligned).arg(step));
                }
                require(flick->property("contentY").toDouble()>0,"drag did not accelerate content");
                QTest::mouseRelease(&window,Qt::LeftButton,Qt::NoModifier,{480,379});
                require(QMetaObject::invokeMethod(flick,"flick",Q_ARG(qreal,0.),Q_ARG(qreal,-350.)),"flick invocation failed");
                bool forward=false, reverse=false, reversed=false; QElapsedTimer flickTime; flickTime.start();
                int frames=0;
                for(int frame=0;frame<120;++frame) {
                    QTest::qWait(8);
                    if(!reversed && flickTime.elapsed()>=120) {
                        require(flick->property("moving").toBool() && flick->property("verticalVelocity").toDouble()>0,"forward motion settled before reversal");
                        require(QMetaObject::invokeMethod(flick,"flick",Q_ARG(qreal,0.),Q_ARG(qreal,240.)),"reverse flick failed");
                        reversed=true;
                    }
                    const auto velocity=flick->property("verticalVelocity").toDouble();
                    forward |= velocity>0; reverse |= velocity<0;
                    check(QString("motion-%1-%2").arg(aligned).arg(frame));
                    ++frames;
                    if(reversed && frame>=35 && !flick->property("moving").toBool()) break;
                }
                record({{"type","motionSummary"},{"fixture",f.id},{"pixelAligned",aligned},{"frames",frames},{"forward",forward},{"reverse",reverse},{"reversedInFlight",reversed}});
                require(forward && reverse && reversed,"motion failed to exercise both directions");
                require(!flick->property("moving").toBool(),"flick did not settle");
                flick->setProperty("contentY",0); settle();
                require(check(QString("motion-return-%1").arg(aligned))==origin,"motion return differs");
            }
        }
        // Replay only after all live motion ends. The static container shares
        // the Flickable viewport/clip, but has no animation or content binding.
        QQuickItem replayContainer(flick);
        clip.setParentItem(&replayContainer);
        for(const auto &pose:poses) {
            replayContainer.setPosition(pose.content);
            if(s) s->setOpacity(1);
            if(authoritative) authoritative->setOpacity(0);
            const auto replayed=settle();
            const bool replayEqual=replayed==pose.image;
            record({{"type","replay"},{"fixture",f.id},{"id",pose.id},{"equal",replayEqual}});
            if(!replayEqual) { pose.image.save(args[2]+"/"+f.id+"-"+pose.id+"-live.png"); replayed.save(args[2]+"/"+f.id+"-"+pose.id+"-replay.png"); }
            require(replayEqual,"replay-failure: source differs from live capture");
            if(s) {
                s->setOpacity(0); authoritative->setOpacity(1);
                const auto independent=settle();
                const bool controlEqual=independent==replayed;
                record({{"type","poseControl"},{"fixture",f.id},{"id",pose.id},{"equal",controlEqual}});
                if(!controlEqual) { replayed.save(args[2]+"/"+f.id+"-"+pose.id+"-source.png"); independent.save(args[2]+"/"+f.id+"-"+pose.id+"-control.png"); }
                require(controlEqual,"control-calibration: replayed framebuffer mismatch");
                s->setOpacity(1); authoritative->setOpacity(0);
                require(surface(text)==s && s->position()==pos && s->size()==size,"renderer-failure: replay surface changed");
            }
            require(BlurProbe::paints(text)==before && layouts.isEmpty() && text.layout()==layout,"renderer-failure: replay counters changed");
            require(BlurProbe::geometry(text)==interactions && text.rangeRectangles(0,f.text.size())==cells && text.logicalHeight()==height,"renderer-failure: replay geometry changed");
        }
        clip.setParentItem(content); settle();
        // Mutation gates deliberately occur after warmed motion accounting.
        if(s) {
            control->setVisible(false); clip.setClip(false); settle();
            const QList<std::function<void()>> mutations={
                [&]{text.setSelectionEnd(3);}, [&]{text.setSearchRanges({QVariantMap{{"start",0},{"length",2}}});},
                [&]{text.setFocusedLink(0);}, [&]{text.setColor(Qt::red);}, [&]{text.setText("NEW "+f.text.mid(4));}};
            int i=0;
            for(const auto &mutate:mutations) {
                if(i==2 && text.linkSpans().isEmpty()) { ++i; continue; }
                const auto old=BlurProbe::paints(text); const auto image=grab(); mutate(); const auto updated=settle();
                require(BlurProbe::paints(text)>old,"mutation failed to repaint"); require(image!=updated,"mutation produced no visible update");
                record({{"type","mutation"},{"fixture",f.id},{"index",i++},{"updated",true}});
            }
        }
    }
}
