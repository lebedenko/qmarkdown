// Standalone empty-window reduction: no text, capture or motion dependencies.
#include <QGuiApplication>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QElapsedTimer>
#include <cstdio>

static QElapsedTimer elapsed;
static void event(const char *name) {
    QJsonObject value{{"stage",name},{"monotonicMs",elapsed.nsecsElapsed()/1e6}};
    puts(QJsonDocument(value).toJson(QJsonDocument::Compact).constData());fflush(stdout);
}
int main(int argc,char **argv) {
    elapsed.start();event("startup");
    QGuiApplication app(argc,argv);
    const bool release=app.arguments().contains("--release-live");
    QObject::connect(&app,&QCoreApplication::aboutToQuit,[]{event("about-to-quit");});
    {
        QQuickWindow window;
        window.resize(320,200);
        QObject::connect(&window,&QQuickWindow::sceneGraphInitialized,&window,[]{event("scene-graph-initialized");},Qt::DirectConnection);
        QObject::connect(&window,&QQuickWindow::sceneGraphInvalidated,&window,[]{event("scene-graph-invalidated");},Qt::DirectConnection);
        QObject::connect(&window,&QQuickWindow::sceneGraphAboutToStop,&window,[]{event("scene-graph-about-to-stop");},Qt::DirectConnection);
        bool ready=false;
        QObject::connect(&window,&QQuickWindow::frameSwapped,&window,[&]{
            if(ready)return;
            ready=true;
            QJsonObject value{{"stage","ready"},{"qt",qVersion()},
                {"api",int(window.rendererInterface()->graphicsApi())},
                {"platform",QGuiApplication::platformName()},{"dpr",window.devicePixelRatio()}};
            puts(QJsonDocument(value).toJson(QJsonDocument::Compact).constData());fflush(stdout);
            QTimer::singleShot(100,&window,[&]{
                event("shutdown");
                if(release) {
                    window.setPersistentSceneGraph(false);window.setPersistentGraphics(false);
                    event("live-hide");window.hide();
                    event("live-release");window.releaseResources();
                    QTimer::singleShot(100,&app,&QCoreApplication::quit);
                } else app.quit();
            });
        },Qt::QueuedConnection);
        window.show();app.exec();
        event("event-loop-returned");
        QObject::disconnect(&window,nullptr,&window,nullptr);
        event("window-destruction-begin");
    }
    event("window-destruction-end");event("exit");
}
