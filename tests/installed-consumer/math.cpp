#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QElapsedTimer>
#include <QThread>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif
int main(int argc, char **argv) {
    QGuiApplication application(argc, argv);
    QQmlEngine engine; QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
import QMarkdown 1.15
MarkdownView { width: 100; mathEnabled: true; inlineMathEnabled: true; selectionEnabled: true; markdown: "A $x^2$ B\n\n$$\n\\frac{1}{2}\n$$" }
)", QUrl());
    QQuickWindow window; window.resize(120, 200);
    std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
    if (!view) { qWarning() << component.errors(); return 1; }
    view->setParentItem(window.contentItem()); window.show();
    QMetaObject::invokeMethod(view.get(), "selectAll");
    if (view->property("selectedText").toString() != QStringLiteral("A x^2 B\n\n\\frac{1}{2}\n")) return 4;
    if (view->property("mathAvailable").toBool() != bool(QMARKDOWN_EXPECT_MATH)) return 2;
    const auto find = [](auto &&self, QQuickItem *item) -> QQuickItem * {
        if (item->objectName() == QStringLiteral("mathFormula")) return item;
        for (auto child : item->childItems()) if (auto result = self(self, child)) return result;
        return nullptr;
    };
    const auto inlineText = [](auto &&self, QQuickItem *item) -> QQuickItem * {
        if (item->property("mathRenderingEnabled").isValid()
            && !item->property("formulaSpans").toList().isEmpty()) return item;
        for (auto child : item->childItems()) if (auto result = self(self, child)) return result;
        return nullptr;
    };
    QElapsedTimer timer; timer.start();
    while (timer.elapsed() < 5000) {
        application.processEvents();
        auto formula = find(find, view.get());
        auto text = inlineText(inlineText, view.get());
        if (text && text->property("text").toString() != QStringLiteral("A x^2 B")) return 5;
        QRectF start, inside;
        const bool measured = text
            && QMetaObject::invokeMethod(text, "sourceRectangleAt", Q_RETURN_ARG(QRectF, start), Q_ARG(int, 2))
            && QMetaObject::invokeMethod(text, "sourceRectangleAt", Q_RETURN_ARG(QRectF, inside), Q_ARG(int, 3));
        if (formula && measured && (!QMARKDOWN_EXPECT_MATH
            || (formula->property("ready").toBool() && !start.isEmpty() && start == inside))) return 0;
        QThread::msleep(5);
    }
    return 3;
}
