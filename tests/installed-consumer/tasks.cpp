#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

static QList<QQuickItem *> indicators(QQuickItem *parent)
{
    QList<QQuickItem *> result;
    for (auto *child : parent->childItems()) {
        if (child->objectName() == "taskIndicator" && child->isVisible()) result.append(child);
        result.append(indicators(child));
    }
    return result;
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import QtQuick\nimport QMarkdown 1.3\nMarkdownView { width: 180; markdown: '- [x] Done\\n- [ ] Pending\\n  - [X] Nested' }", {});
    if (!component.isReady()) { qWarning() << component.errorString(); return 1; }
    std::unique_ptr<QObject> object(component.create());
    auto *view = qobject_cast<QQuickItem *>(object.get());
    if (!view || view->property("taskListsEnabled").toBool()) return 2;
    QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300, 400); window.show();
    if (!QTest::qWaitFor([&] { return view->property("contentHeight").toDouble() > 0; })) return 3;
    if (!indicators(view).isEmpty()) return 4;
    const auto source = view->property("markdown");
    if (!view->setProperty("taskListsEnabled", true)) return 5;
    if (!QTest::qWaitFor([&] { return indicators(view).size() == 3; })) return 6;
    const auto items = indicators(view);
    if (!items[0]->property("checked").toBool() || items[1]->property("checked").toBool()
        || !items[2]->property("checked").toBool()) return 7;
    view->setProperty("taskListsEnabled", false);
    if (!QTest::qWaitFor([&] { return indicators(view).isEmpty(); })) return 8;
    if (view->property("markdown") != source) return 9;
    QQmlComponent custom(&engine);
    custom.setData("import QtQuick\nimport QMarkdown 1.10\nMarkdownView { width: 180; taskListsEnabled: true; markdown: '- [x] Done'; taskIndicator: Component { Item { required property bool checked; required property MarkdownStyle markdownStyle; implicitWidth: 35; implicitHeight: 45 } } }", {});
    if (!custom.isReady()) { qWarning() << custom.errorString(); return 10; }
    std::unique_ptr<QObject> customObject(custom.create());
    auto *customView = qobject_cast<QQuickItem *>(customObject.get());
    if (!customView) return 11;
    customView->setParentItem(window.contentItem());
    if (!QTest::qWaitFor([&] { return indicators(customView).size() == 1; })) return 12;
    auto *indicator = indicators(customView)[0];
    return QTest::qWaitFor([&] { return indicator->width() == 35 && indicator->height() == 45; }) ? 0 : 13;
}
