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

static QStringList destinations(QQuickItem *parent)
{
    QStringList result;
    for (auto *child : parent->childItems()) {
        if (child->metaObject()->indexOfMethod("linkDestination(int)") >= 0) {
            for (int i = 0; ; ++i) {
                QString destination;
                QMetaObject::invokeMethod(child, "linkDestination", Q_RETURN_ARG(QString, destination), Q_ARG(int, i));
                if (destination.isEmpty()) break;
                result.append(destination);
            }
        }
        result.append(destinations(child));
    }
    return result;
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv); QQmlEngine engine; QQmlComponent component(&engine);
    component.setData("import QtQuick\nimport QMarkdown 1.17\nMarkdownView { width: 180; markdown: 'www.a.b https://a.b foo@a.b mailto:foo@a.b xmpp:foo@a.b/res' }", {});
    if (!component.isReady()) { qWarning() << component.errorString(); return 1; }
    std::unique_ptr<QObject> object(component.create()); auto *view = qobject_cast<QQuickItem *>(object.get());
    if (!view || view->property("extendedAutolinksEnabled").toBool() || view->property("strikethroughEnabled").toBool() || view->property("taskListsEnabled").toBool()) return 2;
    QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300,400); window.show();
    QCoreApplication::processEvents(); if (!destinations(view).isEmpty()) return 3;
    const auto source = view->property("markdown");
    QSignalSpy markdownChanges(view, SIGNAL(markdownChanged())); QSignalSpy changes(view, SIGNAL(extendedAutolinksEnabledChanged()));
    if (!view->setProperty("extendedAutolinksEnabled", true)) return 4;
    const QStringList expected{"http://www.a.b", "https://a.b", "mailto:foo@a.b", "mailto:foo@a.b", "xmpp:foo@a.b/res"};
    if (!QTest::qWaitFor([&] { return destinations(view) == expected; })) return 5;
    view->setProperty("extendedAutolinksEnabled", true);
    if (changes.size() != 1 || markdownChanges.size() != 0 || view->property("markdown") != source) return 6;
    view->setProperty("extendedAutolinksEnabled", false);
    if (!QTest::qWaitFor([&] { return destinations(view).isEmpty(); })) return 7;
    view->setProperty("extendedAutolinksEnabled", true); view->setProperty("markdown", "xmpp:new@a.b/res");
    if (!QTest::qWaitFor([&] { return destinations(view) == QStringList{"xmpp:new@a.b/res"}; })) return 8;
    return changes.size() == 3 && markdownChanges.size() == 1 ? 0 : 9;
}
