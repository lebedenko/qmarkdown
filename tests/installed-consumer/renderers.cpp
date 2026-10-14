#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSignalSpy>
#include <QtQml/QQmlExtensionPlugin>
#include <QtTest/QTest>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif
static QQuickItem *renderer(QQuickItem *root)
{
    for (auto *child : root->childItems()) {
        if (child->objectName() == "renderer" && child->isVisible()) return child;
        if (auto *item = renderer(child)) return item;
    }
    return nullptr;
}
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv); QQmlEngine engine; QQmlComponent component(&engine);
    component.setData(R"QML(
pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown 1.7
MarkdownView {
    width: 200
    markdown: "~~~ preview &amp;\n  source\n~~~"
    fencedBlockRenderers: ({ preview: nativePreview })
    Component {
        id: nativePreview
        Item {
            objectName: "renderer"
            required property string codeText
            required property string infoString
            required property string language
            required property var markdownStyle
            implicitHeight: width / 2
        }
    }
}
)QML", {});
    std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
    if (!view) { qWarning() << component.errorString(); return 1; }
    auto *item = renderer(view.get());
    if (!item || item->property("codeText").toString() != "  source\n"
        || item->property("infoString").toString() != "preview &"
        || item->property("language").toString() != "preview"
        || item->property("markdownStyle").value<QObject *>() != view->property("style").value<QObject *>()) return 2;
    view->setWidth(100);
    if (item->width() > 100 || item->height() != item->width() / 2) return 3;
    view->setProperty("selectionEnabled", true);
    if (renderer(view.get())) return 4;
    QMetaObject::invokeMethod(view.get(), "selectAll");
    QSignalSpy copy(view.get(), SIGNAL(copyRequested(QString)));
    QMetaObject::invokeMethod(view.get(), "requestCopy");
    if (copy.size() != 1 || copy[0][0].toString() != "  source\n") return 5;
    view->setProperty("selectionEnabled", false);
    if (!renderer(view.get())) return 6;
    return 0;
}
