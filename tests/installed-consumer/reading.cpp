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
import QMarkdown 1.9
MarkdownView {
    width: 200
    markdown: "# Install\n\n> ## Install\n\nSearch **needle** and NEEDLE."
    searchActive: true
    searchQuery: "needle"
    searchHighlightColor: "yellow"
    currentSearchHighlightColor: "orange"
    property int requests: 0
    onNavigationRequested: function(target) { if (target.width > 0) ++requests }
    function check(): bool {
        if (headings.length !== 2 || headings[1].fragmentId !== "install-1") return false
        if (searchResultCount !== 2 || currentSearchResult !== 0) return false
        if (!searchPrevious() || currentSearchResult !== 1) return false
        if (!searchNext() || currentSearchResult !== 0) return false
        searchCaseSensitive = true
        if (searchResultCount !== 1) return false
        if (!navigateToFragment("#%69nstall-1") || !navigateToHeading(0)) return false
        if (navigateToFragment("#missing")) return false
        selectionEnabled = true
        selectAll()
        const selected = selectedText
        searchActive = false
        if (searchResultCount !== 0 || selectedText !== selected) return false
        return navigateToHeading(1)
    }
}
)", {});
    std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
    if (!view) { qWarning() << component.errorString(); return 1; }
    bool result = false;
    if (!QMetaObject::invokeMethod(view.get(), "check", Q_RETURN_ARG(bool, result)) || !result) return 2;
    QCoreApplication::processEvents();
    if (view->property("requests").toInt() == 0) return 3;
    return 0;
}
