#include <QGuiApplication>
#include <QQuickView>
#include <QQuickItem>
#include <QTest>
#include <QDebug>
int main(int argc, char **argv) {
 QGuiApplication app(argc, argv);
 QQuickView view;
 view.setSource(QUrl::fromLocalFile("/tmp/mdv-lines-probe.qml"));
 if (view.status() != QQuickView::Ready) return 1;
 view.show(); QTest::qWait(300);
 for (bool align: {false,true}) for (double offset: {0.,0.25,0.5,0.75,1.}) {
   view.rootObject()->setProperty("aligned", align);
   view.rootObject()->setProperty("shift", offset); QTest::qWait(80);
   auto img = view.grabWindow();
   auto name = QString("/tmp/mdv-lines-%1-%2-%3.png").arg(view.devicePixelRatio()).arg(align).arg(offset);
   img.save(name); qInfo() << name << view.rootObject()->property("shift");
 }
}
