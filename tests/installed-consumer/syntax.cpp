#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QElapsedTimer>
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQmlEngine engine; QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
import QMarkdown 1.11
MarkdownView {
    width: 300
    markdown: "```cpp\nconst int n = 42;\n```"
    syntaxHighlightingEnabled: true
    syntaxTheme: SyntaxTheme { preset: SyntaxTheme.Dark; keywordColor: "#ff00ff" }
    selectionEnabled: true
}
)", {});
    std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
    if (!view) { qCritical() << component.errorString(); return 1; }
    if (!view->property("syntaxHighlightingEnabled").toBool() || !view->property("syntaxTheme").value<QObject *>()) return 2;
    // Traverse visual delegates, which are not necessarily QObject children.
    std::function<bool(QQuickItem *)> colored = [&](QQuickItem *item) {
        if (!item->property("foregroundRanges").toList().isEmpty()) return true;
        for (auto *child : item->childItems()) if (colored(child)) return true;
        return false;
    };
    QElapsedTimer deadline; deadline.start();
    if (view->property("syntaxHighlightingAvailable").toBool()) {
        while (!colored(view.get()) && deadline.elapsed() < 5000) QCoreApplication::processEvents();
        if (!colored(view.get())) return 3;
    }
    QMetaObject::invokeMethod(view.get(), "selectAll");
    if (!view->property("selectedText").toString().contains("const int n = 42;")) return 4;
    return 0;
}
