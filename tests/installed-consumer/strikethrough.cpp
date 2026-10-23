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

static QStringList labels(QQuickItem *parent)
{
    QStringList result;
    for (auto *child : parent->childItems()) {
        if (child->metaObject()->indexOfProperty("text") >= 0) result.append(child->property("text").toString());
        result.append(labels(child));
    }
    return result;
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv); QQmlEngine engine; QQmlComponent component(&engine);
    component.setData("import QtQuick\nimport QMarkdown 1.17\nMarkdownView { width: 180; markdown: '~~**bold** [link](strike:link)~~' }", {});
    if (!component.isReady()) { qWarning() << component.errorString(); return 1; }
    std::unique_ptr<QObject> object(component.create()); auto *view = qobject_cast<QQuickItem *>(object.get());
    if (!view || view->property("strikethroughEnabled").toBool() || view->property("taskListsEnabled").toBool()) return 2;
    QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300, 400); window.show();
    if (!QTest::qWaitFor([&] { return labels(view).contains("~~bold link~~"); })) return 3;
    const auto source = view->property("markdown");
    QSignalSpy markdownChanges(view, SIGNAL(markdownChanged())); QSignalSpy changes(view, SIGNAL(strikethroughEnabledChanged()));
    if (!view->setProperty("strikethroughEnabled", true)) return 4;
    if (!QTest::qWaitFor([&] { return labels(view).contains("bold link"); })) return 5;
    view->setProperty("strikethroughEnabled", true);
    if (changes.size() != 1 || markdownChanges.size() != 0 || view->property("markdown") != source) return 6;
    view->setProperty("strikethroughEnabled", false);
    if (!QTest::qWaitFor([&] { return labels(view).contains("~~bold link~~"); })) return 7;
    return changes.size() == 2 && markdownChanges.size() == 0 && view->property("markdown") == source ? 0 : 8;
}
