#include <QGuiApplication>
#include <QAccessible>
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
    QGuiApplication app(argc, argv); QAccessible::setActive(true);
    QQmlEngine engine; QQmlComponent component(&engine);
    component.setData(R"QML(
import QtQuick
import QMarkdown 1.16
MarkdownView {
    width: 280; footnotesEnabled: true; selectionEnabled: true
    markdown: "A[^n] B[^n]\n\n[^n]: Note"
}
)QML", {});
    QQuickWindow window; window.resize(300, 180);
    std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
    if (!view) { qWarning() << component.errors(); return 1; }
    view->setParentItem(window.contentItem()); window.show();
    QMetaObject::invokeMethod(view.get(), "selectAll");
    if (view->property("selectedText").toString() != QStringLiteral("A[1] B[1]\n\n1. Note")) return 2;
    bool found = false;
    if (!QMetaObject::invokeMethod(view.get(), "navigateToFragment", Q_RETURN_ARG(bool, found), Q_ARG(QString, "#fn-1")) || !found) return 3;
    const auto items = [](auto &&self, QQuickItem *root) -> QList<QQuickItem *> {
        QList<QQuickItem *> result;
        for (auto item : root->childItems()) { result.append(item); result.append(self(self, item)); }
        return result;
    };
    QElapsedTimer timer; timer.start();
    while (timer.elapsed() < 5000) {
        app.processEvents(); int links = 0; bool atom = false;
        for (auto item : items(items, view.get())) {
            if (item->property("destination").toString().startsWith("#fn")) {
                auto accessible = QAccessible::queryAccessibleInterface(item);
                if (!accessible || accessible->role() != QAccessible::Link || accessible->text(QAccessible::Name).isEmpty()) return 4;
                ++links;
            }
            if (!item->property("referenceSpans").toList().isEmpty() && item->property("text").isValid()) {
                QRectF start, inside;
                QMetaObject::invokeMethod(item, "sourceRectangleAt", Q_RETURN_ARG(QRectF, start), Q_ARG(int, 1));
                QMetaObject::invokeMethod(item, "sourceRectangleAt", Q_RETURN_ARG(QRectF, inside), Q_ARG(int, 2));
                atom = !start.isEmpty() && start == inside;
            }
        }
        if (links == 4 && atom) return 0;
        QThread::msleep(5);
    }
    return 5;
}
