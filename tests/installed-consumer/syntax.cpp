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
#ifdef QMARKDOWN_EXPECT_SYNTAX
    if (view->property("syntaxHighlightingAvailable").toBool() != bool(QMARKDOWN_EXPECT_SYNTAX)) return 5;
#endif
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
    QQmlComponent codeBlocks(&engine);
    codeBlocks.setData(R"(
import QtQuick
import QMarkdown 1.12
MarkdownView {
    width: 160
    codeBlockWrapEnabled: false
    codeBlockLineNumbersVisible: true
    codeBlockHeightLimitEnabled: true
    codeBlockViewportHeight: 100
    codeBlockMinimumLines: 1
    codeBlockMaximumLines: 2
    style: MarkdownStyle { codeBlock.lineNumberColor: "#123456" }
    markdown: "```CPP extra\nconst int value = 42;\nsecond line\nthird line\n```"
    codeBlockActions: Item {
        required property string codeText
        required property var requestCopy
        required property string language
        required property string languageId
        required property string languageName
        required property string infoString
        required property bool fenced
        Component.onCompleted: {
            if (language !== "CPP" || languageId !== "cpp" || languageName !== "C++" || !fenced)
                throw new Error("Invalid installed header metadata")
        }
    }
}
)", {});
    std::unique_ptr<QQuickItem> codeView(qobject_cast<QQuickItem *>(codeBlocks.create()));
    if (!codeView) { qCritical() << codeBlocks.errorString(); return 6; }
    std::function<QQuickItem *(QQuickItem *)> numbered = [&](QQuickItem *item) -> QQuickItem * {
        if (item->property("sourceLines").toList().size() == 3) return item;
        for (auto *child : item->childItems()) if (auto *found = numbered(child)) return found;
        return nullptr;
    };
    QCoreApplication::processEvents();
    // Source delegates can be polished without a rendered frame.
    std::function<void(QQuickItem *)> polish = [&](QQuickItem *item) {
        item->ensurePolished();
        for (auto *child : item->childItems()) polish(child);
    };
    polish(codeView.get());
    if (!numbered(codeView.get())) return 7;
    return 0;
}
