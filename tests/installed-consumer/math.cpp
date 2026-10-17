#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
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
import QMarkdown 1.13
MarkdownView { width: 100; mathEnabled: true; markdown: "$$\n\\frac{1}{2}\n$$" }
)", QUrl());
    std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
    if (!view) { qWarning() << component.errors(); return 1; }
    if (view->property("mathAvailable").toBool() != bool(QMARKDOWN_EXPECT_MATH)) return 2;
    const auto find = [](auto &&self, QQuickItem *item) -> QQuickItem * {
        if (item->objectName() == QStringLiteral("mathFormula")) return item;
        for (auto child : item->childItems()) if (auto result = self(self, child)) return result;
        return nullptr;
    };
    QElapsedTimer timer; timer.start();
    while (timer.elapsed() < 5000) {
        application.processEvents();
        auto formula = find(find, view.get());
        if (formula && (!QMARKDOWN_EXPECT_MATH || formula->property("ready").toBool())) return 0;
        QThread::msleep(5);
    }
    return 3;
}
