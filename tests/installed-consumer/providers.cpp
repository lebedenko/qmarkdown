#include <QGuiApplication>
#include <QColor>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QElapsedTimer>
#include <QtQml/QQmlExtensionPlugin>
#include <functional>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif
int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    QQmlEngine engine; QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
import QMarkdown 1.17
MarkdownView {
    id: view
    width: 300
    property int calls: 0
    property var pending: null
    syntaxHighlightingEnabled: true
    syntaxHighlightingProvider: SyntaxHighlightingProvider {
        id: provider
        onHighlightRequested: request => {
            view.calls++
            view.pending = request
            delayed.restart()
        }
    }
    Timer {
        id: delayed; interval: 1
        onTriggered: {
            if (!view.pending.cancelled)
                view.pending.complete([{start:0,length:5,role:SyntaxHighlightingProvider.Keyword}])
        }
    }
    syntaxTheme: SyntaxTheme { keywordColor: "magenta" }
    selectionEnabled: true
    markdown: "```HostOnly\nhello 😀\n```"
    function invalidate() { provider.invalidate() }
}
)", {});
    std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
    if (!view) { qCritical() << component.errorString(); return 1; }
    std::function<bool(QQuickItem *)> colored = [&](auto *item) {
        if (!item->property("foregroundRanges").toList().isEmpty()) return true;
        for (auto *child : item->childItems()) if (colored(child)) return true;
        return false;
    };
    auto wait = [&](auto condition) {
        QElapsedTimer timer; timer.start();
        while (!condition() && timer.elapsed() < 5000) QCoreApplication::processEvents();
        return condition();
    };
    if (!wait([&] { return colored(view.get()); })) return 2;
    if (view->property("calls").toInt() != 1) return 3;
    view->property("syntaxTheme").value<QObject *>()->setProperty("keywordColor", QColor(Qt::cyan));
    if (view->property("calls").toInt() != 1) return 4;
    QMetaObject::invokeMethod(view.get(), "selectAll");
    if (!view->property("selectedText").toString().contains(QString::fromUtf8("hello 😀\n"))) return 5;
    view->setProperty("searchActive", true); view->setProperty("searchQuery", "hello");
    if (view->property("searchResultCount").toInt() != 1) return 6;
    QMetaObject::invokeMethod(view.get(), "invalidate");
    if (!wait([&] { return view->property("calls").toInt() == 2 && colored(view.get()); })) return 7;
    return 0;
}
