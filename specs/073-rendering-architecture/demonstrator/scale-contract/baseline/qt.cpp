#include "private/formattedtext.h"
#include <QGuiApplication>
#include <QSocketNotifier>
#include <unistd.h>
#include <QQuickWindow>
#include <QSGTextNode>
#include <QSGSimpleTextureNode>
#include <QSGSimpleRectNode>
#include <QSGTexture>
#include <QSGRendererInterface>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QElapsedTimer>
#include <QTimer>
#include <QScreen>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QClipboard>
#include <QPainter>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QVulkanInstance>
#include <QVulkanFunctions>
#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>

static QElapsedTimer elapsed;
static void record(QJsonObject object) {
    object["monotonicMs"] = elapsed.nsecsElapsed()/1e6;
    puts(QJsonDocument(object).toJson(QJsonDocument::Compact).constData());
    fflush(stdout);
}
class DemoAccess {
public:
    static QJsonObject geometry(const FormattedText &text) {
        QJsonArray cells, lines, raw;
        for(int j=0;j<text.m_layout->lineCount();++j) {
            auto l=text.m_layout->lineAt(j);
            for(int pos=l.textStart();pos<l.textStart()+l.textLength();++pos) {
                QJsonArray runs;
                for(auto r:l.glyphRuns(pos,1))runs.append(QJsonObject{{"x",r.boundingRect().x()},{"width",r.boundingRect().width()},{"rtl",r.isRightToLeft()}});
                raw.append(QJsonObject{{"line",j},{"pos",pos},{"valid",text.m_layout->isValidCursorPosition(pos)},
                    {"a",l.cursorToX(pos,QTextLine::Leading)},{"b",l.cursorToX(pos,QTextLine::Trailing)},{"runs",runs}});
            }
        }
        for (const auto &c : text.m_cursorCells)
            cells.append(QJsonObject{{"start",c.start},{"end",c.end},{"rtl",c.reversed},
                {"x",c.rect.x()},{"y",c.rect.y()},{"width",c.rect.width()},{"height",c.rect.height()},
                {"leftHit",text.cursorAt(c.rect.left()+.01,c.rect.center().y())},
                {"rightHit",text.cursorAt(c.rect.right()-.01,c.rect.center().y())}});
        for (const auto &l : text.m_cursorLines)
            lines.append(QJsonObject{{"start",l.start},{"top",l.top},{"bottom",l.bottom}});
        return {{"cells",cells},{"lines",lines},{"raw",raw},{"layoutText",text.m_layout->text()}};
    }
    static void prepare(FormattedText &text) { text.updatePolish(); }
    static QTextLayout *layout(FormattedText &text) { return text.m_layout.get(); }
    static QRectF ink(const FormattedText &text) { return text.m_inkBounds; }
    static bool native(const FormattedText &text) { return text.sceneGraphEligible(); }
};
class Paragraph final : public QQuickItem {
public:
    FormattedText text;
    bool native = false, curve = false, rule = false, table = false;
    quint64 paints = 0, uploads = 0, nodes = 0;
    qreal zoom = 1, lastFactor = 0;
    QSize lastPixels;
    QPointF lastPhase;
    bool forceRaster = true;
    explicit Paragraph(QQuickItem *parent) : QQuickItem(parent) { setFlag(ItemHasContents); }
    void invalidate(bool style=false) { forceRaster |= style; update(); }
protected:
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override {
        if (native && old) return old;
        if (rule) { delete old; return new QSGSimpleRectNode(QRectF(0,0,width(),1), QColor("#85919b")); }
        if (native) {
            delete old;
            auto *root = new QSGNode;
            if (table) root->appendChildNode(new QSGSimpleRectNode(QRectF(0,height()-1,width(),1), QColor("#cbd5df")));
            for (const auto &rect : text.selectionRectangles())
                root->appendChildNode(new QSGSimpleRectNode(rect,QColor("#99bbee")));
            auto *node = window()->createTextNode();
            node->setRenderType(curve ? QSGTextNode::CurveRendering : QSGTextNode::QtRendering);
            node->setColor(Qt::black);
            node->addTextLayout({}, DemoAccess::layout(text));
            root->appendChildNode(node);
            ++nodes;
            return root;
        }
        // Paint the visible intersection at its actual physical phase. The texture
        // has no DPR metadata: its target rectangle is explicitly in local units.
        const qreal dpr = window()->devicePixelRatio(), factor = dpr*zoom;
        const QPointF scene = mapToScene({});
        QRectF bounds = DemoAccess::ink(text).united(QRectF(0,0,width(),height())).adjusted(-2,-2,2,2);
        const QRectF visible((-scene.x())/zoom, (-scene.y())/zoom,
                             window()->width()/zoom, window()->height()/zoom);
        bounds = bounds.intersected(visible);
        if (bounds.isEmpty()) { delete old;return nullptr; }
        const QPointF origin(std::floor((scene.x()+bounds.left()*zoom)*dpr),
                             std::floor((scene.y()+bounds.top()*zoom)*dpr));
        const QPointF end(std::ceil((scene.x()+bounds.right()*zoom)*dpr),
                          std::ceil((scene.y()+bounds.bottom()*zoom)*dpr));
        const QSize pixels(int(end.x()-origin.x()),int(end.y()-origin.y()));
        const QPointF phase=scene*dpr-origin;
        const QRectF target((origin-scene*dpr)/factor,QSizeF(pixels)/factor);
        if(old && !forceRaster && lastPixels==pixels && lastPhase==phase && lastFactor==factor) {
            static_cast<QSGSimpleTextureNode *>(old)->setRect(target);
            return old;
        }
        delete old;
        lastPixels=pixels;lastPhase=phase;lastFactor=factor;forceRaster=false;
        QImage image(pixels,QImage::Format_RGBA8888_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.translate(scene*dpr-origin);
        painter.scale(factor,factor);
        text.paint(&painter);
        painter.end();
        auto *node = new QSGSimpleTextureNode;
        node->setTexture(window()->createTextureFromImage(image));
        node->setOwnsTexture(true);
        node->setFiltering(QSGTexture::Nearest);
        node->setRect(target);
        ++paints; ++uploads;
        return node;
    }
};
class Window final : public QQuickWindow {
public:
    QJsonObject manifest, fixture;
    QQuickItem *content;
    std::vector<std::unique_ptr<Paragraph>> paragraphs;
    QFont regular, mono;
    QTimer motion;
    bool curve, moving = false;
    qreal offset = 0, speed = .25, zoom = 1;
    int selectionBlock = -1, anchor = 0;
    QString mode;
    Window(QJsonObject data, QJsonObject f, bool c, QString reduction)
        : manifest(data),fixture(f),curve(c),mode(reduction) {
        const auto viewport=manifest["viewport"].toArray();
        resize(viewport[0].toInt(),viewport[1].toInt());
        setColor(QColor(manifest["background"].toString()));
        content=new QQuickItem(contentItem());
        content->setTransformOrigin(QQuickItem::TopLeft);
        content->setPosition({24,24});
        const auto fonts=manifest["fonts"].toObject();
        for (auto it=fonts.begin();it!=fonts.end();++it) {
            const int id=QFontDatabase::addApplicationFont(it.value().toString());
            if(id<0) qFatal("Cannot load fixture font");
            if(it.key()=="regular") regular=QFont(QFontDatabase::applicationFontFamilies(id).first());
            if(it.key()=="mono") mono=QFont(QFontDatabase::applicationFontFamilies(id).first());
        }
        regular.setPixelSize(manifest["fontSize"].toInt());
        mono.setPixelSize(manifest["fontSize"].toInt());
        prepare();
        connect(&motion,&QTimer::timeout,this,[this]{
            offset+=speed; if(std::abs(offset)>180) speed=-speed;
            place();
        });
        connect(this,&QWindow::widthChanged,this,[this]{prepare();});
        connect(this,&QWindow::heightChanged,this,[this]{place();});
        connect(this,&QWindow::screenChanged,this,[this]{place();});
        title();
    }
    void title() {
        int fallback=0;
        QStringList regions;
        for(auto &p:paragraphs) {
            if(!p->native && !p->rule) ++fallback;
            regions.append(p->rule?"rule":p->native?"native":"raster");
        }
        setTitle(QString("073 %1 | %2 | %3 fallback blocks ["+regions.join(",")+"] | Space pause, S slow, F flick, R reverse, 0 reset, +/- zoom, drag select, Ctrl+C")
                 .arg(curve?"curve":"distance",fixture["id"].toString()).arg(fallback));
    }
    void prepare() {
        const auto start=elapsed.nsecsElapsed();
        paragraphs.clear();
        qreal y=0;
        if(mode=="empty") return;
        auto blocks=fixture["blocks"].toArray();
        if(mode=="rectangle") blocks=QJsonArray{QJsonObject{{"kind","rule"}}};
        for(const auto &value:blocks) {
            auto b=value.toObject();
            auto p=std::make_unique<Paragraph>(content);
            p->curve=curve;
            p->rule=b["kind"]=="rule"; p->table=b["kind"]=="table";
            auto &t=p->text;
            t.setText(b["text"].toString());
            t.setFont(regular); t.setCodeFont(mono);
            t.setCodeBackground(QColor("#eeeeee")); t.setCodeCornerRadius(4);
            t.setSelectionColor(QColor("#99bbee"));
            const auto padding=manifest["padding"].toArray();
            t.setCodeHorizontalPadding(padding[0].toDouble());
            t.setCodeVerticalPadding(padding[1].toDouble());
            QVariantList formats;
            for(const auto &s:b["spans"].toArray()) {
                auto span=s.toObject();
                const QString k=span["kind"].toString();
                formats.append(QVariantMap{{"start",span["start"].toInt()},{"length",span["length"].toInt()},
                    {"flags",k=="code"?4:k=="bold"?2:k=="italic"?1:8}});
            }
            t.setFormatRanges(formats);
            t.setProperty("demoObjects",b["objects"].toArray().toVariantList());
            t.setLayoutWidth(qMax(1.,qMin(fixture["width"].toDouble(),(width()-48)/zoom)));
            DemoAccess::prepare(t);
            p->native=DemoAccess::native(t);
            p->setPosition({0,y});
            p->setSize({t.layoutWidth(),p->rule?12:t.logicalHeight()});
            y+=p->height()+18;
            paragraphs.push_back(std::move(p));
        }
        record({{"stage","prepared"},{"blocks",int(paragraphs.size())},{"preparationMs",(elapsed.nsecsElapsed()-start)/1e6}});
        place(); title();
    }
    void place() {
        content->setScale(zoom); content->setY(24+offset);
        for(auto &p:paragraphs) { p->zoom=zoom; if(!p->native) p->invalidate(); }
        update();
    }
    void counters(QString stage) {
        qint64 paints=0, uploads=0,nodes=0;
        QJsonArray regions;
        for(auto &p:paragraphs) {
            paints+=p->paints;uploads+=p->uploads;nodes+=p->nodes;
            regions.append(p->rule?"rule":p->native?"native":"phase-raster-fallback");
        }
        record({{"stage",stage},{"rasterPaints",paints},{"uploads",uploads},{"nativeNodes",nodes},{"regions",regions}});
    }
    static void selectRange(FormattedText &text,int anchor,int cursor) {
        // Prepared selection rectangles take a logical interval, independent
        // of the pointer's direction through visual bidi cells.
        text.setSelectionStart(qMin(anchor,cursor));
        text.setSelectionEnd(qMax(anchor,cursor));
    }
    QString selectedSource() const {
        QString copied;
        for(auto &p:paragraphs) {
            int a=qMin(p->text.selectionStart(),p->text.selectionEnd());
            int b=qMax(p->text.selectionStart(),p->text.selectionEnd());
            if(b>a)copied+=p->text.text().mid(a,b-a);
        }
        return copied;
    }
public:
    void studyCheck(QString checkpoint) {
        bool inverse=true,contained=true;
        QJsonArray geometry;
        for(auto &p:paragraphs) {
            if(p->rule)continue;
            auto g=DemoAccess::geometry(p->text);
            for(auto value:g["cells"].toArray()) {
                auto c=value.toObject();
                QPointF local(c["x"].toDouble()+c["width"].toDouble()/2,
                              c["y"].toDouble()+c["height"].toDouble()/2);
                auto restored=p->mapFromScene(p->mapToScene(local));
                inverse &= QLineF(local,restored).length()<=.0001;
                inverse &= p->text.cursorAt(local.x(),local.y())==
                           p->text.cursorAt(restored.x(),restored.y());
            }
            auto &text=p->text;
            for(const auto &value:text.property("demoObjects").toList()) {
                auto object=value.toMap();
                int begin=object["start"].toInt(),end=begin+object["length"].toInt();
                auto rects=text.rangeRectangles(begin,end);
                bool valid=rects.size()==1;
                if(valid) {
                    auto rect=rects.first();
                    int left=text.cursorAt(rect.left()+.01,rect.center().y());
                    int right=text.cursorAt(rect.right()-.01,rect.center().y());
                    valid=qAbs(rect.width()-object["width"].toDouble())<.05
                        && rect.height()+.05>=object["ascent"].toDouble()+object["descent"].toDouble()
                        && rect.top()>=0 && rect.bottom()<=text.logicalHeight()+.05
                        && qMin(left,right)==begin && qMax(left,right)==end;
                }
                contained &= valid;
            }
            geometry.append(g);
        }
        QString forward,reverse;
        for(auto &p:paragraphs) if(!p->text.text().isEmpty()) {
            selectRange(p->text,0,p->text.text().size());forward=selectedSource();
            selectRange(p->text,p->text.text().size(),0);reverse=selectedSource();
            selectRange(p->text,0,0);break;
        }
        record({{"stage","study-check"},{"checkpoint",checkpoint},{"inverse",inverse},{"contained",contained},
                {"forward",forward},{"reverse",reverse},{"geometry",geometry}});
    }
protected:
    bool event(QEvent *event) override {
        if(event->type()==QEvent::DevicePixelRatioChange)
            record({{"stage","scale-notification"},{"dpr",devicePixelRatio()}});
        return QQuickWindow::event(event);
    }
    void keyPressEvent(QKeyEvent *event) override {
        switch(event->key()) {
        case Qt::Key_Space: moving=!moving; break;
        case Qt::Key_S: speed=.25; moving=true; break;
        case Qt::Key_F: speed=12; moving=true; break;
        case Qt::Key_R: speed=-speed; break;
        case Qt::Key_0: offset=0;zoom=1;moving=false;prepare();break;
        case Qt::Key_Plus: case Qt::Key_Equal: zoom=qMin(3.,zoom+.125);prepare();break;
        case Qt::Key_Minus: zoom=qMax(.5,zoom-.125);prepare();break;
        case Qt::Key_C:
            if(event->modifiers()&Qt::ControlModifier) {
                const QString copied=selectedSource();
                QGuiApplication::clipboard()->setText(copied);
                record({{"stage","copied"},{"source",copied}});
            }
            break;
        default: QQuickWindow::keyPressEvent(event);return;
        }
        moving?motion.start(16):motion.stop();
        place(); counters("interaction");
    }
    void mousePressEvent(QMouseEvent *event) override {
        selectionBlock=-1;
        for(size_t i=0;i<paragraphs.size();++i) {
            auto &p=paragraphs[i];auto local=p->mapFromScene(event->position());
            if(p->boundingRect().contains(local)) {
                selectionBlock=int(i);anchor=p->text.cursorAt(local.x(),local.y());break;
            }
        }
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        if(selectionBlock<0 || !(event->buttons()&Qt::LeftButton)) return;
        auto &p=paragraphs[size_t(selectionBlock)];
        auto local=p->mapFromScene(event->position());
        selectRange(p->text,anchor,p->text.cursorAt(local.x(),local.y()));
        // Native geometry resources are replaced only for selection changes.
        p->setVisible(false);p->setVisible(true);
        p->native=false; // Explicit raster inspection for selection foreground/geometry.
        p->invalidate(true);title();
    }
};
int main(int argc,char **argv) {
    elapsed.start(); record({{"stage","startup"}});
    QGuiApplication app(argc,argv);
    const auto args=app.arguments();
    if(args.size()<5) return 2;
    QFile file(args[1]);if(!file.open(QIODevice::ReadOnly))return 2;
    QJsonParseError error;
    auto manifest=QJsonDocument::fromJson(file.readAll(),&error).object();
    if(error.error!=QJsonParseError::NoError)return 2;
    QJsonObject fixture;
    for(auto f:manifest["fixtures"].toArray())if(f.toObject()["id"]==args[2])fixture=f.toObject();
    if(fixture.isEmpty()) return 2;
    const QString output=args[4];
    if(!QDir().mkpath(output))return 4;
    QString reduction="combined";
    for(const auto &arg:args) if(arg.startsWith("--case="))reduction=arg.mid(7);
    Window window(manifest,fixture,args[3]=="curve",reduction);
    const bool live=args.contains("--scale-live");
    const bool interactive=args.contains("--interactive") || live, closePath=args.contains("--close");
    int phase=0,frames=0,motionFrame=0;
    const bool motionReplay=args.contains("--motion-replay");
    QImage before;
    bool pending=false;
    QImage settled;
    const auto capture=[&](QString name) {
        record({{"stage","capture"},{"name",name}});
        auto image=window.grabWindow();
        if(image.isNull() || !image.save(output+"/"+name+".png")) {
            record({{"stage","capture-failed"},{"name",name}}); app.exit(4);
            return QImage();
        }
        record({{"stage","capture-mapping"},{"name",name},
                {"output",window.screen()->name()},{"window",QJsonArray{window.width(),window.height()}},
                {"buffer",QJsonArray{image.width(),image.height()}},
                {"dpr",window.devicePixelRatio()},{"zoom",window.zoom}});
        return image;
    };
    const auto mapping=[&](QString reason) {
        record({{"stage","mapping"},{"reason",reason},{"output",window.screen()->name()},
                {"window",QJsonArray{window.width(),window.height()}},
                {"dpr",window.devicePixelRatio()},{"zoom",window.zoom},
                {"nominalFontSize",manifest["fontSize"]},
                {"windowId",QString::number(window.winId())}});
    };
    QObject::connect(&window,&QWindow::screenChanged,&window,[&]{mapping("screen-change");});

    QSocketNotifier input(STDIN_FILENO,QSocketNotifier::Read,&window);
    input.setEnabled(live);
    QByteArray commands;
    QObject::connect(&input,&QSocketNotifier::activated,&window,[&]{
        char data[1024];auto n=::read(STDIN_FILENO,data,sizeof(data));
        if(n<=0){input.setEnabled(false);app.quit();return;}
        commands.append(data,n);
        while(commands.contains('\n')) {
            auto at=commands.indexOf('\n');auto command=commands.left(at);commands.remove(0,at+1);
            if(command=="quit"){window.counters("shutdown");app.quit();return;}
            if(command.startsWith("checkpoint ")) {
                auto label=QString::fromUtf8(command.mid(11));
                window.motion.stop();window.offset=0;window.zoom=1;window.place();
                mapping(label);window.studyCheck(label);
                QTimer::singleShot(100,&window,[&,label]{
                    auto image=capture("checkpoint-"+label);
                    record({{"stage","checkpoint"},{"id",label},
                        {"buffer",QJsonArray{image.width(),image.height()}}});
                });
            }
        }
    });
    // The receiver is the window: queued frame callbacks are disconnected before
    // referenced state is destroyed. A queued callback never owns a layout/node.
    QObject::connect(&window,&QQuickWindow::frameSwapped,&window,[&] {
        if(!window.isExposed() || pending)return;
        if(phase==0) {
            record({{"stage","ready"},{"qt",qVersion()},
                    {"api",int(window.rendererInterface()->graphicsApi())},
                    {"platform",QGuiApplication::platformName()},
                    {"dpr",window.devicePixelRatio()},
                    {"framebufferWidth",qRound(window.width()*window.devicePixelRatio())},
                    {"framebufferHeight",qRound(window.height()*window.devicePixelRatio())},
                    {"refreshHz",window.screen()->refreshRate()}});
            mapping("ready");window.studyCheck("ready");
            phase=1;
            if(interactive)return;
        }
        if(interactive)return;
        if(motionReplay && phase==2 && motionFrame<60) {
            const qreal step=motionFrame<20?.25:motionFrame<40?12:-12;
            window.offset+=step;
            record({{"stage","motion-frame"},{"frame",motionFrame},{"offset",window.offset},{"step",step}});
            ++motionFrame;window.place();return;
        }
        if(++frames<3){window.update();return;}
        frames=0;pending=true;
        QTimer::singleShot(0,&window,[&] {
            if(phase==1) {
                if(reduction=="capture" || reduction=="combined")before=capture("stationary");
                phase=2;
            } else if(phase==2) {
                if(motionReplay) before=capture("motion-pose");
                else if(reduction=="motion" || reduction=="combined"){window.offset=17.375;window.place();}
                phase=3;
            } else if(phase==3) {
                if(reduction=="combined") {
                    auto pose=capture("pose");
                    if(pose.isNull())return;
                    if(motionReplay) {
                        record({{"stage","replay"},{"operation","captured-motion-pose"},{"exact",before==pose}});
                        if(before!=pose){app.exit(5);return;}
                    }
                    before=pose;
                }
                phase=4;
            } else if(phase==4) {
                if(reduction=="combined") {
                    auto replay=capture("replay");
                    if(replay.isNull())return;
                    record({{"stage","replay"},{"exact",before==replay}});
                    if(before!=replay){app.exit(5);return;}
                }
                window.counters("translation-counters");
                if(reduction=="combined") { window.offset=-40.25;window.place();phase=5; }
                else { window.motion.stop();window.counters("shutdown");phase=12;
                       if(closePath)window.close();else app.quit(); }
            } else if(phase==5) {
                capture("clip-entry");window.offset=0;window.place();phase=6;
            } else if(phase==6) {
                settled=capture("settled");window.resize(620,480);phase=7;
            } else if(phase==7) {
                capture("resize");window.zoom=1.25;window.prepare();phase=8;
            } else if(phase==8) {
                capture("zoom");window.zoom=1;window.resize(800,600);window.prepare();phase=9;
            } else if(phase==9) {
                auto reset=capture("reset");
                if(reset.isNull())return;
                record({{"stage","replay"},{"exact",settled==reset},{"operation","resize-zoom-reset"}});
                if(settled!=reset){app.exit(5);return;}
                for(auto &p:window.paragraphs) if(!p->text.text().isEmpty()) {
                    Window::selectRange(p->text,p->text.text().size(),0);
                    p->native=false;p->invalidate(true);break;
                }
                window.place();phase=10;
            } else if(phase==10) {
                if(capture("selected").isNull())return;
                record({{"stage","selected-source"},{"source",window.selectedSource()}});
                window.motion.stop();window.counters("shutdown");phase=12;
                if(closePath)window.close();else app.quit();
            }
            pending=false;
            window.update();
        });
    },Qt::QueuedConnection);
    bool deviceRecorded=false;
    QObject::connect(&window,&QQuickWindow::afterRendering,&window,[&] {
        if(deviceRecorded)return;
        deviceRecorded=true;
        QString device="unavailable";
        auto *ri=window.rendererInterface();
        if(ri->graphicsApi()==QSGRendererInterface::OpenGL) {
            if(auto *context=QOpenGLContext::currentContext())
                device=QString::fromLatin1(reinterpret_cast<const char *>(context->functions()->glGetString(GL_RENDERER)));
        } else if(ri->graphicsApi()==QSGRendererInterface::Vulkan && window.vulkanInstance()) {
            auto *physical=static_cast<VkPhysicalDevice *>(ri->getResource(&window,QSGRendererInterface::PhysicalDeviceResource));
            if(physical) {
                VkPhysicalDeviceProperties props;
                window.vulkanInstance()->functions()->vkGetPhysicalDeviceProperties(*physical,&props);
                device=QString::fromLatin1(props.deviceName);
            }
        }
        record({{"stage","device"},{"name",device}});
    },Qt::DirectConnection);
    window.show();
    const int result=app.exec();
    window.motion.stop();
    QObject::disconnect(&window,nullptr,&window,nullptr);
    window.hide();
    window.releaseResources();
    record({{"stage","exit"},{"code",result}});
    return result;
}
