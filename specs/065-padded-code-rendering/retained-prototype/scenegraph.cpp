#include "private/formattedtext.h"
#include "private/inline.h"
#include <QGuiApplication>
#include <QQuickWindow>
#include <QSGImageNode>
#include <QSGTexture>
#include <QSGRendererInterface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#if QT_CONFIG(vulkan) && __has_include(<vulkan/vulkan.h>)
#include <QVulkanInstance>
#include <QVulkanFunctions>
#endif
#include <QPainter>
#include <QFontDatabase>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QElapsedTimer>
#include <QThread>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>

[[noreturn]] static void fail(const char *message)
{ std::fprintf(stderr,"scenegraph-run-gate: %s\n",message); std::fflush(stderr); std::exit(2); }
static void output(const QJsonObject &o)
{ std::puts(QJsonDocument(o).toJson(QJsonDocument::Compact).constData()); std::fflush(stdout); }
static QJsonArray point(QPointF p) { return {p.x(),p.y()}; }
static QJsonArray rectangle(QRect r) { return {r.x(),r.y(),r.width(),r.height()}; }
static QString loadFont(const QString &path)
{
    QFile f(path); if (!f.open(QIODevice::ReadOnly)) fail("Cannot read font");
    const auto bytes=f.readAll(); const int id=QFontDatabase::addApplicationFontFromData(bytes);
    if (id<0) fail("Cannot load font");
    const auto family=QFontDatabase::applicationFontFamilies(id).first();
    output({{"type","font"},{"path",path},{"family",family},
        {"sha256",QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())}});
    return family;
}
struct Metrics {
    int changed=0, maximum=0;
    QJsonObject json() const { return {{"changed",changed},{"maxChannelDelta",maximum}}; }
};
static Metrics compare(const QImage &a, const QImage &b, QImage *diff=nullptr)
{
    if (a.size()!=b.size() || a.isNull()) fail("Invalid capture dimensions");
    Metrics m;
    if (diff) { *diff=QImage(a.size(),QImage::Format_ARGB32_Premultiplied); diff->fill(Qt::transparent); }
    if (a.format()!=QImage::Format_ARGB32_Premultiplied || b.format()!=a.format()) fail("Invalid premultiplied format");
    for (int y=0;y<a.height();++y) {
        const auto *ap=reinterpret_cast<const QRgb *>(a.constScanLine(y));
        const auto *bp=reinterpret_cast<const QRgb *>(b.constScanLine(y));
        auto *dp=diff?reinterpret_cast<QRgb *>(diff->scanLine(y)):nullptr;
        for (int x=0;x<a.width();++x) {
            const auto p=ap[x],q=bp[x];
            if (p==q) continue;
            ++m.changed; int delta=0;
            for (int c=0;c<4;++c) delta=qMax(delta,qAbs(int((p>>(8*c))&255)-int((q>>(8*c))&255)));
            m.maximum=qMax(m.maximum,delta);
            if (dp) dp[x]=qRgba(255,0,255,255);
        }
    }
    return m;
}
struct Counters {
    std::atomic<int> layouts{0}, rasters{0}, nodes{0}, textures{0}, liveNodes{0}, liveTextures{0}, roots{0};
    QJsonObject json() const { return {{"layouts",layouts.load()},{"rasters",rasters.load()},
        {"nodesCreated",nodes.load()},{"texturesCreated",textures.load()},
        {"liveNodes",liveNodes.load()},{"liveTextures",liveTextures.load()},{"liveRoots",roots.load()}}; }
};
// This is the complete render-thread interface: no fonts, glyph runs or layout.
struct RunImage { QRect bounds; QImage tile,crop,full; };
struct Generation { quint64 id; qreal dpr; QSize canvas; QVector<RunImage> runs; };
struct PaintSpec { QGlyphRun glyphs; QPointF origin; QColor color; };
struct Prepared {
    std::shared_ptr<Generation> images;
    QVector<PaintSpec> paint; // GUI-only diagnostic reference, never passed to Surface.
    QJsonArray log;
    int glyphs=0, croppedInk=0, borderInk=0, preparationChanged=0;
    QPointF phase; bool historical=false;
};
struct Fixture { QString text; qreal width=400; int alignment=Qt::AlignLeft, kind=0; };
// Fixed across fixtures: paragraph widths vary, native swapchain geometry does not.
static QSize viewport(qreal dpr)
{
    const qreal scale=qEnvironmentVariable("QT_SCALE_FACTOR","1").toDouble();
    int grid=1;
    while (grid<=120 && (qAbs(grid*dpr-qRound(grid*dpr))>1e-8
        || qAbs(grid*scale-qRound(grid*scale))>1e-8)) ++grid;
    if (grid>120) fail("No common viewport grid for this DPR/platform scale");
    return {qCeil(600./grid)*grid,qCeil(384./grid)*grid};
}
static Prepared prepare(const Fixture &f, qreal dpr, QPointF phase, bool historical,
                        const QStringList &families, const QStringList &files, Counters &counts)
{
    ++counts.layouts;
    FormattedText text;
    QFont font(families[0],40); font.setStyle(QFont::StyleOblique);
    QFont mono(families[1],40); mono.setStyle(QFont::StyleNormal); mono.setWeight(QFont::Normal);
    text.setFont(font); text.setCodeFont(mono); text.setText(f.text); text.setLayoutWidth(f.width);
    text.setHorizontalAlignment(f.alignment); text.setWrapEnabled(true);
    text.setCodeHorizontalPadding(7); text.setCodeVerticalPadding(3);
    if (f.kind==1 || f.kind==2)
        text.setFormatRanges({QVariantMap{{"start",0},{"length",f.kind==1?2:4},{"flags",QMarkdownPrivate::Code}}});
    if (f.kind==1) text.setCodeSpans({QVariantMap{{"start",0},{"length",2}}});
    if (f.kind==2) text.setCodeSpans({QVariantMap{{"start",0},{"length",2}},QVariantMap{{"start",2},{"length",2}}});
    if (f.kind==3) text.setForegroundRanges({QVariantMap{{"start",1},{"length",2},{"color",QColor(20,80,190,180)}}});
    text.diagnosticPrepare();
    if ((f.kind==1 || f.kind==2) && text.codeRectangles().size()!=f.kind) fail("Unpadded fixture");
    const auto runs=text.diagnosticRuns();
    if (runs.isEmpty() || runs.first().glyphs.positions().isEmpty()) fail("Empty prepared runs");
    QPointF origin(96,32);
    if (historical) origin={24.25,24.25};
    else {
        const auto anchor=(origin+runs.first().origin+runs.first().glyphs.positions().first())*dpr;
        origin+=(phase-QPointF(anchor.x()-std::floor(anchor.x()),anchor.y()-std::floor(anchor.y())))/dpr;
    }
    static quint64 serial=0;
    if (QThread::currentThread()!=QGuiApplication::instance()->thread()) fail("Preparation left GUI thread");
    Prepared result; result.phase=phase; result.historical=historical;
    result.images=std::make_shared<Generation>();
    auto &g=*result.images; g.id=++serial; g.dpr=dpr;
    const auto logicalViewport=viewport(dpr);
    if (text.logicalHeight()+100>logicalViewport.height()) fail("Fixture exceeds fixed viewport");
    g.canvas={qRound(logicalViewport.width()*dpr),qRound(logicalViewport.height()*dpr)};
    for (const auto &run:runs) {
        auto all=run.glyphs; all.setUnderline(false); all.setStrikeOut(false); all.setOverline(false);
        const auto indexes=all.glyphIndexes(); const auto positions=all.positions();
        if (indexes.isEmpty() || indexes.size()!=positions.size()) fail("Invalid glyph list");
        const QRawFont supplied(files[all.rawFont().familyName()==families[1]?1:0],all.rawFont().pixelSize());
        for (const char *table:{"head","name","cmap","glyf"})
            if (all.rawFont().fontTable(table)!=supplied.fontTable(table)) fail("Prepared font differs from supplied file");
        QRectF ink; QJsonArray glyphLog;
        for (qsizetype i=0;i<indexes.size();++i) {
            const auto rect=all.rawFont().boundingRect(indexes[i]).translated(origin+run.origin+positions[i]);
            if (!rect.isEmpty()) ink=ink.isEmpty()?rect:ink.united(rect);
            glyphLog.append(QJsonObject{{"index",int(indexes[i])},{"position",point(positions[i])}});
        }
        RunImage image;
        image.bounds=QRectF(ink.topLeft()*dpr,ink.size()*dpr).toAlignedRect().adjusted(-4,-4,4,4);
        image.full=QImage(g.canvas,QImage::Format_ARGB32_Premultiplied);
        image.full.setDevicePixelRatio(dpr); image.full.fill(Qt::transparent);
        { QPainter p(&image.full); p.setPen(run.color); p.drawGlyphRun(origin+run.origin,all); ++counts.rasters; }
        image.tile=QImage(image.bounds.size(),image.full.format());
        image.tile.setDevicePixelRatio(dpr); image.tile.fill(Qt::transparent);
        { QPainter p(&image.tile); p.setPen(run.color); p.translate(-QPointF(image.bounds.topLeft())/dpr);
          p.drawGlyphRun(origin+run.origin,all); ++counts.rasters; }
        for (int y=0;y<image.full.height();++y) {
            const auto *pixels=reinterpret_cast<const QRgb *>(image.full.constScanLine(y));
            for (int x=0;x<image.full.width();++x) if (qAlpha(pixels[x]))
                result.croppedInk+=!image.bounds.contains(x,y) || x==0 || y==0 || x==image.full.width()-1 || y==image.full.height()-1;
        }
        image.crop=image.full.copy(image.bounds);
        for (int y=0;y<image.tile.height();++y) for (int x=0;x<image.tile.width();++x)
            if ((x==0 || y==0 || x==image.tile.width()-1 || y==image.tile.height()-1) && qAlpha(image.tile.pixel(x,y))) ++result.borderInk;
        image.full.setDevicePixelRatio(1); image.tile.setDevicePixelRatio(1); image.crop.setDevicePixelRatio(1);
        result.preparationChanged+=compare(image.tile,image.crop).changed;
        result.log.append(QJsonObject{{"ordinal",g.runs.size()},{"origin",point(run.origin)},
            {"drawOrigin",point(origin+run.origin)},{"bounds",rectangle(image.bounds)},
            {"family",all.rawFont().familyName()},{"style",all.rawFont().styleName()},
            {"pixelSize",all.rawFont().pixelSize()},{"color",run.color.name(QColor::HexArgb)},{"glyphs",glyphLog}});
        result.glyphs+=indexes.size(); result.paint.append({all,origin+run.origin,run.color}); g.runs.append(image);
    }
    return result;
}
class Surface final : public QQuickItem {
public:
    Surface(QQuickItem *parent, int path, std::shared_ptr<Counters> counts)
        : QQuickItem(parent), path(path), counts(std::move(counts)) { setFlag(ItemHasContents); }
    void setGeneration(std::shared_ptr<const Generation> next) { generation=std::move(next); update(); }
    QJsonArray identities; // Written during synchronization while GUI is blocked.
protected:
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override {
        struct Root final : QSGNode {
            std::shared_ptr<Counters> counts; quint64 generation=0; int size=0;
            explicit Root(std::shared_ptr<Counters> c):counts(std::move(c)) { ++counts->roots; }
            ~Root() override {
                while (auto *child=firstChild()) { removeChildNode(child); delete child; }
                counts->liveNodes-=size; counts->liveTextures-=size; --counts->roots;
            }
        };
        auto *root=static_cast<Root *>(old);
        if (!generation || !window()) return root;
        if (window()->rendererInterface()->graphicsApi()!=QSGRendererInterface::Software
            && QThread::currentThread()==QGuiApplication::instance()->thread())
            fail("Native nodes are not on a render thread");
        if (root && root->generation==generation->id) return root;
        delete root; root=new Root(counts); root->generation=generation->id;
        identities={};
        for (qsizetype ordinal=0;ordinal<generation->runs.size();++ordinal) {
            const auto &run=generation->runs[ordinal];
            const auto &image=path==0?run.tile:path==1?run.crop:run.full;
            const QRect bounds=path==2?QRect(QPoint(),generation->canvas):run.bounds;
            auto *texture=window()->createTextureFromImage(image,QQuickWindow::TextureHasAlphaChannel);
            if (!texture) fail("Texture creation failed");
            texture->setHorizontalWrapMode(QSGTexture::ClampToEdge); texture->setVerticalWrapMode(QSGTexture::ClampToEdge);
            auto *node=window()->createImageNode();
            if (!node) fail("Image node creation failed");
            node->setTexture(texture); node->setOwnsTexture(true); node->setFiltering(QSGTexture::Linear);
            node->setMipmapFiltering(QSGTexture::None);
            node->setRect(QRectF(QPointF(bounds.topLeft())/generation->dpr,QSizeF(bounds.size())/generation->dpr));
            node->setSourceRect(QRectF(QPointF(),QSizeF(image.size())));
            root->appendChildNode(node); ++root->size;
            ++counts->nodes; ++counts->textures; ++counts->liveNodes; ++counts->liveTextures;
            identities.append(QJsonObject{{"ordinal",ordinal},{"node",QString::number(quintptr(node),16)},
                {"texture",QString::number(quintptr(texture),16)},{"pixels",QJsonArray{image.width(),image.height()}},
                {"atlas",texture->isAtlasTexture()},{"bounds",rectangle(bounds)}});
            if (texture->isAtlasTexture()) fail("Unexpected texture atlas");
        }
        return root;
    }
private:
    int path; std::shared_ptr<Counters> counts; std::shared_ptr<const Generation> generation;
};
class View {
public:
    std::shared_ptr<Counters> counts;
    // Callback state outlives window destruction and its render-thread shutdown.
    std::atomic<int> synchronized{0}, rendered{0};
    QJsonObject device;
    QMetaObject::Connection beforeConnection, syncConnection, afterConnection;
    QQuickWindow window;
    QQuickItem *clip, *motion;
    Surface *surfaces[3];
    View(std::shared_ptr<Counters> c, qreal requestedDpr, QString requestedApi):counts(std::move(c)) {
        const auto extent=viewport(requestedDpr>0?requestedDpr:window.effectiveDevicePixelRatio());
        window.resize(extent); window.setMinimumSize(extent); window.setMaximumSize(extent); window.setColor(Qt::white);
        clip=new QQuickItem(window.contentItem()); motion=new QQuickItem(clip);
        for (int i=0;i<3;++i) { surfaces[i]=new Surface(motion,i,counts); surfaces[i]->setVisible(false); }
        beforeConnection=QObject::connect(&window,&QQuickWindow::beforeRendering,&window,[this] {
            if (!device.isEmpty()) return;
            auto *renderer=window.rendererInterface();
            if (renderer->graphicsApi()==QSGRendererInterface::Software) device={{"name","Qt Quick software"},{"hardware",false}};
            else if (renderer->graphicsApi()==QSGRendererInterface::OpenGL) {
                auto *context=static_cast<QOpenGLContext *>(renderer->getResource(&window,QSGRendererInterface::OpenGLContextResource));
                if (context && QOpenGLContext::currentContext()==context) {
                    auto *f=context->functions();
                    const auto name=QString::fromLatin1(reinterpret_cast<const char *>(f->glGetString(GL_RENDERER)));
                    const auto version=QString::fromLatin1(reinterpret_cast<const char *>(f->glGetString(GL_VERSION)));
                    const auto vendor=QString::fromLatin1(reinterpret_cast<const char *>(f->glGetString(GL_VENDOR)));
                    const auto lower=name.toLower();
                    const bool hardware=!name.isEmpty() && !lower.contains("llvmpipe") && !lower.contains("softpipe") && !lower.contains("swiftshader") && !lower.contains("software");
                    device={{"name",name},{"version",version},{"vendor",vendor},{"hardware",hardware}};
                }
            }
#if QT_CONFIG(vulkan) && __has_include(<vulkan/vulkan.h>)
            else if (renderer->graphicsApi()==QSGRendererInterface::Vulkan) {
                auto *instance=static_cast<QVulkanInstance *>(renderer->getResource(&window,QSGRendererInterface::VulkanInstanceResource));
                auto *physical=static_cast<VkPhysicalDevice *>(renderer->getResource(&window,QSGRendererInterface::PhysicalDeviceResource));
                if (instance && physical) {
                    VkPhysicalDeviceProperties properties{};
                    instance->functions()->vkGetPhysicalDeviceProperties(*physical,&properties);
                    const auto name=QString::fromUtf8(properties.deviceName);
                    const bool hardware=properties.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || properties.deviceType==VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
                    device={{"name",name},{"hardware",hardware},{"vendorId",int(properties.vendorID)},
                        {"deviceId",int(properties.deviceID)},{"driverVersion",qint64(properties.driverVersion)},
                        {"apiVersion",qint64(properties.apiVersion)},{"deviceType",int(properties.deviceType)}};
                }
            }
#endif
        },Qt::DirectConnection);
        syncConnection=QObject::connect(&window,&QQuickWindow::afterSynchronizing,&window,[this]{++synchronized;},Qt::DirectConnection);
        afterConnection=QObject::connect(&window,&QQuickWindow::afterRendering,&window,[this]{rendered.store(synchronized.load());},Qt::DirectConnection);
        window.show();
        QElapsedTimer timer; timer.start();
        while (!window.isExposed() && timer.elapsed()<10000) { QCoreApplication::processEvents(); QThread::msleep(1); }
        if (!window.isExposed()) fail("Window not exposed");
        render(0);
        const auto api=window.rendererInterface()->graphicsApi();
        const QString observed=api==QSGRendererInterface::Software?"software":api==QSGRendererInterface::OpenGL?"opengl":api==QSGRendererInterface::Vulkan?"vulkan":"other";
        const qreal dpr=window.effectiveDevicePixelRatio();
        output({{"type","window"},{"requestedApi",requestedApi},{"api",observed},
            {"requestedDpr",requestedDpr},{"dpr",dpr},{"platform",QGuiApplication::platformName()},{"device",device}});
        if (observed!=requestedApi || (requestedDpr>0 && qAbs(dpr-requestedDpr)>0.000001)) fail("Backend or actual DPR mismatch");
        if (observed!="software" && !device["hardware"].toBool()) fail("Native hardware device not confirmed");
    }
    ~View() {
        // Destroy the tree while counters and the frame callback target are alive.
        QObject::disconnect(beforeConnection); QObject::disconnect(syncConnection); QObject::disconnect(afterConnection);
        // QQuickWindow destruction owns graph shutdown; do not hide a live
        // Vulkan swapchain immediately before destroying its window.
    }
    void setGeneration(std::shared_ptr<const Generation> g) {
        if (g->canvas!=QSize(qRound(window.width()*g->dpr),qRound(window.height()*g->dpr)))
            fail("Generation differs from fixed viewport");
        motion->setSize(QSizeF(window.size()));
        for (auto *surface:surfaces) { surface->setSize(QSizeF(window.size())); surface->setGeneration(g); }
        placement({},false);
    }
    void placement(QPointF physical, bool clipped) {
        const auto dpr=window.effectiveDevicePixelRatio();
        const QPointF top=clipped?QPointF(64,32):QPointF();
        clip->setPosition(top); clip->setSize(clipped?QSizeF(96,96):QSizeF(window.size())); clip->setClip(clipped);
        motion->setPosition(physical/dpr-top);
    }
    void render(int path) {
        for (int i=0;i<3;++i) surfaces[i]->setVisible(i==path);
        const int before=synchronized.load(); window.update();
        QElapsedTimer timer; timer.start();
        while (rendered.load()<=before && timer.elapsed()<10000) { QCoreApplication::processEvents(); QThread::msleep(1); }
        if (rendered.load()<=before) fail("Frame timeout");
    }
    QImage capture(int path) {
        for (int i=0;i<3;++i) surfaces[i]->setVisible(i==path);
        const int before=synchronized.load();
        QElapsedTimer timer; timer.start();
        for (;;) {
            // Public grabWindow polishes, synchronizes and renders on the render
            // thread. Do not depend on a compositor's next presentation callback.
            QCoreApplication::processEvents();
            const auto image=window.grabWindow().convertToFormat(QImage::Format_ARGB32_Premultiplied);
            const qreal dpr=window.effectiveDevicePixelRatio();
            const QSize expected(qRound(window.width()*dpr),qRound(window.height()*dpr));
            if (!image.isNull() && image.size()==expected && rendered.load()>before) return image;
            // Wayland configure/resize acknowledgement can lag the logical size.
            // No pixel comparison has been attempted for these invalid extents.
            output({{"type","capturePending"},{"path",path},
                {"expected",QJsonArray{expected.width(),expected.height()}},
                {"observed",QJsonArray{image.width(),image.height()}},
                {"syncBefore",before},{"renderedSync",rendered.load()}});
            if (timer.elapsed()>10000) fail("Capture extent/synchronization did not settle");
            QThread::msleep(1);
        }
    }
    QJsonArray identities() const { return {surfaces[0]->identities,surfaces[1]->identities,surfaces[2]->identities}; }
};
static QImage fresh(const Prepared &p, QSize size, QColor background, QPointF physical, bool clipped)
{
    QImage image(size,QImage::Format_ARGB32_Premultiplied); image.setDevicePixelRatio(p.images->dpr); image.fill(background);
    QPainter painter(&image);
    if (clipped) painter.setClipRect(QRectF(64,32,96,96));
    painter.translate(physical/p.images->dpr);
    for (const auto &run:p.paint) { painter.setPen(run.color); painter.drawGlyphRun(run.origin,run.glyphs); }
    return image;
}
int main(int argc,char **argv)
{
    QGuiApplication app(argc,argv); const auto args=app.arguments();
    // Independent positive/negative control for the exact premultiplied detector.
    QImage sanity(7,3,QImage::Format_ARGB32_Premultiplied);
    for (int y=0;y<3;++y) for (int x=0;x<7;++x) sanity.setPixel(x,y,qRgba(x+y,2*x+y,x,40));
    QImage altered=sanity; altered.setPixel(1,0,0); altered.setPixel(6,2,qRgba(20,20,20,40));
    QImage sanityDiff; const auto sanityMetrics=compare(sanity,altered,&sanityDiff);
    if (compare(sanity,sanity).changed || sanityMetrics.changed!=2 || sanityMetrics.maximum!=40
        || sanityDiff.pixel(1,0)!=qRgba(255,0,255,255)) fail("Exact detector self-test failed");
    output({{"type","detector"},{"knownChanged",2},{"knownMaxChannelDelta",40},{"passed",true}});
    if (args.size()!=7) fail("Usage: scenegraph-run-gate fonts stage api dpr forward|reverse captures");
    const QStringList files{args[1]+"/DejaVuSans-Oblique.ttf",args[1]+"/DejaVuSansMono.ttf"};
    const QStringList families{loadFont(files[0]),loadFont(files[1])};
    const auto stage=args[2],api=args[3]; const qreal dpr=args[4].toDouble(); const bool reverse=args[5]=="reverse";
    if (stage!="calibrate" && dpr!=1 && dpr!=1.25 && dpr!=1.5 && dpr!=2) fail("Invalid DPR");
    if (args[5]!="forward" && !reverse) fail("Invalid order");
    QDir().mkpath(args[6]);
    output({{"type","identity"},{"qt",qVersion()},{"compiler",__VERSION__},{"stage",stage},
        {"order",args[5]},{"captureFormat","ARGB32_Premultiplied"},{"filtering","linear"}});
    auto counts=std::make_shared<Counters>();
    int cases=0,failures=0; QSet<QString> saved;
    const QList<int> order=reverse?QList<int>{2,1,0}:QList<int>{0,1,2};
    auto check=[&](View &view,const Prepared &p,QString id,QColor background,QPointF offset,bool clipped,
                   const QJsonObject &warm=QJsonObject(),const QJsonArray &retained=QJsonArray()) {
        view.window.setColor(background); view.placement(offset,clipped);
        QImage images[3]; for (int path:order) images[path]=view.capture(path);
        const auto tileCrop=compare(images[0],images[1]),tileFull=compare(images[0],images[2]),cropFull=compare(images[1],images[2]);
        const auto painter=compare(images[0],fresh(p,images[0].size(),background,offset,clipped));
        const auto state=counts->json(); const auto identities=view.identities();
        const auto observedOffset=view.motion->mapToScene(QPointF())*dpr;
        const bool positionMatches=qAbs(observedOffset.x()-offset.x())<1e-8 && qAbs(observedOffset.y()-offset.y())<1e-8;
        bool stable=warm.isEmpty() || (state==warm && identities==retained);
        int leaks=0;
        if (clipped) {
            const QRect clip=QRectF(QPointF(64,32)*dpr,QSizeF(96,96)*dpr).toAlignedRect();
            for (int y=0;y<images[0].height();++y) {
                const QRgb *rows[3];
                for (int i=0;i<3;++i) rows[i]=reinterpret_cast<const QRgb *>(images[i].constScanLine(y));
                for (int x=0;x<images[0].width();++x)
                    if (!clip.contains(x,y)) for (auto *row:rows) leaks+=row[x]!=background.rgba();
            }
        }
        bool countsOk=true;
        for (const auto &path:identities) {
            const auto nodes=path.toArray(); countsOk &= nodes.size()==p.images->runs.size();
            for (qsizetype i=0;i<nodes.size();++i) countsOk &= nodes[i].toObject()["ordinal"].toInt()==i;
        }
        const bool failed=tileCrop.changed || tileFull.changed || cropFull.changed || p.preparationChanged || p.croppedInk || leaks || !stable || !countsOk || !positionMatches;
        ++cases; failures+=failed;
        output({{"type","case"},{"id",id},{"background",background.name()},{"dpr",dpr},{"offset",point(offset)},
            {"observedOffset",point(observedOffset)},{"positionMatches",positionMatches},{"clipped",clipped},{"phase",point(p.phase)},{"historical",p.historical},{"runs",p.log},{"runCount",p.images->runs.size()},{"glyphCount",p.glyphs},
            {"generation",qint64(p.images->id)},{"canvas",QJsonArray{p.images->canvas.width(),p.images->canvas.height()}},
            {"window",QJsonArray{view.window.width(),view.window.height()}},{"capture",QJsonArray{images[0].width(),images[0].height()}},{"tileCrop",tileCrop.json()},{"tileFull",tileFull.json()},
            {"cropFull",cropFull.json()},{"freshPainterDiagnostic",painter.json()},
            {"preparationChanged",p.preparationChanged},{"croppedInk",p.croppedInk},{"borderInk",p.borderInk},{"clipLeaks",leaks},
            {"stableResources",stable},{"orderedCounts",countsOk},{"counters",state},{"resources",identities},{"failed",failed}});
        const auto key=background.name()+(failed?"-failure":"-sample");
        if (!saved.contains(key)) {
            saved.insert(key); const auto prefix=args[6]+"/"+id+"-"+background.name().mid(1);
            images[2].save(prefix+"-reference.png"); images[0].save(prefix+"-candidate.png");
            QImage difference; compare(images[2],images[0],&difference); difference.save(prefix+"-difference.png");
        }
        return images[0];
    };
    if (stage=="calibrate") {
        View view(counts,0,api);
    } else if (stage=="static" || stage=="probe") {
        View view(counts,dpr,api);
        auto run=[&](Fixture f,QPointF phase,QString id,bool historical=false) {
            const auto p=prepare(f,dpr,phase,historical,families,files,*counts); view.setGeneration(p.images);
            for (QColor background:{QColor(Qt::white),QColor("#202020")}) check(view,p,id,background,{},false);
        };
        if (stage=="probe") run({"fX"},{},"probe");
        else {
            if (dpr==1 || dpr==1.25) run({"fX"},{},"historical",true);
            int index=0;
            for (QString fixture:{"f","X","fX","fXfX"}) {
                for (int x=0;x<4;++x) for (int y=0;y<4;++y)
                    run({fixture},{x*.25,y*.25},QString("basic-%1-%2-%3").arg(index).arg(x).arg(y));
                ++index;
            }
            for (int kind=0;kind<5;++kind) for (int alignment:{Qt::AlignLeft,Qt::AlignHCenter,Qt::AlignRight})
                for (int x=0;x<4;++x) for (int y=0;y<4;++y)
                    run({kind==3?"fX fX fX":"fXfX",kind==3?95.:kind==4?1.:400.,alignment,kind<3?kind+1:0},
                        {x*.25,y*.25},QString("extended-%1-%2-%3-%4").arg(kind).arg(alignment).arg(x).arg(y));
        }
    } else if (stage=="scroll" || stage=="lifecycle") {
        const QList<Fixture> fixtures=stage=="lifecycle"?QList<Fixture>{{"fXfX",400,Qt::AlignLeft,3}}:
            QList<Fixture>{{"fXfX"},{"fXfX",400,Qt::AlignLeft,1},{"fXfX",400,Qt::AlignLeft,2},
                {"fXfX",400,Qt::AlignLeft,3},{"fX fX fX",95,Qt::AlignHCenter},{"fXfX",1,Qt::AlignRight}};
        int index=0;
        for (const auto &fixture:fixtures) {
            const auto p=prepare(fixture,dpr,{},false,families,files,*counts);
            const int cycles=stage=="lifecycle"?3:1;
            QMap<QString,QImage> originals;
            for (int cycle=0;cycle<cycles;++cycle) {
                {
                    View view(counts,dpr,api); view.setGeneration(p.images);
                    for (QColor background:{QColor(Qt::white),QColor("#202020")}) for (bool clipped:{false,true}) {
                        const auto initial=check(view,p,QString("warm-%1-%2").arg(index).arg(cycle),background,{},clipped);
                        const auto warm=counts->json(); const auto retained=view.identities();
                        if (stage=="lifecycle") {
                            const auto key=background.name()+QString::number(clipped);
                            const bool equal=!originals.contains(key) || !compare(originals[key],initial).changed;
                            if (!originals.contains(key)) originals[key]=initial;
                            output({{"type","recreated"},{"cycle",cycle},{"background",background.name()},
                                {"clipped",clipped},{"equal",equal},{"counters",warm}});
                            failures+=!equal;
                        } else {
                            QList<QPointF> trace;
                            for (int x=0;x<4;++x) for (int y=0;y<4;++y) trace.append({x*.25,y*.25});
                            trace.append({1,1}); trace.append({-1,-1}); trace.append({0,0});
                            for (int repeat=0;repeat<2;++repeat) for (qsizetype frame=0;frame<trace.size();++frame) {
                                const auto image=check(view,p,QString("scroll-%1-%2-%3").arg(index).arg(repeat).arg(frame),
                                    background,trace[frame],clipped,warm,retained);
                                if (trace[frame]==QPointF()) {
                                    const bool equal=!compare(initial,image).changed; failures+=!equal;
                                    output({{"type","returned"},{"equal",equal}});
                                }
                            }
                        }
                    }
                }
                const bool balanced=counts->liveNodes==0 && counts->liveTextures==0 && counts->roots==0;
                output({{"type","teardown"},{"cycle",cycle},{"balanced",balanced},{"counters",counts->json()}}); failures+=!balanced;
            }
            ++index;
        }
    } else fail("Invalid stage");
    const bool balanced=counts->liveNodes==0 && counts->liveTextures==0 && counts->roots==0;
    failures+=!balanced;
    output({{"type","summary"},{"cases",cases},{"failures",failures},{"balanced",balanced},{"counters",counts->json()}});
    return failures?1:0;
}
