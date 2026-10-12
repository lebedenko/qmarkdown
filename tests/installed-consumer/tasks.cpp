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
    return view->property("markdown") == source ? 0 : 9;
}
