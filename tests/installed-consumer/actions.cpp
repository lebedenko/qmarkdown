#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

static QQuickItem *action(QQuickItem *parent)
{
    for (auto *child : parent->childItems()) {
        if (child->objectName() == "action") return child;
        if (auto *found = action(child)) return found;
    }
    return nullptr;
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(R"(import QtQuick
import QMarkdown 1.2
MarkdownView {
    width: 180
    markdown: "> ```info\n>   λ 😀  \n>\n> ```"
    codeBlockActions: Item {
        required property string codeText
        required property var requestCopy
        objectName: "action"
        implicitHeight: 24
        function activate() { requestCopy() }
    }
})", {});
    if (!component.isReady()) { qWarning() << component.errorString(); return 1; }
    std::unique_ptr<QObject> object(component.create());
    auto *view = qobject_cast<QQuickItem *>(object.get());
    if (!view) return 2;
    QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300, 400); window.show();
    if (!QTest::qWaitFor([&] { return action(view) != nullptr; })) return 3;
    auto *header = action(view);
    const auto expected = QString::fromUtf8("  λ 😀  \n\n");
    if (header->property("codeText").toString() != expected || header->height() != 24
        || header->width() <= 0) return 4;
    QSignalSpy requests(view, SIGNAL(codeBlockCopyRequested(QString)));
    if (!QMetaObject::invokeMethod(header, "activate") || requests.size() != 1
        || requests[0][0].toString() != expected) return 5;
    view->setProperty("codeBlockActions", QVariant::fromValue<QQmlComponent *>(nullptr));
    if (!QTest::qWaitFor([&] { return action(view) == nullptr; })) return 6;
    return view->property("contentHeight").toDouble() > 0 ? 0 : 7;
}
