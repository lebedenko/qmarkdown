#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv); QQmlEngine engine; QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
import QMarkdown 1.18
MarkdownView {
    width: 240
    selectionEnabled: true
    property int sourceNotifications: 0
    property int navigationRequests: 0
    onMarkdownChanged: ++sourceNotifications
    onNavigationRequested: ++navigationRequests
    function check(): bool {
        if (streamingEnabled) return false
        appendMarkdown("# Synchronous")
        if (headings.length !== 1) return false
        markdown = ""
        streamingEnabled = true
        appendMarkdown("# Stream\n\n")
        if (markdown !== "# Stream\n\n" || headings.length !== 0) return false
        flushMarkdown()
        if (headings.length !== 1) return false
        selectAll()
        const selected = selectedText
        markdown += "growing snapshot"
        flushMarkdown()
        if (selectedText !== selected) return false
        appendMarkdown(" pending")
        markdown = "# Replacement"
        if (headings[0].title !== "Replacement") return false
        appendMarkdown(" complete")
        streamingEnabled = false
        if (headings[0].title !== "Replacement complete") return false
        const notifications = sourceNotifications
        appendMarkdown("")
        markdown = markdown
        return sourceNotifications === notifications && navigationRequests === 0
    }
}
)", {});
    std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
    if (!view) { qWarning() << component.errorString(); return 1; }
    bool result = false;
    if (!QMetaObject::invokeMethod(view.get(), "check", Q_RETURN_ARG(bool, result)) || !result) return 2;
    QCoreApplication::processEvents();
    return view->property("navigationRequests").toInt() == 0 ? 0 : 3;
}
